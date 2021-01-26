#include <libvdeplug_mod.h>
#include <n2n.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define NTOP_BUFSIZE 128
#define N2N_CONFIG_FILE "/etc/n2n/super.conf"

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

// ##########################################################

static VDECONN *vde_n2n_super_open(char *sockname,char *descr,int interface_version,
		struct vde_open_args *open_args) {
	printf("vde_n2n_super_open\n");
    struct vde_n2n_super_conn *newconn = NULL;
    n2n_sn_t sss_node;

    sn_init(&sss_node);
        sss_node.daemon = 0;   // Whether to daemonize
        sss_node.lport = 1234; // Main UDP listen port

        sss_node.sock = open_socket(sss_node.lport, 1);
        if (-1 == sss_node.sock)
        {
            exit(-2);
        }

        sss_node.mgmt_sock = open_socket(5645, 0); // Main UDP management port
        if (-1 == sss_node.mgmt_sock)
        {
            exit(-2);
        }

        keep_running = 1;

	    newconn = calloc(1, sizeof(*newconn));
	    newconn->sss_node = &sss_node;
	    newconn->data = sss_node.sock;

	    // Run supernode loop
//	    print_n2n_version();
	    if (fork() == 0) {
			printf("vde_n2n_super_open, child entering loop\n");
			rc = run_sn_loop(&sss_node, &keep_running);
			printf("vde_n2n_super_open, child exiting loop\n");
			return NULL;
		}
		else {
			printf("vde_n2n_super_open, father returning\n");
			return (VDECONN *)newconn;
		}


}

static ssize_t vde_n2n_super_recv(VDECONN *conn,void *buf,size_t len,int flags) {
	printf("vde_n2n_super_recv\n");
	struct vde_n2n_super_conn *vde_con = (struct vde_n2n_super_conn *)conn;
	//return sendto_sock(vde_con->sss_node, vde_con->sock, buf, len);
	return NULL;

}

static ssize_t vde_n2n_super_send(VDECONN *conn,const void *buf,size_t len,int flags) {

	printf("vde_n2n_super_send\n");
	return NULL;

//	struct vde_n2n_super_conn *vde_con = (struct vde_n2n_super_conn *)conn;
//	return sendto_sock(vde_con->sss_node, vde_con->sss_node->sock, buf, len);
}

static int vde_n2n_super_datafd(VDECONN *conn) {

	printf("vde_n2n_super_datafd\n");
	struct vde_n2n_super_conn *vde_con = (struct vde_n2n_super_conn *)conn;

	return conn->data;
}

static int vde_n2n_super_ctlfd(VDECONN *conn) {
	struct vde_n2n_super_conn *vde_con = (struct vde_n2n_super_conn *)conn;
	printf("vde_n2n_super_ctlfd\n");
//	return -1;
	return vde_con->data;
}

static int vde_n2n_super_close(VDECONN *conn) {

	printf("vde_n2n_super_close\n");
	struct vde_n2n_super_conn *vde_con = (struct vde_n2n_super_conn *)conn;
	sn_term(vde_con->sss_node);
//	keep_running = 0;
	return rc;
}



