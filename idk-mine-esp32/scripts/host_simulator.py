#!/usr/bin/env python3
# ===========================================================================
# idk-nagger-4 : Host Pool Simulator
# ---------------------------------------------------------------------------
# SPDX-License-Identifier: MIT
#
# A minimal Stratum v1 pool simulator for testing the miner firmware.
# Accepts TCP connections, serves fake mining jobs, and validates
# share submissions.
#
# Usage:
#   python3 scripts/host_simulator.py [--port 3333] [--difficulty 16]
#
# The miner firmware should be configured to connect to this host's IP
# on the specified port (default: 3333).
# ===========================================================================

import argparse
import hashlib
import json
import os
import socket
import struct
import sys
import threading
import time


class MiningJob:
    """Generates a fake mining job for testing."""

    _counter = 0

    @classmethod
    def generate(cls):
        cls._counter += 1
        job_id = f"job_{cls._counter:08x}"
        prev_hash = os.urandom(32).hex()
        merkle_root = os.urandom(32).hex()
        version = "20000000"
        nbits = "1d00ffff"
        ntime = format(int(time.time()), '08x')

        return {
            "job_id": job_id,
            "prev_hash": prev_hash,
            "merkle_root": merkle_root,
            "version": version,
            "nbits": nbits,
            "ntime": ntime,
        }


class PoolSimulator:
    """Simulated Stratum v1 mining pool server."""

    def __init__(self, host="0.0.0.0", port=3333, difficulty_bits=16):
        self.host = host
        self.port = port
        self.difficulty_bits = difficulty_bits
        self.server_socket = None
        self.running = False
        self.clients = []
        self.stats = {
            "connections": 0,
            "subscribes": 0,
            "authorizations": 0,
            "jobs_sent": 0,
            "shares_received": 0,
            "shares_accepted": 0,
            "shares_rejected": 0,
        }

    def start(self):
        """Start the pool simulator."""
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(5)
        self.running = True

        print(f"[POOL] idk-nagger-4 Test Pool Simulator")
        print(f"[POOL] Listening on {self.host}:{self.port}")
        print(f"[POOL] Difficulty: {self.difficulty_bits} bits")
        print(f"[POOL] Waiting for miner connections...")
        print()

        while self.running:
            try:
                self.server_socket.settimeout(1.0)
                client_sock, addr = self.server_socket.accept()
                self.stats["connections"] += 1
                print(f"[POOL] New connection from {addr[0]}:{addr[1]}")

                t = threading.Thread(
                    target=self._handle_client,
                    args=(client_sock, addr),
                    daemon=True,
                )
                t.start()
                self.clients.append(t)

            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    print(f"[POOL] Accept error: {e}")

    def stop(self):
        """Stop the pool simulator."""
        self.running = False
        if self.server_socket:
            self.server_socket.close()
        print(f"\n[POOL] Shutting down. Stats: {json.dumps(self.stats, indent=2)}")

    def _handle_client(self, sock, addr):
        """Handle a single miner connection."""
        client_name = f"{addr[0]}:{addr[1]}"
        sock.settimeout(120.0)

        try:
            while self.running:
                data = self._recv_line(sock)
                if not data:
                    break

                try:
                    msg = json.loads(data)
                except json.JSONDecodeError:
                    print(f"[POOL] [{client_name}] Invalid JSON: {data[:100]}")
                    continue

                method = msg.get("method", "")
                msg_id = msg.get("id", 0)

                if method == "mining.subscribe":
                    self._handle_subscribe(sock, msg_id, client_name)

                elif method == "mining.authorize":
                    self._handle_authorize(sock, msg_id, msg, client_name)

                elif method == "mining.submit":
                    self._handle_submit(sock, msg_id, msg, client_name)

                else:
                    print(f"[POOL] [{client_name}] Unknown method: {method}")

        except socket.timeout:
            print(f"[POOL] [{client_name}] Connection timed out")
        except ConnectionResetError:
            print(f"[POOL] [{client_name}] Connection reset")
        except Exception as e:
            print(f"[POOL] [{client_name}] Error: {e}")
        finally:
            sock.close()
            print(f"[POOL] [{client_name}] Disconnected")

    def _handle_subscribe(self, sock, msg_id, client_name):
        """Handle mining.subscribe — return session info."""
        self.stats["subscribes"] += 1
        response = {
            "id": msg_id,
            "result": [
                [["mining.notify", "session_001"]],
                "extranonce1_hex",
                4,
            ],
            "error": None,
        }
        self._send_line(sock, json.dumps(response))
        print(f"[POOL] [{client_name}] Subscribed")

        # Send initial job
        self._send_job(sock, client_name)

    def _handle_authorize(self, sock, msg_id, msg, client_name):
        """Handle mining.authorize — always accept."""
        self.stats["authorizations"] += 1
        params = msg.get("params", [])
        worker = params[0] if params else "unknown"

        response = {"id": msg_id, "result": True, "error": None}
        self._send_line(sock, json.dumps(response))
        print(f"[POOL] [{client_name}] Authorized worker: {worker}")

    def _handle_submit(self, sock, msg_id, msg, client_name):
        """Handle mining.submit — accept all shares for testing."""
        self.stats["shares_received"] += 1
        params = msg.get("params", [])

        # For testing, accept all shares
        accepted = True
        self.stats["shares_accepted"] += 1

        response = {"id": msg_id, "result": accepted, "error": None}
        self._send_line(sock, json.dumps(response))

        nonce = params[4] if len(params) > 4 else "unknown"
        print(
            f"[POOL] [{client_name}] Share {'ACCEPTED' if accepted else 'REJECTED'}"
            f" (nonce={nonce}, total={self.stats['shares_accepted']})"
        )

        # Send a new job after each share
        self._send_job(sock, client_name)

    def _send_job(self, sock, client_name):
        """Send a mining.notify with a new job."""
        job = MiningJob.generate()
        self.stats["jobs_sent"] += 1

        notify = {
            "id": None,
            "method": "mining.notify",
            "params": [
                job["job_id"],
                job["prev_hash"],
                "",  # coinb1
                "",  # coinb2
                [],  # merkle branches
                job["version"],
                job["nbits"],
                job["ntime"],
                True,  # clean_jobs
            ],
        }
        self._send_line(sock, json.dumps(notify))
        print(f"[POOL] [{client_name}] Sent job {job['job_id']}")

    def _send_line(self, sock, line):
        """Send a newline-terminated JSON line."""
        sock.sendall((line + "\n").encode())

    def _recv_line(self, sock):
        """Receive a newline-terminated line."""
        buf = b""
        while True:
            try:
                c = sock.recv(1)
                if not c:
                    return None
                if c == b"\n":
                    return buf.decode("utf-8", errors="replace")
                buf += c
                if len(buf) > 8192:
                    return None
            except socket.timeout:
                return None


def main():
    parser = argparse.ArgumentParser(
        description="idk-nagger-4 Test Pool Simulator"
    )
    parser.add_argument(
        "--host", default="0.0.0.0", help="Bind address (default: 0.0.0.0)"
    )
    parser.add_argument(
        "--port", type=int, default=3333, help="Listen port (default: 3333)"
    )
    parser.add_argument(
        "--difficulty",
        type=int,
        default=16,
        help="Difficulty in leading zero bits (default: 16)",
    )
    args = parser.parse_args()

    pool = PoolSimulator(
        host=args.host, port=args.port, difficulty_bits=args.difficulty
    )

    try:
        pool.start()
    except KeyboardInterrupt:
        pool.stop()


if __name__ == "__main__":
    main()
