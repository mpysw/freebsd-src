/*-
 * Copyright (c) 1997, 1998, 1999, 2000
 *	Bill Paul <wpaul@ee.columbia.edu>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/dev/usb/usb_ethersubr.h,v 1.10.12.1 2008/10/02 02:57:24 kensmith Exp $
 */

#ifndef _USB_ETHERSUBR_H_
#define _USB_ETHERSUBR_H_

#include <sys/bus.h>
#include <sys/module.h>

#include <dev/usb/usbdi.h>

#define UE_TX_LIST_CNT		1
#define UE_RX_LIST_CNT		1
#define UE_BUFSZ		1536

struct usb_qdat {
	struct ifnet		*ifp;
	void (*if_rxstart)	(struct ifnet *);
};

struct ue_chain {
	void			*ue_sc;
	usbd_xfer_handle	ue_xfer;
	char			*ue_buf;
	struct mbuf		*ue_mbuf;
	int			ue_idx;
};

struct ue_cdata {
	struct ue_chain		ue_tx_chain[UE_TX_LIST_CNT];
	struct ue_chain		ue_rx_chain[UE_RX_LIST_CNT];
	void			*ue_ibuf;
	int			ue_tx_prod;
	int			ue_tx_cons;
	int			ue_tx_cnt;
	int			ue_rx_prod;
};

void usb_register_netisr	(void);
void usb_ether_input		(struct mbuf *);
void usb_tx_done		(struct mbuf *);
struct mbuf *usb_ether_newbuf	(void);
int usb_ether_rx_list_init	(void *, struct ue_cdata *,
    usbd_device_handle);
int usb_ether_tx_list_init	(void *, struct ue_cdata *,
    usbd_device_handle);
void usb_ether_rx_list_free	(struct ue_cdata *);
void usb_ether_tx_list_free	(struct ue_cdata *);

#endif /* _USB_ETHERSUBR_H_ */
