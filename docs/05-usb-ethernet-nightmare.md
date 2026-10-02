# The AX88179B investigation

The adapter exposed several USB configurations. The workaround combined early selection of the vendor-specific configuration with ASIX's 4.1.0 driver, disabled the driver's separate configuration-selection registration, and forced PHY polling when interrupt-driven link detection did not work on this setup.

These are separate changes at different layers. Device enumeration, driver binding, carrier detection, and sustained forwarding should be tested independently. The presence of a driver with a similar chip name did not prove that this revision worked with it.

See [patches and reproduction](../usb-ethernet/README.md) and the [upstream assessment](../usb-ethernet/upstream-assessment.md). The patches are local workarounds, with known review issues; they are not represented as accepted Linux fixes.
