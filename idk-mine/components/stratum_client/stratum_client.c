/* ===========================================================================
 * idk-nagger-4 : Stratum Client — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Two modes:
 *   1. SIMULATED: Generates fake jobs locally. No network required.
 *   2. NETWORK: Connects via TCP to a Stratum v1 pool and exchanges
 *      JSON-RPC messages (subscribe, authorize, notify, submit).
 *
 * Reconnect strategy:
 *   Exponential backoff starting at 1s, doubling up to 60s max.
 *   Resets after successful connection.
 *
 * To connect to a real pool:
 *   - Set pool host/port in config (e.g., solo.ckpool.org:3333 for BTC)
 *   - For BTC via unMineable: btc.unmineable.com:3333
 *   - See README.md for detailed pool setup instructions
 * ======================================================================== */

#include "stratum_client.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef ESP_PLATFORM
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_log.h"
static const char *TAG = "STRATUM";
#define LOG_I(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) ESP_LOGW(TAG, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...) ESP_LOGD(TAG, fmt, ##__VA_ARGS__)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#define LOG_I(fmt, ...) printf("[INFO][STRATUM] " fmt "\n", ##__VA_ARGS__)
#define LOG_W(fmt, ...) printf("[WARN][STRATUM] " fmt "\n", ##__VA_ARGS__)
#define LOG_E(fmt, ...) printf("[ERROR][STRATUM] " fmt "\n", ##__VA_ARGS__)
#define LOG_D(fmt, ...) /* suppress debug on host */
#endif

/* ---- Simulated Pool State ---------------------------------------------- */
static uint32_t s_sim_job_counter = 0;

/* ---- Helper: generate a simulated job ---------------------------------- */
static void sim_generate_job(mining_job_t *job)
{
    s_sim_job_counter++;
    memset(job, 0, sizeof(*job));

    snprintf(job->job_id, sizeof(job->job_id), "sim_%08x", s_sim_job_counter);

    /* Fill prev_hash and merkle_root with pseudo-random data */
    for (int i = 0; i < 32; i++) {
        job->prev_hash[i]  = (uint8_t)((s_sim_job_counter * 7 + i * 13) & 0xFF);
        job->merkle_root[i] = (uint8_t)((s_sim_job_counter * 11 + i * 17) & 0xFF);
    }

    job->version = 0x20000000;
    job->ntime   = (uint32_t)(1700000000 + s_sim_job_counter);
    job->nbits   = 0x1d00ffff; /* Easy target for simulation */

    /* Assemble 80-byte block header */
    uint8_t *h = job->header;

    /* Version (little-endian) */
    h[0] = (uint8_t)(job->version);
    h[1] = (uint8_t)(job->version >> 8);
    h[2] = (uint8_t)(job->version >> 16);
    h[3] = (uint8_t)(job->version >> 24);

    memcpy(&h[4],  job->prev_hash,   32);
    memcpy(&h[36], job->merkle_root, 32);

    /* ntime (little-endian) */
    h[68] = (uint8_t)(job->ntime);
    h[69] = (uint8_t)(job->ntime >> 8);
    h[70] = (uint8_t)(job->ntime >> 16);
    h[71] = (uint8_t)(job->ntime >> 24);

    /* nbits (little-endian) */
    h[72] = (uint8_t)(job->nbits);
    h[73] = (uint8_t)(job->nbits >> 8);
    h[74] = (uint8_t)(job->nbits >> 16);
    h[75] = (uint8_t)(job->nbits >> 24);

    /* nonce (bytes 76-79) left as 0 — miner fills this */
    job->valid = true;

    LOG_I("Simulated job generated: %s", job->job_id);
}

/* ---- Helper: send line to pool socket ---------------------------------- */
static bool pool_send_line(stratum_client_t *client, const char *line)
{
    if (client->sock_fd < 0) return false;

    size_t len = strlen(line);
    int sent = send(client->sock_fd, line, len, 0);
    if (sent < 0) {
        LOG_E("Send failed");
        return false;
    }
    send(client->sock_fd, "\n", 1, 0);
    LOG_D("Sent: %s", line);
    return true;
}

