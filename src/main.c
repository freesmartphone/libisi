/*
 * Copyright (c) 2010 Sebastian Reichel
 * BSD License (2 clause)
 */
#include <stdio.h>
#include <glib.h>

#include "gisi/netlink.h"
#include "modem.h"
#include "debug.h"
#include "network.h"

static void netlink_status_cb(bool up, uint8_t addr, GIsiModem *idx, void *data) {
	g_debug("Phonet is %s, addr=0x%02x, idx=%p\n", up ? "up" : "down", addr, idx);
	if(up)
		isi_network_init(idx);
}

int main(int argc, char **argv) {
	GMainLoop *loop;
	GPhonetNetlink *link;

	/* Print Header, that everybody knows what this it */
	printf("N900 GSM-Modem Utility\n");
	printf("Sebastian Reichel <sre@ring0.de>\n\n");

	g_log_set_handler (NULL, G_LOG_LEVEL_MASK, print_log, NULL);

	/* check modems */
	link = g_pn_netlink_start(netlink_status_cb, "phonet0", NULL);

	/* Idle, waiting for the callbacks */
	loop = g_main_loop_new(NULL, false);
	g_main_loop_run(loop);
}
