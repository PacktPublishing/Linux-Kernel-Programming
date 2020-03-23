/*
 netlink_simple_intf.c

 ***************************************************************
 * Brief Description:
 *
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include "../../../convenient.h"

MODULE_AUTHOR("<insert your name here>");
MODULE_DESCRIPTION("ch12/netlink_simple_intf: simple netlink recv/send demo kernel module");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define OURMODNAME   "netlink_simple_intf"
#define NETLINK_MY_UNIT_PROTO 31  // kernel netlink protocol # that we're registering..

static struct sock *nlsock;

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
	pr_info("%s: received from PID %d:\n"
		"\"%s\"\n",
		OURMODNAME, pid, (char *)NLMSG_DATA(nlh));

	//--- Lets be polite and reply
	msgsz = strlen(reply);
	skb_tx = nlmsg_new(msgsz, 0);
	if (!skb_tx) {
		pr_warn("skb alloc failed!\n");
		return;
	}

	// Setup the payload
	nlh = nlmsg_put(skb_tx, 0, 0, NLMSG_DONE, msgsz, 0);
	NETLINK_CB(skb_tx).dst_group = 0;  // unicast only (cb is the
		// skb's control buffer, dest group 0 => unicast
	strncpy(nlmsg_data(nlh), reply, msgsz);

	// Send it
	stat = nlmsg_unicast(nlsock, skb_tx, pid);
	if (stat < 0)
		pr_warn("%s: nlmsg_unicast() failed (err=%d)\n",
			OURMODNAME, stat);
	pr_info("%s: reply sent\n", OURMODNAME);
}

static struct netlink_kernel_cfg nl_kernel_cfg = {
	.input = netlink_recv_and_reply,
};

static int __init netlink_simple_intf_init(void)
{
	pr_info("%s: creating kernel netlink socket\n", OURMODNAME);

	/* struct sock *
	 * netlink_kernel_create(struct net *net, int unit,
	 *	struct netlink_kernel_cfg *cfg) */
	nlsock = netlink_kernel_create(&init_net, NETLINK_MY_UNIT_PROTO,
		&nl_kernel_cfg);
	if (!nlsock) {
		pr_warn("%s: netlink_kernel_create failed\n", OURMODNAME);
		return PTR_ERR(nlsock);
	}

	pr_info("%s: inserted\n", OURMODNAME);
	return 0;		/* success */
}

static void __exit netlink_simple_intf_exit(void)
{
	netlink_kernel_release(nlsock);
	pr_info("%s: removed\n", OURMODNAME);
}

module_init(netlink_simple_intf_init);
module_exit(netlink_simple_intf_exit);
