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

/* ######################
 * N2N Public functions
 * ######################
 *
 * n2n_edge_t* edge_init(const tuntap_dev *dev, const n2n_edge_conf_t *conf, int *rv);
 * void edge_term(n2n_edge_t *eee);
 * void edge_set_callbacks(n2n_edge_t *eee, const n2n_edge_callbacks_t *callbacks);
 * void edge_set_userdata(n2n_edge_t *eee, void *user_data);
 * void* edge_get_userdata(n2n_edge_t *eee);
 * void edge_send_packet2net(n2n_edge_t *eee, uint8_t *tap_pkt, size_t len);
 * void edge_read_from_tap(n2n_edge_t *eee);
 * int edge_get_n2n_socket(n2n_edge_t *eee);
 * int edge_get_management_socket(n2n_edge_t *eee);
 * int run_edge_loop(n2n_edge_t *eee, int *keep_running);
 * int quick_edge_init(char *device_name, char *community_name,
 * 		    char *encrypt_key, char *device_mac,
 * 		    char *local_ip_address,
 * 		    char *supernode_ip_address_port,
 * 		    int *keep_on_running);
 * int sn_init(n2n_sn_t *sss);
 * void sn_term(n2n_sn_t *sss);
 * int run_sn_loop(n2n_sn_t *sss, int *keep_running);
 * const char* compression_str(uint8_t cmpr);
 * const char* transop_str(enum n2n_transform tr);
 *
 * ######################
 * Edge Conf
 * ######################
 *
 * typedef struct n2n_edge_conf {
 *  n2n_sn_name_t       sn_ip_array[N2N_EDGE_NUM_SUPERNODES];
 *  n2n_route_t	      *routes;		      /**< Networks to route through n2n
 *  n2n_community_t     community_name;         /**< The community. 16 full octets.
 *  uint8_t	      header_encryption;      /**< Header encryption indicator.
 *  he_context_t	      *header_encryption_ctx; /**< Header encryption cipher context.
 *  he_context_t        *header_iv_ctx;	      /**< Header IV ecnryption cipher context, REMOVE as soon as seperte fileds for checksum and replay protection available
 *  n2n_transform_t     transop_id;             /**< The transop to use.
 *  uint16_t	      compression;	      /**< Compress outgoing data packets before encryption
 *  uint16_t	      num_routes;	      /**< Number of routes in routes
 *  uint8_t             dyn_ip_mode;            /**< Interface IP address is dynamically allocated, eg. DHCP.
 *  uint8_t             allow_routing;          /**< Accept packet no to interface address.
 *  uint8_t             drop_multicast;         /**< Multicast ethernet addresses.
 *  uint8_t             disable_pmtu_discovery; /**< Disable the Path MTU discovery.
 *  uint8_t             allow_p2p;              /**< Allow P2P connection
 *  uint8_t             sn_num;                 /**< Number of supernode addresses defined.
 *  uint8_t             tos;                    /** TOS for sent packets
 *  char                *encrypt_key;
 *  int                 register_interval;      /**< Interval for supernode registration, also used for UDP NAT hole punching.
 *  int                 register_ttl;           /**< TTL for registration packet when UDP NAT hole punching through supernode. d
 *  int                 local_port;
 *  int                 mgmt_port;
 * } n2n_edge_conf_t;
 *
 * ######################
 * Edge Stats
 * ######################
 * struct n2n_edge_stats {
 *        uint32_t tx_p2p;
 *        uint32_t rx_p2p;
 *        uint32_t tx_sup;
 *        uint32_t rx_sup;
 *        uint32_t tx_sup_broadcast;
 *        uint32_t rx_sup_broadcast;
 * };
 *
 * ######################
 * Edge struct
 * ######################
 * struct n2n_edge {
 * 	n2n_edge_conf_t     conf;
 * 
 * 	/* Status
 * 	uint8_t             sn_idx;                 /**< Currently active supernode.
 * 	uint8_t             sn_wait;                /**< Whether we are waiting for a supernode response.
 * 	size_t              sup_attempts;           /**< Number of remaining attempts to this supernode.
 * 	tuntap_dev          device;                 /**< All about the TUNTAP device
 * 	n2n_trans_op_t      transop;                /**< The transop to use when encoding
 * 	n2n_cookie_t        last_cookie;            /**< Cookie sent in last REGISTER_SUPER.
 * 	n2n_route_t         *sn_route_to_clean;     /**< Supernode route to clean
 * 	n2n_edge_callbacks_t cb;	            /**< API callbacks
 * 	void 	            *user_data;             /**< Can hold user data
 *         uint64_t            sn_last_valid_time_stamp;/*< last valid time stamp from supernode
 * 
 * 	/* Sockets
 * 	n2n_sock_t          supernode;
 * 	int                 udp_sock;
 * 	int                 udp_mgmt_sock;          /**< socket for status info.
 * 
 * #ifndef SKIP_MULTICAST_PEERS_DISCOVERY
 * 	n2n_sock_t          multicast_peer;         /**< Multicast peer group (for local edges)
 * 	int                 udp_multicast_sock;     /**< socket for local multicast registrations.
 * 	int                 multicast_joined;       /**< 1 if the group has been joined.
 * #endif
 * 
 * 	/* Peers
 * 	struct peer_info *  known_peers;            /**< Edges we are connected to.
 * 	struct peer_info *  pending_peers;          /**< Edges we have tried to register with.
 * 
 * 	/* Timers
 * 	time_t              last_register_req;      /**< Check if time to re-register with super
 * 	time_t              last_p2p;               /**< Last time p2p traffic was received.
 * 	time_t              last_sup;               /**< Last time a packet arrived from supernode.
 * 	time_t              start_time;             /**< For calculating uptime
 * 
 * 	/* Statistics
 * 	struct n2n_edge_stats stats;
 *   /* Tuntap config
 *   n2n_tuntap_priv_config_t tuntap_priv_conf;
 * };
##########################################################
 */

