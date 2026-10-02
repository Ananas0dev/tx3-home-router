# Deployment boundaries

The public repository uses documentation-only subnets and generic domain names. The private notes, DNS account details, TLS material, SSH files, application databases, uploaded files, DHCP leases, packet captures, and raw live logs are excluded.

The applications were hosted behind nginx on the LAN. A publicly trusted HTTPS certificate obtained through DNS-01 does not require opening the application to the Internet. DNS API credentials and certificate private keys must remain outside version control.

The notes describe firewall isolation and IPv6 bypass checks, but a repository snapshot is not proof of current firewall coverage. Inspect the deployed rules and both protocol families locally before changing the network. No changes to the household router are required to read or build this repository.
