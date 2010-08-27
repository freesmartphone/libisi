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

void register_callback(bool error, void *user_data) {
	if(!error)
		message("You are now in Auto Register mode");
	else
		warning("Switching to Auto Registration mode failed");
}

void status_callback(bool error, Network.status status, void *user_data) {
	if(!error) {
		message("STATUS: LAC: %d, CID: %d", status.lac, status.cid);
	} else {
		warning("ERROR: Network Status failed!");
	}
}

void strength_callback(bool error, uint8 strength, void *user_data) {
	if(!error) {
		message("Signal Strength: %d", strength);
	} else {
		warning("ERROR: Network Strength failed!");
	}
}

void operator_callback(bool error, Network.operator op, void *user_data) {
	if(!error) {
		message("Current Provider: Name: %s, MCC: %s, MNC: %s", op.name, op.mcc, op.mnc);
	} else {
		warning("ERROR: Could not get information about current Network operator");
	}
}

void network_reachable(bool error, void *data) {
	if(!error) {
		/* the following commands are all working, but will spam your terminal */

		//n.subscribe_status(status_callback, data);
		//n.subscribe_strength(strength_callback, data);
		//n.list_operators(operator_list_callback, data);
		//n.register_auto(register_callback, data);
		//n.request_status(status_callback, data);
		//n.request_strength(strength_callback, data);
		//n.current_operator(operator_callback, data);
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
