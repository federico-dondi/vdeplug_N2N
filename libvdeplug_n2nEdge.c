#include <libvdeplug_mod.h>
#include <stdio.h>
#include <sys/capability.h>
#include <n2n.h>


#define NTOP_BUFSIZE 128

// VDE functions

static VDECONN *vde_n2nEdge_open(char *sockname, char *descr,int interface_version,
		struct vde_open_args *open_args);
static ssize_t vde_n2nEdge_recv(VDECONN *conn,void *buf,size_t len,int flags);
static ssize_t vde_n2nEdge_send(VDECONN *conn,const void *buf,size_t len,int flags);
static int vde_n2nEdge_datafd(VDECONN *conn);
static int vde_n2nEdge_ctlfd(VDECONN *conn);
static int vde_n2nEdge_close(VDECONN *conn);

// Globals

// VDE options

struct vdeplug_module vdeplug_ops = {
		.vde_open_real=vde_n2nEdge_open,
		.vde_recv=vde_n2nEdge_recv,
		.vde_send=vde_n2nEdge_send,
		.vde_datafd=vde_n2nEdge_datafd,
		.vde_ctlfd=vde_n2nEdge_ctlfd,
		.vde_close=vde_n2nEdge_close

};

// Keep running

static int keep_running;



static VDECONN *vde_n2nEdge_open(char *sockname,char *descr,int interface_version,
		struct vde_open_args *open_args) {

	n2n_edge_conf_t conf;
	    tuntap_dev tuntap;
	    n2n_edge_t *eee;
	    int rc;

	    edge_init_conf_defaults(&conf);
	    conf.allow_p2p = 1;                                                                      // Whether to allow peer-to-peer communication
	    conf.allow_routing = 1;                                                                  // Whether to allow the edge to route packets to other edges
	    snprintf((char *)conf.community_name, sizeof(conf.community_name), "%s", "mycommunity"); // Community to connect to
	    conf.disable_pmtu_discovery = 1;                                                         // Whether to disable the path MTU discovery
	    conf.drop_multicast = 0;                                                                 // Whether to disable multicast
	    conf.dyn_ip_mode = 0;                                                                    // Whether the IP address is set dynamically (see IP mode; 0 if static, 1 if dynamic)
	    conf.encrypt_key = "mysecret";                                                           // Secret to decrypt & encrypt with
	    conf.local_port = 0;                                                                     // What port to use (0 = any port)
	    conf.mgmt_port = N2N_EDGE_MGMT_PORT;                                                     // Edge management port (5644 by default)
	    conf.register_interval = 1;                                                              // Interval for both UDP NAT hole punching and supernode registration
	    conf.register_ttl = 1;                                                                   // Interval for UDP NAT hole punching through supernode
	    edge_conf_add_supernode(&conf, "localhost:1234");                                        // Supernode to connect to
	    conf.tos = 16;                                                                           // Type of service for sent packets
	    conf.transop_id = N2N_TRANSFORM_ID_TWOFISH;                                              // Use the twofish encryption

	    if (edge_verify_conf(&conf) != 0)
	    {
	        return -1;
	    }

	    if (tuntap_open(&tuntap,
	                    "edge0",             // Name of the device to create
	                    "static",            // IP mode; static|dhcp
	                    "10.0.0.1",          // Set ip address
	                    "255.255.255.0",     // Netmask to use
	                    "DE:AD:BE:EF:01:10", // Set mac address
	                    DEFAULT_MTU) < 0)    // MTU to use
	    {
	        return -1;
	    }

	    eee = edge_init(&tuntap, &conf, &rc);
	    if (eee == NULL)
	    {
	        exit(1);
	    }

	    keep_running = 1;
	    rc = run_edge_loop(eee, &keep_running);

	    edge_term(eee);
	    tuntap_close(&tuntap);

	    return rc;
}

static ssize_t vde_n2nEdge_recv(VDECONN *conn,void *buf,size_t len,int flags) {
	return NULL;
}

static ssize_t vde_n2nEdge_send(VDECONN *conn,const void *buf,size_t len,int flags) {
	return NULL;
}

static int vde_n2nEdge_datafd(VDECONN *conn) {
	return 0;
}

static int vde_n2nEdge_ctlfd(VDECONN *conn) {
	return -1;
}

static int vde_n2nEdge_close(VDECONN *conn) {
	return -1;
}




