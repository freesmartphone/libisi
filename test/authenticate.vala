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
SIMAuth s;

void pin_callback(SIMAuth.answer msg, void *data) {
	if(msg == SIMAuth.answer.OK) {
		message("Valid PIN");
	} else {
		message("SIM Authentication Failed");
		message("Message: %s", msg.to_string());
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

void main(string[] argv) {
	stdout.printf("N900 test utility\n");
	m = new Modem("phonet0", modem_reachable, null);
	var loop = new GLib.MainLoop();
	loop.run();
}
