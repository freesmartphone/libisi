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

/**
 * lowlevel interface to the GSM modem of the N900
 */
namespace ISI {
	/**
	 * Send as answer to the creation of a subsystem
	 */
	[CCode (cname = "isi_subsystem_reachable_cb", has_target = false, cheader_filename = "isi/modem.h")]
	public delegate void subsystem_reachable(bool error, void *data);

	/**
	 * GSM modem
	 */
	[CCode (cname = "struct isi_modem", free_function = "isi_modem_destroy", cheader_filename = "isi/modem.h")]
	[Compact]
	public class Modem {
		[CCode (cname = "isi_powerstatus_cb", has_target = false)]
		public delegate void powerstatus_cb(bool power, void *data);

		/**
		 * Create modem class
		 * @param interface interface name (e.g. "phonet0")
		 * @param cb callback informing about (un-)successful creation
		 * @param user_data user data being sent with the callback
		 */
		[CCode (cname = "isi_modem_create")]
		public Modem(char *interface, subsystem_reachable cb, void *user_data);

		/**
		 * callback will be called if powerstatus changes. This will
		 * overwrite the previous callback.
		 * @param cb callback being called on powerstatus changes
		 * @param user_data user data being sent with the callback
		 */
		[CCode (cname = "isi_modem_set_powerstatus_notification")]
		public void set_powerstatus_cb(powerstatus_cb cb, void *user_data);

		/**
		 * get current powerstatus
		 */
		[CCode (cname = "isi_modem_get_powerstatus")]
		public bool get_powerstatus();

		/**
		 * enable the modem
		 */
		[CCode (cname = "isi_modem_enable")]
		public void enable();

		/**
		 * disable the modem
		 */
		[CCode (cname = "isi_modem_disable")]
		public void disable();
	}

	/**
	 * The network subsystem of the GSM modem
	 */
	[CCode (cname = "struct isi_network", free_function = "isi_network_destroy", cheader_filename = "isi/network.h")]
	[Compact]
	public class Network {
		[CCode (cname = "net_reg_status")]
		public enum registration_status {
			HOME = 0x00,
			ROAM = 0x01,
			ROAM_BLINK = 0x02,
			NOSERV = 0x03,
			NOSERV_SEARCHING = 0x04,
			NOSERV_NOTSEARCHING = 0x05,
			NOSERV_NOSIM = 0x06,
			POWER_OFF = 0x08,
			NSPS = 0x09,
			NSPS_NO_COVERAGE = 0x0A,
			NOSERV_SIM_REJECTED_BY_NW = 0x0B
		}

		[CCode (cname = "net_technology")]
		public enum technology {
			GSM = 0x00,
			GSM_COMPACT = 0x01,
			UMTS = 0x02,
			EPGRS = 0x03,
			HSDPA = 0x04,
			HSUPA = 0x05,
			HSPA = 0x06
		}

		/**
		 * Data send to network status callback
		 */
		[CCode (cname = "network_status")]
		public struct status {
			/**
			 * Network Registration Status
			 */
			registration_status status;
			/**
			 * Location Area Code
			 */
			uint16 lac;
			/**
			 * Cell ID
			 */
			uint16 cid;
			/**
			 * Technology
			 */
			technology technology;
		}

		/**
		 * Data send to network operator callback
		 */
		[CCode (cname = "network_operator")]
		public struct operator {
			/**
			 * Network Operator Name
			 */
			char *name;
			/**
			 * Mobile Country Code
			 */
			char *mcc;
			/**
			 * Mobile Network Code
			 */
			char *mnc;
			/**
			 * Status Code (not set by current_operator method)
			 */
			int status;
		}

		/**
		 * Create network GSM subsystem
		 */
		[CCode (cname = "isi_network_create")]
		public Network(Modem *modem, subsystem_reachable cb, void *user_data);

		[CCode (cname = "isi_network_status_cb", has_target = false)]
		public delegate void status_cb(bool error, status *status, void *user_data);

		[CCode (cname = "isi_network_strength_cb", has_target = false)]
		public delegate void strength_cb(bool error, uint8 strength, void *user_data);

		[CCode (cname = "isi_network_register_cb", has_target = false)]
		public delegate void register_cb(bool error, void *user_data);

		[CCode (cname = "isi_network_operator_cb", has_target = false)]
		public delegate void operator_cb(bool error, operator *operator, void *user_data);

		[CCode (cname = "isi_network_operator_list_cb", has_target = false)]
		public delegate void operator_list_cb(bool error, operator[] operators, void *user_data);