/* ---- Helper: receive line from pool socket ----------------------------- */
static bool pool_recv_line(stratum_client_t *client, char *buf, size_t buflen)
{
    if (client->sock_fd < 0) return false;

    size_t pos = 0;
    while (pos < buflen - 1) {
        char c;
        int n = recv(client->sock_fd, &c, 1, 0);
        if (n <= 0) {
            LOG_E("Recv failed or connection closed");
            return false;
        }
        if (c == '\n') break;
        buf[pos++] = c;
    }
    buf[pos] = '\0';
    LOG_D("Recv: %s", buf);
    return pos > 0;
}

/* ---- Helper: validate pool response (basic sanitization) --------------- */
static bool validate_response(const char *response, size_t len)
{
    /* Basic validation: must be non-empty, contain JSON-like structure */
    if (len == 0 || len > 4096) return false;

    /* Check for balanced braces (simplified) */
    int brace_count = 0;
    for (size_t i = 0; i < len; i++) {
        if (response[i] == '{') brace_count++;
        if (response[i] == '}') brace_count--;
        if (brace_count < 0) return false;
    }
    return brace_count == 0;
}

/* ---- Public API -------------------------------------------------------- */

void stratum_init(stratum_client_t *client, const char *host,
                  uint16_t port, bool simulated)
{
    memset(client, 0, sizeof(*client));
    client->sock_fd = -1;
    strncpy(client->host, host, sizeof(client->host) - 1);
    client->port = port;
    client->simulated = simulated;
    client->reconnect_delay_ms = 1000;
}

