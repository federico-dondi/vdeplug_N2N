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

	cap_t caps;
	caps = caps_get_proc();
	return NULL;
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




