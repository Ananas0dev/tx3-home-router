# Validation — 2026-10-02

- All 13 recovered eBPF C programs compiled in Debian WSL with Clang 19.1.7 for the BPF target.
- The Linux patch passed `git apply --check` against isolated files from recovered base commit `4c81f10ed9fea62e29081f0d2a129f1b59bf2fe5`.
- The normalized ASIX patch passed the same check against isolated LF-normalized files from `42feb1252fe0669845a8d6714bcc77bfc19eea1d`.
- Local source files and deployment references were checked for private identifiers and secret/file patterns before Git staging.

No new kernel/module was built or loaded. No verifier load, packet workload, reboot, PPP reconnect, or firewall application was performed on the household router. Patch applicability and compilation are narrower checks than hardware validation.
