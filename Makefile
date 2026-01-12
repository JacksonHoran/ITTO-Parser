SHELL := /bin/sh
.ONESHELL:

CC := gcc
CFLAGS := -O2 -Wall -Wextra

BIN_DIR := bin
SRC_DIR := src
TOOLS_DIR := tools/tcp
DATA_DIR := data
OUT_DIR := out

PARSER := $(BIN_DIR)/parser
TCP_SEND := $(BIN_DIR)/tcp_send

all: $(PARSER) $(TCP_SEND)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(PARSER): $(SRC_DIR)/parser.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(TCP_SEND): $(TOOLS_DIR)/tcp_send.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

run: all | $(OUT_DIR)
	@set -e
	$(PARSER) 9000 $(OUT_DIR)/parsed.jsonl &
	PID=$$!
	sleep 1
	$(TCP_SEND) 127.0.0.1 9000 $(DATA_DIR)/ITCH
	wait $$PID

sample: run
	@python3 tools/sample.py

clean:
	@rm -f $(PARSER) $(TCP_SEND)
