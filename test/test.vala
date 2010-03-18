/*
 * Copyright Sebastian Reichel <sre@ring0.de>
 * License: BSD (2 clause)
 */

using ISI;

void modem_reachable(bool error, void *data) {
	message("Modem Reachable Status: %s", error ? "up" : "down");
}

void main() {
	new Modem("phonet0", modem_reachable, null);
}
