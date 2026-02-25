# ===========================================================================
# idk-nagger-4 : Makefile — Convenience Wrappers
# ---------------------------------------------------------------------------
# SPDX-License-Identifier: MIT
#
# Usage:
#   make build          — full idf.py build
#   make flash PORT=/dev/ttyUSB0
#   make monitor PORT=/dev/ttyUSB0
#   make clean          — idf.py fullclean
#   make docker-build   — build inside Docker
#   make test           — run host unit tests
#   make sim            — launch host pool simulator
# ===========================================================================

PORT     ?= /dev/ttyUSB0
BAUD     ?= 115200
IDF_PY   ?= idf.py

.PHONY: all build flash monitor clean fullclean docker-build docker-flash test sim bench

all: build

build:
	$(IDF_PY) build

flash:
	$(IDF_PY) -p $(PORT) -b $(BAUD) flash

monitor:
	$(IDF_PY) -p $(PORT) -b $(BAUD) monitor

flash-monitor:
	$(IDF_PY) -p $(PORT) -b $(BAUD) flash monitor

clean:
	$(IDF_PY) fullclean

menuconfig:
	$(IDF_PY) menuconfig

# --- Docker targets ---
docker-build:
	docker build -t esp32-build-env .
	docker run --rm -v $(shell pwd):/project -w /project esp32-build-env idf.py build

docker-flash:
	docker run --rm -v $(shell pwd):/project --device=$(PORT) -w /project \
		esp32-build-env idf.py -p $(PORT) -b $(BAUD) flash

# --- Testing ---
test:
	cd tests && bash run_tests.sh

sim:
	python3 scripts/host_simulator.py

bench:
	cd tools/bench && bash run_bench.sh

# --- CI ---
ci:
	bash scripts/ci_tests.sh
