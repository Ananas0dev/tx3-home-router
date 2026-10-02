# Is this worth sending upstream?

**The hardware failure and reproduction evidence are useful. The recovered patch should not be submitted unchanged.** Assessment made 2026-10-02; no report or patch has been sent.

## Existing discussion

Ivan Hu proposed a quirk with the same `USB_QUIRK_CHOOSE_VENDOR_SPEC_CFG` name and ASIX device ID in April 2025. Greg Kroah-Hartman's [reply](https://lkml.indiana.edu/hypermail/linux/kernel/2504.1/06490.html) argues that configuration choice should be handled in userspace. This is prior work that must be credited and addressed, not presented as an original new mechanism.

The recovered patch allocates `BIT(19)`. Current [upstream quirks.h](https://github.com/torvalds/linux/blob/master/include/linux/usb/quirks.h) uses that bit for `USB_QUIRK_WINDOWS_CONFIG_REQ_SIZE`. Reusing the old bit in a new submission would be incorrect.

The official [ASIX driver documentation](https://github.com/ASIXElectronics/asix-usb-nic-linux-driver) shows multiple controllers sharing `0b95:1790`; `bcdDevice` distinguishes revisions. A VID/PID-only quirk is broader than the reported AX88179B failure.

## Separate the issues

1. **USB configuration selection:** reproduce on a current kernel with its in-tree driver. Determine whether a userspace configuration choice resolves it. Explain why any kernel change is needed beyond that choice.
2. **PHY/link polling:** the recovered change modifies ASIX's out-of-tree driver. Report that issue to ASIX with device revision, host controller, descriptors, and minimal evidence. A global polling override is not a narrow upstream fix.
3. **Driver support:** ongoing AX88179A-family development may change the result. Check the current networking tree and mailing-list discussion before preparing a patch against an old 6.12 tree.

## Evidence to collect in a maintenance window

Record adapter revision/firmware and USB descriptors with serial numbers and MAC addresses removed; host controller; kernel revision/configuration; in-tree and vendor driver versions; selected configuration; carrier behavior; and reproducible failure/recovery steps. Compare baseline, userspace selection only, and polling only. Include unplug/replug, sustained transfer, reboot, and relevant power-management results.

Then prepare one minimal change per problem against the appropriate current tree, preserve authorship, run `scripts/checkpatch.pl`, and use `scripts/get_maintainer.pl` to identify reviewers. Follow the [kernel submission guide](https://docs.kernel.org/process/submitting-patches.html), including a truthful Signed-off-by supplied by the contributor. Do not send the complete local driver or a kernel image as the proposed fix.
