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

parameters:
1. tapname=edge0 
1. ipmode=static
1. netmask=255.255.255.0
1. mac= DE:AD:BE:EF:01:10
1. community=MYCOMMUNITY
1. secret=MYSECRET
1. snodeport=1234
1. snodeaddr=localhost


## Configuration

Reading configuration from file will be available soon.
