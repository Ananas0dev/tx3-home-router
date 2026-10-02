# AX88179B on the TX3

The recorded working combination used a Linux 6.12 USB-core workaround and the ASIX 4.1.0 vendor driver. The source archive preserves two separate changes:

| Patch | Target | Effect |
| --- | --- | --- |
| `patches/0001-asix-ax88179b-force-usb-config1.patch` | Linux USB core | Prefer a vendor-specific configuration for USB ID `0b95:1790` |
| `patches/0002-asix-4.1.0-known-good.patch` | ASIX 4.1.0 vendor source | Enable/force PHY polling, fix a preprocessor typo, and bypass the separate config-select device driver |

The first patch's filename says config1, but its code selects a matching vendor-specific interface configuration; it does not simply hardcode the number 1. The second patch was normalized from the recovered CRLF-heavy diff so that reviewers can see its actual changes.

## Reproduction scope

The patches were recovered from the WSL build workspace. Do not apply the USB-core patch to current Linux unchanged: its bit assignment conflicts with newer upstream code, and its USB ID matches several chip revisions. The vendor patch forces polling broadly and retains experimental `#if 1` code. These properties need review before general use.

Use the exact original kernel and ASIX source, normalize ASIX text files to LF, and run `git apply --check` before applying a patch. Build against the target kernel's prepared headers/configuration and compiler ABI. A local build is not a substitute for unplug/replug, carrier, traffic, suspend/resume, or reboot tests on matching hardware.

The official vendor source is [ASIXElectronics/asix-usb-nic-linux-driver](https://github.com/ASIXElectronics/asix-usb-nic-linux-driver). The recovered reference checkout identifies commit `42feb1252fe0669845a8d6714bcc77bfc19eea1d`; validate patch applicability rather than assuming a moving branch still matches.

See [upstream assessment](upstream-assessment.md) and [provenance](../NOTICE.md). No compiled modules or kernel images are distributed here.
