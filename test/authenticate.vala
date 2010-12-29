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
string pin;

Modem m;
SIMAuth s;
MainLoop l;

void pin_callback(SIMAuth.Answer msg) {
	if(msg == SIMAuth.Answer.OK) {
		message("Valid PIN");
		l.quit();
	} else {
		message("SIM Authentication Failed");
		message("Message: %s", msg.to_string());
		l.quit();
	}
}

void modem_reachable(bool error) {
	message("Modem Reachable Status: %s", error ? "down" : "up");

	if(!error) {
		m.enable();
		s = new SIMAuth(m);
		s.set_pin(pin, pin_callback);
	} else {
		l.quit();
	}
}

void main(string[] argv) {
	stdout.printf("N900 test utility\n");

	if(argv.length < 2)
		error("Usage: %s <pin>", argv[0]);
	else
		pin = argv[1];

	m = new Modem("phonet0", modem_reachable);
	l = new GLib.MainLoop();
	l.run();
}
