ROOT := ../..

C_SRC := $(wildcard *.c)

BUILD_DIR := $(ROOT)/build/
INCLUDE_DIR := $(ROOT)/include/

.PHONY: all

all: $(C_SRC)
	mkdir -p $(BUILD_DIR)
	gcc $(C_SRC) -o $(BUILD_DIR)/decoder -lm -I$(INCLUDE_DIR)


