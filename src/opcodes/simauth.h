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

#ifndef __ISIMODEM_SIM_AUTH_H
#define __ISIMODEM_SIM_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

#define PN_SIM_AUTH		0x08

#define SIM_AUTH_TIMEOUT	5

#define SIM_MAX_PIN_LENGTH	8
#define SIM_MAX_PUK_LENGTH	8

enum sim_auth_message_id {
	SIM_AUTHENTICATION_REQ = 0x07,
	SIM_AUTHENTICATION_SUCCESS_RESP = 0x08,
	SIM_AUTHENTICATION_FAIL_RESP = 0x09
};

enum sim_auth_req_sub_id {
	SIM_AUTHENTICATION_REQ_PIN = 0x02,
	SIM_AUTHENTICATION_REQ_PUK = 0x03
};

enum sim_auth_errror_id {
	SIM_AUTHENTICATION_ERROR_INVALID_PW = 0x06,
	SIM_AUTHENTICATION_ERROR_NEED_PUK   = 0x18
};

#ifdef __cplusplus
};
#endif

#endif /* __ISIMODEM_SIM_AUTH_H */
