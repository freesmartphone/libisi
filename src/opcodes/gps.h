/*
 * Copyright (C) 2010, Sebastian Reichel <sre@ring0.de>
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

#ifndef __ISIMODEM_GPS_H
#define __ISIMODEM_GPS_H

#ifdef __cplusplus
extern "C" {
#endif

#define PN_GPS					0x54
#define GPS_TIMEOUT				5

enum gps_message_id {
	GPS_STATUS_IND	= 0x7d,
	GPS_STATUS_REQ	= 0x90,
	GPS_STATUS_RESP	= 0x91,
	GPS_DATA_IND	= 0x92
};

#ifdef __cplusplus
};
#endif

#endif /* !__ISIMODEM_GPS_H */
