/*
 * user_kernel_comm/netlink_simple_intf/kernelspace_netlink/netlink_simple_intf.c
 **************************************************************************
 * This program is part of the source code released for the book
 *  "Learn Linux Kernel Development"
 *  (c) Author: Kaiwan N Billimoria
 *  Publisher:  Packt
 *  GitHub repository:
 *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
 *
 * From: Ch - User-Kernel communication pathways
 **************************************************************************
 * Brief Description:
 *
 */
#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include "../../../convenient.h"

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("user_kernel_comm/netlink_simple_intf: simple netlink recv/send demo kernel module");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define OURMODNAME   "netlink_simple_intf"
#define NETLINK_MY_UNIT_PROTO   31
	// kernel netlink protocol # that we're registering..
#define NLSPACE              1024

static struct sock *nlsock;

/*
 * netlink_recv_and_reply
 * When a userspace process (or thread) provides any input (i.e. transmits
 * something) to us, this callback function is invoked. It's important to
 * understand that it runs in process context (and not any kind of interrupt
 * context).
 * Here, we simply display the received 'message' and then reply by sending
 * a sample message to our userspace peer (process).
 */
static void netlink_recv_and_reply(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	struct sk_buff *skb_tx;
	char *reply = "Reply from kernel netlink";
	int pid, msgsz, stat;

	/* Find that this code runs in process context, the process
	 * (or thread) being the one that issued the sendmsg(2) */
	PRINT_CTX();

	nlh = (struct nlmsghdr *)skb->data;
	pid = nlh->nlmsg_pid;	/*pid of sending process */
	pr_info("received from PID %d:\n"
		"\"%s\"\n", pid, (char *)NLMSG_DATA(nlh));

	//--- Lets be polite and reply
	msgsz = strnlen(reply, NLSPACE);
	skb_tx = nlmsg_new(msgsz, 0);
	if (!skb_tx) {
		pr_warn("skb alloc failed!\n");
		return;
	}

	// Setup the payload
	nlh = nlmsg_put(skb_tx, 0, 0, NLMSG_DONE, msgsz, 0);
	NETLINK_CB(skb_tx).dst_group = 0;  /* unicast only (cb is the
						* skb's control buffer), dest group 0 => unicast */
	strncpy(nlmsg_data(nlh), reply, msgsz);

	// Send it
	stat = nlmsg_unicast(nlsock, skb_tx, pid);
	if (stat < 0)
		pr_warn("nlmsg_unicast() failed (err=%d)\n", stat);
	pr_info("reply sent\n");
}

static struct netlink_kernel_cfg nl_kernel_cfg = {
	.input = netlink_recv_and_reply,
};

static int __init netlink_simple_intf_init(void)
{
	pr_info("creating kernel netlink socket\n");

	/* struct sock *
	 * netlink_kernel_create(struct net *net, int unit,
	 *	struct netlink_kernel_cfg *cfg) */
	nlsock = netlink_kernel_create(&init_net, NETLINK_MY_UNIT_PROTO,
		&nl_kernel_cfg);
	if (!nlsock) {
		pr_warn("netlink_kernel_create failed\n");
		return PTR_ERR(nlsock);
	}

	pr_info("inserted\n");
	return 0;		/* success */
}

static void __exit netlink_simple_intf_exit(void)
{
	netlink_kernel_release(nlsock);
	pr_info("removed\n");
}

module_init(netlink_simple_intf_init);
module_exit(netlink_simple_intf_exit);
