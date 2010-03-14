#include "gisi/client.h"

struct isi_modem {
	GIsiModem *idx;
	GIsiClient *client;
	void (*powerstatus)(bool state);
};

int isi_modem_probe(struct isi_modem *modem);
int isi_modem_enable(struct isi_modem *modem);
int isi_modem_disable(struct isi_modem *modem);
