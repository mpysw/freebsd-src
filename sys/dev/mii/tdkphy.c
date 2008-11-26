/*-
 * Copyright (c) 2000,2001 Jonathan Chen.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/sys/dev/mii/tdkphy.c,v 1.16.2.4.4.1 2008/10/02 02:57:24 kensmith Exp $");

/*
 * Driver for the TDK 78Q2120 MII
 *
 * References:
 *   Datasheet for the 78Q2120 - http://www.tsc.tdk.com/lan/78q2120.pdf
 *   Most of this code stolen from ukphy.c
 */

/*
 * The TDK 78Q2120 is found on some Xircom X3201 based cardbus cards.  It's just
 * like any other normal phy, except it does auto negotiation in a different
 * way.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <net/if.h>
#include <net/if_media.h>

#include <machine/clock.h>

#include <dev/mii/mii.h>
#include <dev/mii/miivar.h>
#include "miidevs.h"

#include <dev/mii/tdkphyreg.h>

#include "miibus_if.h"

static int tdkphy_probe(device_t);
static int tdkphy_attach(device_t);

static device_method_t tdkphy_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		tdkphy_probe),
	DEVMETHOD(device_attach,	tdkphy_attach),
	DEVMETHOD(device_detach,	mii_phy_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	{ 0, 0 }
};

static devclass_t tdkphy_devclass;

static driver_t tdkphy_driver = {
	"tdkphy",
	tdkphy_methods,
	sizeof(struct mii_softc)
};

DRIVER_MODULE(tdkphy, miibus, tdkphy_driver, tdkphy_devclass, 0, 0);

static int tdkphy_service(struct mii_softc *, struct mii_data *, int);
static void tdkphy_status(struct mii_softc *);

static const struct mii_phydesc tdkphys[] = {
	MII_PHY_DESC(TDK, 78Q2120),
	MII_PHY_END
};

static int
tdkphy_probe(device_t dev)
{

	return (mii_phy_dev_probe(dev, tdkphys, BUS_PROBE_DEFAULT));
}

static int
tdkphy_attach(device_t dev)
{
	struct mii_softc *sc;
	struct mii_attach_args *ma;
	struct mii_data *mii;
	sc = device_get_softc(dev);
	ma = device_get_ivars(dev);
	sc->mii_dev = device_get_parent(dev);
	mii = device_get_softc(sc->mii_dev);
	LIST_INSERT_HEAD(&mii->mii_phys, sc, mii_list);

	if (bootverbose)
		device_printf(dev, "OUI 0x%06x, model 0x%04x, rev. %d\n",
		    MII_OUI(ma->mii_id1, ma->mii_id2),
		    MII_MODEL(ma->mii_id2), MII_REV(ma->mii_id2));

	sc->mii_inst = mii->mii_instance;
	sc->mii_phy = ma->mii_phyno;
	sc->mii_service = tdkphy_service;
	sc->mii_pdata = mii;

	mii->mii_instance++;

	/*
	 * Apparently, we can't do loopback on this PHY.
	 */
	sc->mii_flags |= MIIF_NOLOOP;

	mii_phy_reset(sc);

	sc->mii_capabilities =
	    PHY_READ(sc, MII_BMSR) & ma->mii_capmask;
	device_printf(dev, " ");
	mii_phy_add_media(sc);
	printf("\n");

	MIIBUS_MEDIAINIT(sc->mii_dev);
	return (0);
}

