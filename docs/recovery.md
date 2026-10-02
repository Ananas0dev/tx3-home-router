# Recovery and change discipline

This is a planning reference, not a script to run on a live household router.

1. Establish console or another tested recovery path before changing WAN, firewall, boot, or driver settings.
2. Identify every storage device by current mount, label, and UUID. Historical `/dev` names in notes are not safe flashing targets.
3. Preserve known-good boot media and export private configuration to a private backup location.
4. Record the current PPP session, routes, firewall, TC handles, BPF pins, and active services locally.
5. Test one change with a timed rollback that has already been checked.
6. Verify forwarding and household use before making the change persistent.
7. Test reconnect and reboot only in a maintenance window after the service and hook definitions agree.

A rollback script stored in `/tmp` is temporary. A live BPF attachment is not a complete deployment. The historical V5D rollback restored V4; use it only if V4 is actually the desired fallback and the relevant objects still exist.