		/**
		 * Request to send notification for current
		 * network status.
		 */
		[CCode (cname = "isi_network_request_status")]
		public void request_status(status_cb cb, void *data);

		/**
		 * Subscribe to status changing notifications
		 * Overwrites previous set callback
		 */
		[CCode (cname = "isi_network_subscribe_status")]
		public void subscribe_status(status_cb cb, void *data);

		/**
		 * Unsubscribe from status changing notifications
		 */
		[CCode (cname = "isi_network_unsubscribe_status")]
		public void unsubscribe_status();

		/**
		 * Request to send notification for current
		 * signal strength.
		 */
		[CCode (cname = "isi_network_request_strength")]
		public void request_strength(strength_cb cb, void *data);

		/**
		 * Subscribe to strength changing notifications
		 * Overwrites previous set callback
		 */
		[CCode (cname = "isi_network_subscribe_strength")]
		public void subscribe_strength(strength_cb cb, void *data);

		/**
		 * Unsubscribe from strength changing notifications
		 */
		[CCode (cname = "isi_network_unsubscribe_strength")]
		public void unsubscribe_strength();

		/**
		 * Register to a specific operator
		 */
		[CCode (cname = "isi_network_register_manual")]
		public void register_manual(char *mcc, char *mnc, register_cb cb, void *data);

		/**
		 * Register to a specific operator
		 */
		[CCode (cname = "isi_network_register_auto")]
		public void register_auto(register_cb cb, void *data);

		/**
		 * Deregister from the network (''not yet implemented'')
		 */
		[CCode (cname = "isi_network_deregister")]
		public void deregister(register_cb cb, void *data);

		/**
		 * Get Information about the current operator
		 */
		[CCode (cname = "isi_network_current_operator")]
		public void current_operator(operator_cb cb, void *data);

		/**
		 * Get Information about all nearby operators
		 */
		[CCode (cname = "isi_network_list_operators")]
		public void list_operators(operator_list_cb cb, void *data);
	}

	/**
	 * The USSD subsystem of the GSM modem (''not yet implemented'')
	 */
	public class USSD { }

	/**
	 * The Device Information subsystem of the GSM modem
	 */
	[CCode (cname = "struct isi_device_info", free_function = "isi_device_info_destroy", cheader_filename = "isi/device_info.h")]
	[Compact]
	public class DeviceInfo {
		/**
		 * Create device information GSM subsystem
		 */
		[CCode (cname = "isi_device_info_create")]
		public DeviceInfo(Modem *modem, subsystem_reachable cb, void *user_data);

		[CCode (cname = "isi_device_info_cb", has_target = false)]
		public delegate void device_info_cb(bool error, string msg, void *user_data);

		/**
		 * Query Manufacturer
		 */
		[CCode (cname = "isi_device_info_query_manufacturer")]
		public void query_manufacturer(device_info_cb cb, void *data);

		/**
		 * Query Model
		 */
		[CCode (cname = "isi_device_info_query_model")]
		public void query_model(device_info_cb cb, void *data);

		/**
		 * Query Revision
		 */
		[CCode (cname = "isi_device_info_query_revision")]
		public void query_revision(device_info_cb cb, void *data);

		/**
		 * Query Serial (IMEI)
		 */
		[CCode (cname = "isi_device_info_query_serial")]
		public void query_serial(device_info_cb cb, void *data);
	}

	/**
	 * The SIM subsystem of the GSM modem (''not yet implemented'')
	 */
	[CCode (cname = "struct isi_sim", free_function = "isi_sim_destroy", cheader_filename = "isi/sim.h")]
	[Compact]
	public class SIM {
		[CCode (cname = "enum isi_sim_pin_answer")]
		public enum pin_answer {
			SIM_PIN_UNKNOWN_ERROR = 0x00,
			SIM_PIN_OK = 0x01,
			SIM_PIN_TOO_LONG = 0x02,
			SIM_PIN_INVALID = 0x03
		}

		/**
		 * Create SIM card GSM subsystem
		 */
		[CCode (cname = "isi_sim_create")]
		public SIM(Modem *modem, subsystem_reachable cb, void *user_data);

		[CCode (cname = "isi_sim_pin_cb", has_target = false)]
		public delegate void pin_cb(bool error, pin_answer code, void *user_data);

		/**
		 * Set PIN code
		 */
		[CCode (cname = "isi_sim_set_pin")]
		public void set_pin(string pin, pin_cb cb, void *data);
	}

	/**
	 * The Call subsystem of the GSM modem (''not yet implemented'')
	 */
	public class Call { }

	/**
	 * The SMS subsystem of the GSM modem (''not yet implemented'')
	 */
	public class SMS { }
}