static int
tdkphy_service(struct mii_softc *sc, struct mii_data *mii, int cmd)
{
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int reg;

	switch (cmd) {
	case MII_POLLSTAT:
		/*
		 * If we're not polling our PHY instance, just return.
		 */
		if (IFM_INST(ife->ifm_media) != sc->mii_inst)
			return (0);
		break;

	case MII_MEDIACHG:
		/*
		 * If the media indicates a different PHY instance,
		 * isolate ourselves.
		 */
		if (IFM_INST(ife->ifm_media) != sc->mii_inst) {
			reg = PHY_READ(sc, MII_BMCR);
			PHY_WRITE(sc, MII_BMCR, reg | BMCR_ISO);
			return (0);
		}

		/*
		 * If the interface is not up, don't do anything.
		 */
		if ((mii->mii_ifp->if_flags & IFF_UP) == 0)
			break;

		mii_phy_setmedia(sc);
		break;

	case MII_TICK:
		/*
		 * If we're not currently selected, just return.
		 */
		if (IFM_INST(ife->ifm_media) != sc->mii_inst)
			return (0);
		if (mii_phy_tick(sc) == EJUSTRETURN)
			return (0);
		break;
	}

	/* Update the media status. */
	tdkphy_status(sc);
	if (sc->mii_pdata->mii_media_active & IFM_FDX)
		PHY_WRITE(sc, MII_BMCR, PHY_READ(sc, MII_BMCR) | BMCR_FDX);
	else
		PHY_WRITE(sc, MII_BMCR, PHY_READ(sc, MII_BMCR) & ~BMCR_FDX);

	/* Callback if something changed. */
	mii_phy_update(sc, cmd);
	return (0);
}

static void
tdkphy_status(struct mii_softc *phy)
{
	struct mii_data *mii = phy->mii_pdata;
	struct ifmedia_entry *ife = mii->mii_media.ifm_cur;
	int bmsr, bmcr, anlpar, diag;

	mii->mii_media_status = IFM_AVALID;
	mii->mii_media_active = IFM_ETHER;

	bmsr = PHY_READ(phy, MII_BMSR) | PHY_READ(phy, MII_BMSR);
	if (bmsr & BMSR_LINK)
		mii->mii_media_status |= IFM_ACTIVE;

	bmcr = PHY_READ(phy, MII_BMCR);
	if (bmcr & BMCR_ISO) {
		mii->mii_media_active |= IFM_NONE;
		mii->mii_media_status = 0;
		return;
	}

	if (bmcr & BMCR_LOOP)
		mii->mii_media_active |= IFM_LOOP;

	if (bmcr & BMCR_AUTOEN) {
		/*
		 * NWay autonegotiation takes the highest-order common
		 * bit of the ANAR and ANLPAR (i.e. best media advertised
		 * both by us and our link partner).
		 */
		if ((bmsr & BMSR_ACOMP) == 0) {
			/* Erg, still trying, I guess... */
			mii->mii_media_active |= IFM_NONE;
			return;
		}

		anlpar = PHY_READ(phy, MII_ANAR) & PHY_READ(phy, MII_ANLPAR);
		/*
		 * ANLPAR doesn't get set on my card, but we check it anyway,
		 * since it is mentioned in the 78Q2120 specs.
		 */
		if (anlpar & ANLPAR_T4)
			mii->mii_media_active |= IFM_100_T4;
		else if (anlpar & ANLPAR_TX_FD)
			mii->mii_media_active |= IFM_100_TX|IFM_FDX;
		else if (anlpar & ANLPAR_TX)
			mii->mii_media_active |= IFM_100_TX;
		else if (anlpar & ANLPAR_10_FD)
			mii->mii_media_active |= IFM_10_T|IFM_FDX;
		else if (anlpar & ANLPAR_10)
			mii->mii_media_active |= IFM_10_T;
		else {
			/*
			 * ANLPAR isn't set, which leaves two possibilities:
			 * 1) Auto negotiation failed
			 * 2) Auto negotiation completed, but the card forgot
			 *    to set ANLPAR.
			 * So we check the MII_DIAG(18) register...
			 */
			diag = PHY_READ(phy, MII_DIAG);
			if (diag & DIAG_NEGFAIL) /* assume 10baseT if no neg */
				mii->mii_media_active |= IFM_10_T;
			else {
				if (diag & DIAG_DUPLEX)
					mii->mii_media_active |= IFM_FDX;
				if (diag & DIAG_RATE_100)
					mii->mii_media_active |= IFM_100_TX;
				else
					mii->mii_media_active |= IFM_10_T;
			}
		}
	} else
		mii->mii_media_active = ife->ifm_media;
}
