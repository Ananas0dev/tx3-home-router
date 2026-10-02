# Provenance and licensing

The eBPF files were recovered from the owner's WSL development directory. Each declares `GPL` in its BPF license section. Public copies substitute a documentation subnet and make classifier network constants overridable at build time. Historical code is preserved for comparison.

Linux USB-core patch context is from GPL-2.0 Linux sources. The quirk mechanism matches prior work proposed by Ivan Hu in April 2025; see the linked [discussion](https://lkml.indiana.edu/hypermail/linux/kernel/2504.1/06490.html). The recovered bare diff lacks author/commit metadata, so this repository does not claim sole authorship or supply an invented Signed-off-by.

ASIX patch context belongs to ASIX Electronic Corporation. The vendor source carries its own GPL notices. Only a small normalized patch is included, with the original license and copyright remaining applicable to its source context.

No new blanket license is asserted over mixed third-party material. A BPF `GPL` loader declaration is preserved as found; it does not resolve every licensing question about an entire repository. Retain upstream notices when redistributing and clarify the owner's preferred license for new documentation and tooling before treating those additions as separately licensed.