static VDECONN *vde_n2nEdge_open(char *sockname,char *descr,int interface_version,
		struct vde_open_args *open_args) {


		n2n_edge_conf_t conf;
	    tuntap_dev tuntap;
	    n2n_edge_t *eee;
	    int rc;
	    
	    /*
	     * TODO Implementare argument passing
	     */


	    char
				*tapname = NULL,
	    		*ipmode = NULL,
				*ipaddr = NULL,
				*netmask = NULL,
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
				{"secret", &secret},
				{"snodeport", &snodeport},
				{"snodeaddr", &snodeaddr},
				{"cfgfile", &cfgfile},
	    		{NULL, NULL}};


	    // Utility per parsare i parametri
	    memset(&hints, 0, sizeof(struct addrinfo));

		if (vde_parseparms(sockname, parms) != 0)
			return NULL;
		// Se viene specificato un file di configurazione
		if (*cfgfile != NULL) {
			// Parse the file
//			parseFile(cfgfile, &conf);

		}
		// @@@@@@@@@@@@ DEBUG @@@@@@@@@@@@@@
		for (int i=0; i < 10; i++) {
			if (*parms[i].value != NULL)
				printf("%s: %s\n", parms[i].tag, *parms[i].value);
		}
		// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	    char snode[256];
		snprintf(snode, sizeof snode, "%s:%s", snodeaddr, snodeport);

	    //return NULL;
	    // ##################################


	    edge_init_conf_defaults(&conf);
	    conf.allow_p2p = 1;                                                                      // Whether to allow peer-to-peer communication
	    conf.allow_routing = 1;                                                                  // Whether to allow the edge to route packets to other edges
	    snprintf((char *)conf.community_name, sizeof(conf.community_name), "%s", community); // Community to connect to
	    conf.disable_pmtu_discovery = 1;                                                         // Whether to disable the path MTU discovery
	    conf.drop_multicast = 0;                                                                 // Whether to disable multicast
	    conf.dyn_ip_mode = 0;                                                                    // Whether the IP address is set dynamically (see IP mode; 0 if static, 1 if dynamic)
	    conf.encrypt_key = secret;                                                           // Secret to decrypt & encrypt with
	    conf.local_port = 0;                                                                     // What port to use (0 = any port)
	    conf.mgmt_port = N2N_EDGE_MGMT_PORT;                                                     // Edge management port (5644 by default)
	    conf.register_interval = 1;                                                              // Interval for both UDP NAT hole punching and supernode registration
	    conf.register_ttl = 1;                                                                   // Interval for UDP NAT hole punching through supernode
	    

	    edge_conf_add_supernode(&conf, snode);                                        		 // Supernode to connect to
	    conf.tos = 16;                                                                           // Type of service for sent packets
	    conf.transop_id = N2N_TRANSFORM_ID_TWOFISH;                                              // Use the twofish encryption

	    if (edge_verify_conf(&conf) != 0)
	    {
	        return -1;
	    }
	    // Try to create a new TUNTAP interface
	    if (tuntap_open(&tuntap,
	                    tapname,    		// Name of the device to create
	                    ipmode,     		// IP mode; static|dhcp
	                    ipaddr,     		// Set ip address
						netmask,    		// Netmask to use
	                    mac, 				// Set mac address
	                    DEFAULT_MTU) < 0)   // MTU to use
	    {
	        return -1;
	    }

	    eee = edge_init(&tuntap, &conf, &rc);
	    if (eee == NULL)
	    {
	        exit(1);
	    }

	    keep_running = 1;
	    // Run the edge routine
	    rc = run_edge_loop(eee, &keep_running);

	    edge_term(eee);
	    // Remove the TUNTAP interface
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

static void parseConf(char *file, n2n_edge_conf_t *conf) {
	
	FILE *fp;
	char buf[255];

	fp = fopen(file, "r");
	fgets(buf, sizeof(buf), (FILE*)fp);
	printf("%s\n", buf );
	fclose(fp);


	return;
}




