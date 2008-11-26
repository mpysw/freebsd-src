/*-
 * Copyright (c) 2003-2004 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project in part by Network
 * Associates Laboratories, the Security Research Division of Network
 * Associates, Inc. under DARPA/SPAWAR contract N66001-01-C-8035 ("CBOSS"),
 * as part of the DARPA CHATS research program.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/security/mac/mac_sysv_msg.c,v 1.2.12.1 2008/10/02 02:57:24 kensmith Exp $");

#include "opt_mac.h"

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/mac.h>
#include <sys/sbuf.h>
#include <sys/systm.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/file.h>
#include <sys/namei.h>
#include <sys/sysctl.h>
#include <sys/msg.h>

#include <sys/mac_policy.h>

#include <security/mac/mac_internal.h>

static int	mac_enforce_sysv_msg = 1;
SYSCTL_INT(_security_mac, OID_AUTO, enforce_sysv_msg, CTLFLAG_RW,
    &mac_enforce_sysv_msg, 0,
    "Enforce MAC policy on System V IPC Message Queues");
TUNABLE_INT("security.mac.enforce_sysv_msg", &mac_enforce_sysv_msg);

#ifdef MAC_DEBUG
static unsigned int nmacipcmsgs, nmacipcmsqs;
SYSCTL_UINT(_security_mac_debug_counters, OID_AUTO, ipc_msgs, CTLFLAG_RD,
    &nmacipcmsgs, 0, "number of sysv ipc messages inuse");
SYSCTL_UINT(_security_mac_debug_counters, OID_AUTO, ipc_msqs, CTLFLAG_RD,
    &nmacipcmsqs, 0, "number of sysv ipc message queue identifiers inuse");
#endif

static struct label *
mac_sysv_msgmsg_label_alloc(void)
{
	struct label *label;

	label = mac_labelzone_alloc(M_WAITOK);
	MAC_PERFORM(init_sysv_msgmsg_label, label);
	MAC_DEBUG_COUNTER_INC(&nmacipcmsgs);
	return (label);
}

void
mac_init_sysv_msgmsg(struct msg *msgptr)
{

	msgptr->label = mac_sysv_msgmsg_label_alloc();
}

static struct label *
mac_sysv_msgqueue_label_alloc(void)
{
	struct label *label;

	label = mac_labelzone_alloc(M_WAITOK);
	MAC_PERFORM(init_sysv_msgqueue_label, label);
	MAC_DEBUG_COUNTER_INC(&nmacipcmsqs);
	return (label);
}

void
mac_init_sysv_msgqueue(struct msqid_kernel *msqkptr)
{

	msqkptr->label = mac_sysv_msgqueue_label_alloc();
}

static void
mac_sysv_msgmsg_label_free(struct label *label)
{

	MAC_PERFORM(destroy_sysv_msgmsg_label, label);
	mac_labelzone_free(label);
	MAC_DEBUG_COUNTER_DEC(&nmacipcmsgs);
}

void
mac_destroy_sysv_msgmsg(struct msg *msgptr)
{

	mac_sysv_msgmsg_label_free(msgptr->label);
	msgptr->label = NULL;
}

static void
mac_sysv_msgqueue_label_free(struct label *label)
{

	MAC_PERFORM(destroy_sysv_msgqueue_label, label);
	mac_labelzone_free(label);
	MAC_DEBUG_COUNTER_DEC(&nmacipcmsqs);
}

void
mac_destroy_sysv_msgqueue(struct msqid_kernel *msqkptr)
{

	mac_sysv_msgqueue_label_free(msqkptr->label);
	msqkptr->label = NULL;
}

void
mac_create_sysv_msgmsg(struct ucred *cred, struct msqid_kernel *msqkptr, 
    struct msg *msgptr)
{
				
	MAC_PERFORM(create_sysv_msgmsg, cred, msqkptr, msqkptr->label, 
		msgptr, msgptr->label);
}

void
mac_create_sysv_msgqueue(struct ucred *cred, struct msqid_kernel *msqkptr)
{
				
	MAC_PERFORM(create_sysv_msgqueue, cred, msqkptr, msqkptr->label);
}

void
mac_cleanup_sysv_msgmsg(struct msg *msgptr)
{

	MAC_PERFORM(cleanup_sysv_msgmsg, msgptr->label);
}

void
mac_cleanup_sysv_msgqueue(struct msqid_kernel *msqkptr)
{
				
	MAC_PERFORM(cleanup_sysv_msgqueue, msqkptr->label);
}

int
mac_check_sysv_msgmsq(struct ucred *cred, struct msg *msgptr,
	struct msqid_kernel *msqkptr)
{
	int error;

	if (!mac_enforce_sysv_msg)
		return (0);

	MAC_CHECK(check_sysv_msgmsq, cred,  msgptr, msgptr->label, msqkptr,
	    msqkptr->label);

	return(error);
}

int
mac_check_sysv_msgrcv(struct ucred *cred, struct msg *msgptr)
{
	int error;

	if (!mac_enforce_sysv_msg)
		return (0);

	MAC_CHECK(check_sysv_msgrcv, cred, msgptr, msgptr->label);

	return(error);
}

int
mac_check_sysv_msgrmid(struct ucred *cred, struct msg *msgptr)
{
	int error;

	if (!mac_enforce_sysv_msg)
		return (0);

	MAC_CHECK(check_sysv_msgrmid, cred,  msgptr, msgptr->label);

	return(error);
}

int
mac_check_sysv_msqget(struct ucred *cred, struct msqid_kernel *msqkptr)
{
	int error;

	if (!mac_enforce_sysv_msg)
		return (0);

	MAC_CHECK(check_sysv_msqget, cred, msqkptr, msqkptr->label);

	return(error);
}

int
mac_check_sysv_msqsnd(struct ucred *cred, struct msqid_kernel *msqkptr)
{
	int error;

	if (!mac_enforce_sysv_msg)
		return (0);

	MAC_CHECK(check_sysv_msqsnd, cred, msqkptr, msqkptr->label);

	return(error);
}

int
mac_check_sysv_msqrcv(struct ucred *cred, struct msqid_kernel *msqkptr)
{
	int error;

	if (!mac_enforce_sysv_msg)
		return (0);

	MAC_CHECK(check_sysv_msqrcv, cred, msqkptr, msqkptr->label);

	return(error);
}

int
mac_check_sysv_msqctl(struct ucred *cred, struct msqid_kernel *msqkptr,
    int cmd)
{
	int error;

	if (!mac_enforce_sysv_msg)
		return (0);

	MAC_CHECK(check_sysv_msqctl, cred, msqkptr, msqkptr->label, cmd);

	return(error);
}
