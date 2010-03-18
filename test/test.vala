/*
 * Copyright Sebastian Reichel <sre@ring0.de>
 * License: BSD (2 clause)
 */

using ISI;

Modem m;
Network n;

void strength_callback(bool error, uint8 strength, void *data) {
	if(!error)
		stdout.printf("Strength: %d\n", strength);
}

void network_reachable(bool error, void *data) {
	stdout.printf("Network Reachable Status: %s\n", error ? "down" : "up");
	if(!error) {
		n.request_strength(strength_callback, data);
	}
}

void modem_reachable(bool error, void *data) {
	stdout.printf("Modem Reachable Status: %s\n", error ? "down" : "up");
	if(!error) {
		n = new Network(m, network_reachable, data);
	}
}

void main() {
	stdout.printf("N900 test utility\n");
	m = new Modem("phonet0", modem_reachable, null);
	var loop = new GLib.MainLoop();
	loop.run();
}
