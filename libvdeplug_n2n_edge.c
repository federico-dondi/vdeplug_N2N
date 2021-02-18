#include <libvdeplug_mod.h>
#include <n2n.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "utils.h"


#define NTOP_BUFSIZE 128
#define VDEPLUG_EDGE_MAX_PARAMS 8
#define N2N_EDGE_CONFIG_FILE "/etc/n2n/edge.conf"

// Utility
//int parseConf(const char *path, struct vdeparms *parms, const int MAX_PARMS);
// VDE functions
static VDECONN *vde_n2nEdge_open(char *sockname, char *descr,int interface_version,
		struct vde_open_args *open_args);
static ssize_t vde_n2nEdge_recv(VDECONN *conn,void *buf,size_t len,int flags);
static ssize_t vde_n2nEdge_send(VDECONN *conn,const void *buf,size_t len,int flags);
static int vde_n2nEdge_datafd(VDECONN *conn);
static int vde_n2nEdge_ctlfd(VDECONN *conn);
static int vde_n2nEdge_close(VDECONN *conn);

// VDE options

struct vdeplug_module vdeplug_ops = {
		.vde_open_real=vde_n2nEdge_open,
		.vde_recv=vde_n2nEdge_recv,
		.vde_send=vde_n2nEdge_send,
		.vde_datafd=vde_n2nEdge_datafd,
		.vde_ctlfd=vde_n2nEdge_ctlfd,
		.vde_close=vde_n2nEdge_close

};

struct vde_n2n_conn {
	void *handle;
	struct vdeplug_module *module;
	void *data;
	tuntap_dev *tuntap;
	n2n_edge_t *eee;
};

// Keep running.

static int keep_running;
static int rc;

static VDECONN *vde_n2nEdge_open(char *sockname,char *descr,int interface_version,
		struct vde_open_args *open_args) {

	// N2N stuff
	n2n_edge_conf_t conf;
    	tuntap_dev tuntap;
	n2n_edge_t *eee;
	    
	// VDE stuff
	struct vde_n2n_conn *newconn = NULL;

	char
		*tapname = "edge0", 		// Default is edge0
		*ipmode = "static",		// Default is STATIC
		*ipaddr = NULL,
		*netmask = "255.255.255.0",	// Default is 255.255.255.0
		*mac = NULL,
		*community = NULL,
		*secret = NULL,
		*snodeport = NULL,
		*snodeaddr = NULL,
		*cfgfile = NULL;

	struct addrinfo hints;
	struct vdeparms parms[] = {
		{"tapname", &tapname},
		{"ipmode", &ipmode},
		{"ipaddr", &ipaddr},
		{"netmask", &netmask},
		{"mac", &mac},
		{"community", &community},
		{"snodeport", &snodeport},
		{"snodeaddr", &snodeaddr},
		{"cfgfile", &cfgfile},
		{"secret", &secret},
		{NULL, NULL}};


	memset(&hints, 0, sizeof(struct addrinfo));

	if (vde_parseparms(sockname, parms) != 0)
		return NULL;
		
	// Utility per parsare i parametri da file di conf.
	if (cfgfile != NULL) {
	printf("trying to parse file...\n");
		parseConf(N2N_EDGE_CONFIG_FILE, &parms, VDEPLUG_EDGE_MAX_PARAMS);
	}
	
// For debug: print all parameters
//	for (int i=0; i < 10; i++) {
//		if (*parms[i].value != NULL)
//			printf("%s: %s\n", parms[i].tag, *parms[i].value);
//	}


// Simple "safe" string concatenation 
	char snode[256];
	snprintf(snode, sizeof snode, "%s:%s", snodeaddr, snodeport);

// Config phase
	edge_init_conf_defaults(&conf);
	conf.allow_p2p = 1; 									// Whether to allow peer-to-peer communication
	conf.allow_routing = 1;									// Whether to allow the edge to route packets to other edges
	snprintf((char *)conf.community_name, sizeof(conf.community_name), "%s", community); 	// Community to connect to
	conf.disable_pmtu_discovery = 1;                                                        // Whether to disable the path MTU discovery
	conf.drop_multicast = 0;                                                                // Whether to disable multicast

	if (strcmp(ipmode, "static") == 0)
		conf.dyn_ip_mode = 0;                                                           // Whether the IP address is set dynamically (see IP mode; 0 if static, 1 if dynamic)
	else
		conf.dyn_ip_mode = 1;

	conf.encrypt_key = secret;                                                           	// Secret to decrypt & encrypt with
	conf.local_port = 0;                                                                    // What port to use (0 = any port)
	conf.mgmt_port = N2N_EDGE_MGMT_PORT;                                                    // Edge management port (5644 by default)
	conf.register_interval = 1;                                                             // Interval for both UDP NAT hole punching and supernode registration
	conf.register_ttl = 1;                                                                  // Interval for UDP NAT hole punching through supernode
	edge_conf_add_supernode(&conf, snode);                                        		// Supernode to connect to
	conf.tos = 16;                                                                          // Type of service for sent packets
	conf.transop_id = N2N_TRANSFORM_ID_TWOFISH;						// Use the twofish encryption

// End config phase

	//Verify if legit config has been used
	if (edge_verify_conf(&conf) != 0) {
		exit(EXIT_FAILURE);
	}

	// Try to create a new TUNTAP interface
	if (tuntap_open(&tuntap,
			tapname,    		// Name of the tuntap device to create
			ipmode,     		// IP mode; static|dhcp
			ipaddr,     		// Set ip address
			netmask,    		// Netmask to use
			mac, 			// Set mac address
			DEFAULT_MTU) < 0)   // MTU to use
	{
		exit(EXIT_FAILURE);
	}
	// Init the structure with current conf
	eee = edge_init(&tuntap, &conf, &rc);
	// Correctness check
	if (eee == NULL) {
		exit(EXIT_FAILURE);
	}
	// Keep looping
	keep_running = 1;
	// Allocation of our vdeconn structure
	newconn = calloc(1, sizeof(*newconn));
	newconn->eee = eee;
	newconn->tuntap = &tuntap;

	// Run the edge routine
	// Tricky solution to return a value instead of hanging in the loop
	if (fork() == 0) {
		rc = run_edge_loop(eee, &keep_running);
		return (VDECONN *)NULL;
	}
	else
		return (VDECONN *)newconn;
}

static ssize_t vde_n2nEdge_recv(VDECONN *conn,void *buf,size_t len,int flags) {
	//struct vde_n2n_conn *vde_con = (struct vde_n2n_conn *)conn;
	//edge_read_from_tap(vde_con->eee);
	return (ssize_t)NULL;
}

static ssize_t vde_n2nEdge_send(VDECONN *conn,const void *buf,size_t len,int flags) {
	//struct vde_n2n_conn *vde_con = (struct vde_n2n_conn *)conn;
	//edge_send_packet2net(vde_con->eee, (uint8_t *)buf, len);
	return (ssize_t)NULL;
}

static int vde_n2nEdge_datafd(VDECONN *conn) {
	struct vde_n2n_conn *vde_con = (struct vde_n2n_conn *)conn;
//	return edge_get_userdata(vde_con->eee);
	return -1;
}

static int vde_n2nEdge_ctlfd(VDECONN *conn) {
	return -1;
}

static int vde_n2nEdge_close(VDECONN *conn) {

	struct vde_n2n_conn *vde_con = (struct vde_n2n_conn *)conn;
	edge_term(vde_con->eee);
	tuntap_close(vde_con->tuntap);
	return rc;
}








