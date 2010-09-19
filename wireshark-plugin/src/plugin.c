#include <gmodule.h>

#ifndef ENABLE_STATIC
G_MODULE_EXPORT const gchar version[] = "0.0.1";

extern void proto_register_isi(void);
extern void proto_reg_handoff_isi(void);

G_MODULE_EXPORT void plugin_register (void) {
	proto_register_isi();
}

G_MODULE_EXPORT void plugin_reg_handoff(void) {
	proto_reg_handoff_isi();
}
#endif
