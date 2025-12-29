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

$(PARSER): $(SRC_DIR)/parser.c
	$(CC) $(CFLAGS) $< -o $@

$(TCP_SEND): $(TOOLS_DIR)/tcp_send.c
	$(CC) $(CFLAGS) $< -o $@

run: all
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
