# ===========================================================================
# idk-nagger-4 : Dockerfile — Reproducible ESP-IDF Build Environment
# ---------------------------------------------------------------------------
# SPDX-License-Identifier: MIT
# Based on: espressif/idf:v5.2.2
#
# Usage:
#   docker build -t esp32-build-env .
#   docker run --rm -v $(pwd):/project -w /project esp32-build-env \
#       idf.py build
# ===========================================================================

FROM espressif/idf:v5.2.2

LABEL maintainer="idk-nagger-4"
LABEL description="ESP-IDF v5.2.2 build environment for idk-nagger-4 miner"

# Install additional host tools for testing and scripting
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-pip \
    python3-venv \
    jq \
    xxd \
    gcc \
    make \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages for host simulation and testing
RUN pip3 install --no-cache-dir \
    pytest \
    pyserial \
    requests

WORKDIR /project

# Default command: build the project
CMD ["idf.py", "build"]
