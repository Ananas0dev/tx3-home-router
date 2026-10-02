# TX3 Mini and Armbian

The later notes identify a TX3 Mini (Amlogic S905W, ARM64), Armbian 26.8.3 Bookworm, and kernel `6.12.110-ophub`. LAN uses the built-in Ethernet port; WAN uses an ASIX AX88179B USB adapter. Compilation was performed on a separate Debian WSL machine.

An older part of the notes describes ImmortalWrt as the active router and Armbian as a migration target. This is a historical stage, not evidence that both systems are currently active. Storage roles also changed over time. Never infer a safe flash target from an old device name.

The repository includes source and patches instead of multi-gigabyte kernel trees, private build paths, or machine-specific binaries. The custom driver requires a matching kernel build environment; ordinary userspace headers alone are insufficient.
