/*
 * Copyright (c) 2010, Sebastian Reichel <sre@ring0.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

using ISI;

Modem m;
Network n;

void operator_list_callback(bool error, Network.operator[] operators) {
	if(!error) {
		for(int i=0; i<operators.length; i++)
			message("operator %d: %s - %s - %s", i, operators[i].name, operators[i].mcc, operators[i].mnc);
	} else {
		warning("Could not get operator list");
	}
}

void register_callback(bool error) {
	if(!error)
		message("You are now in Auto Register mode");
	else
		warning("Switching to Auto Registration mode failed");
}

void status_callback(bool error, Network.status status) {
	if(!error) {
		message("STATUS: LAC: %d, CID: %d", status.lac, status.cid);
	} else {
		warning("ERROR: Network Status failed!");
	}
}

void strength_callback(bool error, uint8 strength) {
	if(!error) {
		message("Signal Strength: %d", strength);
	} else {
		warning("ERROR: Network Strength failed!");
	}
}

void operator_callback(bool error, Network.operator op) {
	if(!error) {
		message("Current Provider: Name: %s, MCC: %s, MNC: %s", op.name, op.mcc, op.mnc);
	} else {
		warning("ERROR: Could not get information about current Network operator");
	}
}

void network_reachable(bool error) {
	if(!error) {
		/* the following commands are all working, but will spam your terminal */

		n.subscribe_status(status_callback);
		n.subscribe_strength(strength_callback);
		n.list_operators(operator_list_callback);
		n.register_auto(register_callback);
		n.request_status(status_callback);
		n.request_strength(strength_callback);
		n.current_operator(operator_callback);
	} else {
		warning("Could not create network object");
	}
}

void modem_reachable(bool error) {
	stdout.printf("Modem Reachable Status: %s\n", error ? "down" : "up");

	if(!error) {
		m.enable();
		n = new Network(m, network_reachable);
	}
}

void main() {
	stdout.printf("N900 test utility\n");
	m = new Modem("phonet0", modem_reachable);
	var loop = new GLib.MainLoop();
	loop.run();
}
