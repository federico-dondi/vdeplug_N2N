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
#define N2N_SUPER_CONFIG_FILE "/etc/n2n/supernode.conf"
#define VDEPLUG_SUPER_MAX_PARAMS 1

// VDE functions

static VDECONN *vde_n2n_super_open(char *sockname, char *descr,int interface_version,
		struct vde_open_args *open_args);
static ssize_t vde_n2n_super_recv(VDECONN *conn,void *buf,size_t len,int flags);
static ssize_t vde_n2n_super_send(VDECONN *conn,const void *buf,size_t len,int flags);
static int vde_n2n_super_datafd(VDECONN *conn);
static int vde_n2n_super_ctlfd(VDECONN *conn);
static int vde_n2n_super_close(VDECONN *conn);

struct vdeplug_module vdeplug_ops = {
		.vde_open_real=vde_n2n_super_open,
		.vde_recv=vde_n2n_super_recv,
		.vde_send=vde_n2n_super_send,
		.vde_datafd=vde_n2n_super_datafd,
		.vde_ctlfd=vde_n2n_super_ctlfd,
		.vde_close=vde_n2n_super_close

};

struct vde_n2n_super_conn {
	void *handle;
	struct vdeplug_module *module;
	void *data;
	n2n_sn_t *sss_node;
};

static int keep_running;
static int rc;

static VDECONN *vde_n2n_super_open(char *sockname,char *descr,int interface_version,
		struct vde_open_args *open_args) {

	n2n_sn_t sss_node;

	struct vde_n2n_super_conn *newconn = NULL;

	char
		*lport = "1234", 	// Default is 1234
		*cfgfile = NULL;

	struct addrinfo hints;
	struct vdeparms parms[] = {
			{"lport", &lport},
			{"cfgfile", &cfgfile},
			{NULL, NULL}};


	memset(&hints, 0, sizeof(struct addrinfo));

	if (vde_parseparms(sockname, parms) != 0)
		return NULL;

	if (*cfgfile != NULL && strcmp(cfgfile, "yes") == 0) {
		parseConf(N2N_SUPER_CONFIG_FILE, &parms, VDEPLUG_SUPER_MAX_PARAMS);
	}
// Useful for debugging
//	for (int i=0; i < 2; i++) {
//		if (*parms[i].value != NULL)
//			printf("%s: %s\n", parms[i].tag, *parms[i].value);
//	}

	sn_init(&sss_node);
	sss_node.daemon = 0;   // Whether to daemonize, always false
	sss_node.lport = 1234; // Main UDP listen port

	sss_node.sock = open_socket(sss_node.lport, 1);


	if (-1 == sss_node.sock) {
		exit(-2);
	}
	// Function defined in n2n, informative logging
	traceEvent(TRACE_NORMAL, "supernode is listening on UDP %u (main)", sss_node.lport);

	sss_node.mgmt_sock = open_socket(5645, 0); // Main UDP management port
	
	if (-1 == sss_node.mgmt_sock) {
		exit(-2);
	}
	// Function defined in n2n, informative logging
	traceEvent(TRACE_NORMAL, "supernode is listening on UDP %u (management)", sss_node.mport);


	keep_running = 1;
	// Allocating vdeconn struct
	newconn = calloc(1, sizeof(*newconn));
	newconn->sss_node = &sss_node;

	// Run supernode loop, tricky sol.
	if (fork() == 0) {
		rc = run_sn_loop(&sss_node, &keep_running);
		return NULL;
	}
	else {
		return (VDECONN *)newconn;
	}

}

static ssize_t vde_n2n_super_recv(VDECONN *conn,void *buf,size_t len,int flags) {
	return (ssize_t)NULL;
}

static ssize_t vde_n2n_super_send(VDECONN *conn,const void *buf,size_t len,int flags) {
	return (ssize_t)NULL;
}

static int vde_n2n_super_datafd(VDECONN *conn) {
	return 0;
}

static int vde_n2n_super_ctlfd(VDECONN *conn) {
	return -1;
}

static int vde_n2n_super_close(VDECONN *conn) {

	struct vde_n2n_super_conn *vde_con = (struct vde_n2n_super_conn *)conn;
	sn_term(vde_con->sss_node);
	return rc;
}



