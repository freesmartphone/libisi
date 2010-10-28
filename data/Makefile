include ../config.mk

all:
	@echo "Nothing to compile"

install:
	@echo Install vapi file...
	@$(CREATE_DIR) $(DESTDIR)$(PREFIX)/share/vala/vapi
	@$(INSTALL_DATA) libisi.vapi $(DESTDIR)$(PREFIX)/share/vala/vapi
	@echo Install pkg-config file...
	@$(CREATE_DIR) $(DESTDIR)$(PREFIX)/lib/pkgconfig
	@$(INSTALL_DATA) libisi.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig

uninstall:
	@echo Uninstall vapi file...
	@rm -rf $(DESTDIR)$(PREFIX)/share/vala/vapi/libisi.vapi
	@echo Uninstall pkg-config file...
	@rm -rf $(DESTDIR)$(PREFIX)/lib/pkgconfig/libisi.pc

.PHONY: all install
