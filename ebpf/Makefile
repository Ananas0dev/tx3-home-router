CLANG ?= clang
CC ?= cc
MULTIARCH ?= $(shell $(CC) -dumpmachine)
LAN_NET ?= 0xC0000200U
LAN_MASK ?= 0xFFFFFF00U
BPF_CFLAGS = -O2 -g -target bpf -Wall -I/usr/include/$(MULTIARCH) -DLAN_NET=$(LAN_NET) -DLAN_MASK=$(LAN_MASK)
CURRENT = $(patsubst src/%.c,build/%.o,$(wildcard src/*.c))
HISTORY = $(patsubst history/%.c,build/history/%.o,$(wildcard history/*.c))

.PHONY: all history clean
all: $(CURRENT)
history: $(HISTORY)
build/%.o: src/%.c
	@mkdir -p $(@D)
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@
build/history/%.o: history/%.c
	@mkdir -p $(@D)
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@
clean:
	rm -rf build
