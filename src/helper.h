#include <stdlib.h>

struct isi_cb_data {
	void *subsystem;
	void *callback;
	void *data;
};

static inline struct isi_cb_data* isi_cb_data_new(void *subsystem, void *callback, void *data) {
	struct isi_cb_data *output = malloc(sizeof(struct isi_cb_data));
	output->subsystem = subsystem;
	output->callback  = callback;
	output->data      = data;
	return output;
}

static inline void isi_cb_data_free(struct isi_cb_data *data) {
	if(data)
		free(data);
}
