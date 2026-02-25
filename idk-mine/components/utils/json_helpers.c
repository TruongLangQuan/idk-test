/* ===========================================================================
 * idk-nagger-4 : JSON Helpers — Implementation
 * ---------------------------------------------------------------------------
 * SPDX-License-Identifier: MIT
 *
 * Minimal JSON parser for extracting values from simple JSON objects.
 * No external dependencies (no cJSON). Handles flat key-value pairs.
 * For nested objects, extend or use cJSON component.
 * ======================================================================== */

#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Find the value string for a given key in JSON.
 * Returns pointer to first char of value, or NULL.
 */
static const char *json_find_key(const char *json, const char *key)
{
    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char *pos = strstr(json, search);
    if (!pos) return NULL;

    /* Skip past key and colon */
    pos += strlen(search);
    while (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r') pos++;
    if (*pos != ':') return NULL;
    pos++;
    while (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r') pos++;

    return pos;
}

bool json_get_string(const char *json, const char *key, char *out, size_t outlen)
{
    const char *val = json_find_key(json, key);
    if (!val || *val != '"') return false;

    val++; /* skip opening quote */
    size_t i = 0;
    while (*val && *val != '"' && i < outlen - 1) {
        if (*val == '\\' && *(val + 1)) {
            val++; /* skip escape */
        }
        out[i++] = *val++;
    }
    out[i] = '\0';
    return true;
}

bool json_get_int(const char *json, const char *key, int *out)
{
    const char *val = json_find_key(json, key);
    if (!val) return false;

    /* Handle quoted numbers */
    if (*val == '"') val++;

    char *end;
    long v = strtol(val, &end, 10);
    if (end == val) return false;
    *out = (int)v;
    return true;
}

bool json_get_bool(const char *json, const char *key, bool *out)
{
    const char *val = json_find_key(json, key);
    if (!val) return false;

    if (strncmp(val, "true", 4) == 0) {
        *out = true;
        return true;
    }
    if (strncmp(val, "false", 5) == 0) {
        *out = false;
        return true;
    }
    return false;
}
