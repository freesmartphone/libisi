/*
 * Copyright Sebastian Reichel <sre@ring0.de>
 * License: BSD (2 clause)
 */

using ISI;

Modem m;
DeviceInfo n;
SIM s;

void info_callback(bool error, string msg, void *data) {
	if(!error)
		stdout.printf("%s: %s\n", (string) data, msg);
}

void pin_callback(bool error, ISI.SIM.pin_answer msg, void *data) {
	//if(!error)
	//	stdout.printf("%s: %s\n", (string) data, msg);
}

void dev_info_reachable(bool error, void *data) {
	stdout.printf("Device Info Reachable Status: %s\n", error ? "down" : "up");
	if(!error) {
		n.query_manufacturer(info_callback, "manufacturer");
		n.query_model(info_callback, "model");
		n.query_revision(info_callback, "revision");
	}
}

void sim_reachable(bool error, void *data) {
	stdout.printf("SIM Reachable Status: %s\n", error ? "down" : "up");
	if(!error) {
		s.set_pin("1234", pin_callback, null);
	}
}


void modem_reachable(bool error, void *data) {
	stdout.printf("Modem Reachable Status: %s\n", error ? "down" : "up");
	if(!error) {
		//n = new DeviceInfo(m, dev_info_reachable, data);
		s = new SIM(m, sim_reachable, data);
	}
}

void main() {
	stdout.printf("N900 test utility\n");
	m = new Modem("phonet0", modem_reachable, null);
	var loop = new GLib.MainLoop();
	loop.run();
}