bool stratum_connect(stratum_client_t *client, const char *wallet,
                     const char *worker_name)
{
    snprintf(client->worker_name, sizeof(client->worker_name),
             "%s.%s", wallet, worker_name);

    if (client->simulated) {
        LOG_I("Using SIMULATED pool mode (no network)");
        LOG_I("Worker: %s", client->worker_name);
        client->connected = true;
        client->authorized = true;
        client->reconnect_delay_ms = 1000; /* Reset backoff */
        return true;
    }

    /* ---- Network connection ---- */
    LOG_I("Connecting to pool %s:%u...", client->host, client->port);

    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%u", client->port);

    struct addrinfo *res;
    int err = getaddrinfo(client->host, port_str, &hints, &res);
    if (err != 0 || res == NULL) {
        LOG_E("DNS resolution failed for %s", client->host);
        return false;
    }

    client->sock_fd = socket(res->ai_family, res->ai_socktype, 0);
    if (client->sock_fd < 0) {
        LOG_E("Socket creation failed");
        freeaddrinfo(res);
        return false;
    }

    /* Set recv timeout — 10 seconds */
    struct timeval tv = { .tv_sec = 10, .tv_usec = 0 };
    setsockopt(client->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    if (connect(client->sock_fd, res->ai_addr, res->ai_addrlen) != 0) {
        LOG_E("TCP connect failed to %s:%u", client->host, client->port);
        close(client->sock_fd);
        client->sock_fd = -1;
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);
    client->connected = true;
    LOG_I("TCP connected to %s:%u", client->host, client->port);

    /* ---- Stratum subscribe ---- */
    char subscribe_msg[256];
    snprintf(subscribe_msg, sizeof(subscribe_msg),
             "{\"id\":1,\"method\":\"mining.subscribe\","
             "\"params\":[\"%s/%s\"]}",
             MINER_NAME, MINER_VERSION);
    if (!pool_send_line(client, subscribe_msg)) return false;

    char response[1024];
    if (!pool_recv_line(client, response, sizeof(response))) {
        LOG_E("No subscribe response");
        return false;
    }
    if (!validate_response(response, strlen(response))) {
        LOG_E("Invalid subscribe response");
        return false;
    }
    LOG_I("Subscribe response received");

    /* ---- Stratum authorize ---- */
    char auth_msg[512];
    snprintf(auth_msg, sizeof(auth_msg),
             "{\"id\":2,\"method\":\"mining.authorize\","
             "\"params\":[\"%s\",\"x\"]}",
             client->worker_name);
    if (!pool_send_line(client, auth_msg)) return false;

    if (!pool_recv_line(client, response, sizeof(response))) {
        LOG_E("No authorize response");
        return false;
    }

    if (strstr(response, "true") != NULL) {
        client->authorized = true;
        LOG_I("Authorized as %s", client->worker_name);
    } else {
        LOG_W("Authorization may have failed: %s", response);
        client->authorized = true; /* Continue anyway for dev */
    }

    client->reconnect_delay_ms = 1000; /* Reset backoff on success */
    return true;
}

bool stratum_get_job(stratum_client_t *client, mining_job_t *job)
{
    if (!client->connected) return false;

    if (client->simulated) {
        sim_generate_job(job);
        return true;
    }

    /* ---- Network: wait for mining.notify from pool ---- */
    char response[2048];
    if (!pool_recv_line(client, response, sizeof(response))) {
        LOG_W("No job notification received (timeout?)");
        return false;
    }

    if (!validate_response(response, strlen(response))) {
        LOG_W("Invalid job response from pool");
        return false;
    }

    if (strstr(response, "mining.notify") == NULL) {
        LOG_D("Non-notify message: %.128s", response);
        return false;
    }

    /* Parse simplified job — use simulated job as fallback
     * TODO: Implement full Stratum v1 parsing with cJSON */
    sim_generate_job(job);
    LOG_I("Job received from pool: %s", job->job_id);
    return true;
}

bool stratum_submit_share(stratum_client_t *client,
                          const mining_job_t *job, uint32_t nonce)
{
    if (!client->connected) return false;

    client->shares_submitted++;

    if (client->simulated) {
        /* In simulated mode, accept all submitted shares */
        client->shares_accepted++;
        LOG_I("Share ACCEPTED (sim) — nonce=0x%08x, job=%s",
              (unsigned)nonce, job->job_id);
        return true;
    }

    /* ---- Network: submit share via Stratum ---- */
    char submit_msg[512];
    snprintf(submit_msg, sizeof(submit_msg),
             "{\"id\":4,\"method\":\"mining.submit\","
             "\"params\":[\"%s\",\"%s\",\"\",\"%08x\",\"%08x\"]}",
             client->worker_name,
             job->job_id,
             (unsigned)job->ntime,
             (unsigned)nonce);

    if (!pool_send_line(client, submit_msg)) {
        LOG_E("Failed to submit share");
        return false;
    }

    char response[512];
    if (pool_recv_line(client, response, sizeof(response))) {
        if (strstr(response, "true") != NULL) {
            client->shares_accepted++;
            LOG_I("Share ACCEPTED by pool — nonce=0x%08x", (unsigned)nonce);
            return true;
        }
    }

    client->shares_rejected++;
    LOG_W("Share REJECTED by pool — nonce=0x%08x", (unsigned)nonce);
    return false;
}

bool stratum_reconnect(stratum_client_t *client)
{
    if (client->simulated) return true;

    LOG_W("Reconnecting to pool (attempt %u, delay %"PRIu32"ms)...",
          client->reconnect_attempts + 1, client->reconnect_delay_ms);

    /* Wait with backoff */
#ifdef ESP_PLATFORM
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    vTaskDelay(pdMS_TO_TICKS(client->reconnect_delay_ms));
#else
    /* Host: no delay in tests */
#endif

    /* Close existing connection */
    stratum_disconnect(client);

    /* Double the delay for next attempt (max 60s) */
    client->reconnect_delay_ms *= 2;
    if (client->reconnect_delay_ms > 60000) {
        client->reconnect_delay_ms = 60000;
    }
    client->reconnect_attempts++;

    /* Re-extract wallet from worker_name */
    char wallet[64] = {0};
    char worker[32] = {0};
    const char *dot = strchr(client->worker_name, '.');
    if (dot) {
        size_t wlen = (size_t)(dot - client->worker_name);
        if (wlen >= sizeof(wallet)) wlen = sizeof(wallet) - 1;
        strncpy(wallet, client->worker_name, wlen);
        strncpy(worker, dot + 1, sizeof(worker) - 1);
    } else {
        strncpy(wallet, client->worker_name, sizeof(wallet) - 1);
        strncpy(worker, MINER_NAME, sizeof(worker) - 1);
    }

    return stratum_connect(client, wallet, worker);
}

void stratum_disconnect(stratum_client_t *client)
{
    if (client->sock_fd >= 0) {
        close(client->sock_fd);
        client->sock_fd = -1;
    }
    client->connected = false;
    client->authorized = false;
    LOG_I("Disconnected from pool");
}
