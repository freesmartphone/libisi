#include <glib.h>
#include "opcodes/network.h"
#include "gisi/client.h"

struct network_status {
	enum net_reg_status status;
	guint16 lac;
	guint16 cid;
	enum net_technology technology;
};

struct network_data {
	GIsiClient *client;
	guint8 last_reg_mode;
	guint8 rat;
	guint8 gsm_compact;
	void (*status_callback)(struct network_status *status, void *user_data);
	void (*strength_callback)(guint8 strength, void *user_data);
	void *user_data;
};

/* subsystem */
struct network_data* isi_network_create(GIsiModem *idx);
void isi_network_destroy(struct network_data *nd);

inline void network_set_user_data(struct network_data *nd, void *user_data) {
	nd->user_data = user_data;
}

inline void network_set_status_cb(struct network_data *nd, void (*status_callback)(struct network_status *status, void *user_data)) {
	nd->status_callback = status_callback;
}

inline void network_set_strength_cb(struct network_data *nd, void (*strength_callback)(guint8 strength, void *user_data)) {
	nd->strength_callback = strength_callback;
}

/* status */
gboolean isi_network_request_status(struct network_data *nd);
gboolean isi_network_subscribe_status(struct network_data *nd);
void isi_network_unsubscribe_status(struct network_data *nd);

/* strength */
gboolean isi_network_request_strength(struct network_data *nd);
gboolean isi_network_subscribe_strength(struct network_data *nd);
gboolean isi_network_unsubscribe_strength(struct network_data *nd);
