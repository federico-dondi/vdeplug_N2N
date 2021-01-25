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

## Usage:

Run:
`sudo vde_plug n2nEdge://`

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


## Configuration

Reading configuration from file will be available soon.
