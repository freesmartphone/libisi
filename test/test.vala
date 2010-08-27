/*
 * Copyright Sebastian Reichel <sre@ring0.de>
 * License: BSD (2 clause)
 */

using ISI;

Modem m;
SIMAuth s;
Network n;

void operator_list_callback(bool error, Network.operator[] operators, void *user_data) {
	if(!error) {
		for(int i=0; i<operators.length; i++)
			message("operator %d: %s - %s - %s", i, operators[i].name, operators[i].mcc, operators[i].mnc);
	} else {
		warning("Could not get operator list");
	}
}

void network_reachable(bool error, void *data) {
	if(!error) {
		n.list_operators(operator_list_callback, data);
	} else {
		warning("Could not create network object");
	}
}

void pin_callback(SIMAuth.auth_answer msg, void *data) {
	if(msg == 0) {
		message("Valid PIN, trying to register to Network");
		n = new Network(m, network_reachable, data);
	} else {
		message("Invalid PIN");
		message("Code: %d", msg);
	}
}

void modem_reachable(bool error, void *data) {
	stdout.printf("Modem Reachable Status: %s\n", error ? "down" : "up");

	if(!error) {
		m.enable();
		s = new SIMAuth(m);
		s.set_pin("5336", pin_callback, null);
	}
}

void main() {
	stdout.printf("N900 test utility\n");
	m = new Modem("phonet0", modem_reachable, null);
	var loop = new GLib.MainLoop();
	loop.run();
}
