# vdeplug_N2N
Peer-to-Peer VPN plugin module for [VDEPlug](https://github.com/rd235/vdeplug4). Based on the [N2N](https://github.com/ntop/n2n) library.
## How to install:

1. `git clone` this repo
2. `cd` into the new directory
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `make configure `
7. `sudo make install`

## Edge Usage:

Run:
`sudo vde_plug n2n_edge://`

### NB: root privileges are needed for the creation of the tap interface.

parameters: (see [vde_url arguments parsing](https://github.com/rd235/vdeplug4/blob/master/doc/howto_create_a_vdeplug_plugin#L52))
- tapname=edge0 
- ipmode=static
- netmask=255.255.255.0
- mac=DE:AD:BE:EF:01:10
- community=MYCOMMUNITY
- secret=MYSECRET
- snodeport=1234
- snodeaddr=localhost

## Supernode Usage:

Run:
`vde_plug n2n_super://`

parameters:
- lport=1234

## Using configuration files

Both edge and super can read parameters from configuration files, to be placed respectively in
- /etc/n2n/edge.conf
- /etc/n2n/supernode.conf

In this repository you can find default configuration files.

## Connect the dots

Once a supernode has been started, two edge dots can reach each other, for example:

1st node:
`sudo vde_plug n2n_edge://first/cfgfile=yes/ipaddr=10.0.0.1/secret=unibo/community=vde`

2nd node:
`sudo vde_plug n2n_edge://second/cfgfile=yes/ipaddr=10.0.0.2/secret=unibo/community=vde`

Once set up, the machine running the first node can contact the machine running the second node, via the tap interfaces that
have been created.

1st node:
`ping -I edge0 10.0.0.2`


