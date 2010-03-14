#include "gisi/client.h"

struct isi_modem {
	GIsiModem *idx;
	GIsiClient *client;
	void (*powerstatus)(bool state);
};

int isi_modem_create(struct isi_modem *modem);
void isi_modem_destroy(struct isi_modem *modem);
int isi_modem_enable(struct isi_modem *modem);
int isi_modem_disable(struct isi_modem *modem);
