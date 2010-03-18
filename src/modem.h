#include "gisi/client.h"

#ifndef ISI_MODEM_H
#define ISI_MODEM_H

struct isi_modem {
	GIsiModem *idx;
	GIsiClient *client;
	void (*powerstatus)(bool state);
};

typedef void (*isi_subsystem_reachable_cb)(bool error, void *data);

int isi_modem_create(struct isi_modem *modem);
void isi_modem_destroy(struct isi_modem *modem);
int isi_modem_enable(struct isi_modem *modem);
int isi_modem_disable(struct isi_modem *modem);

#endif
