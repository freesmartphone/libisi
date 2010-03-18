/*
 * Copyright Sebastian Reichel <sre@ring0.de>
 * License: BSD (2 clause)
 */

using ISI;

Modem m;
Network n;

void strength_callback(bool error, uint8 strength, void *data) {
	if(!error)
		message("Strength: %d", strength);
}

void network_reachable(bool error, void *data) {
	message("Network Reachable Status: %s", error ? "down" : "up");
	if(!error) {
		n.request_strength(strength_callback, data);
	}
}

void modem_reachable(bool error, void *data) {
	message("Modem Reachable Status: %s", error ? "down" : "up");
	if(!error) {
		n = new Network(m, network_reachable, data);
	}
}

void main() {
	m = new Modem("phonet0", modem_reachable, null);
	var loop = new GLib.MainLoop();
	loop.run();
}
