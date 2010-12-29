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
	[CCode (cname = "isi_subsystem_reachable_cb", cheader_filename = "isi/modem.h")]
	public delegate void subsystem_reachable(bool error);

	/**
	 * GSM modem
	 */
	[CCode (cname = "struct isi_modem", free_function = "isi_modem_destroy", cheader_filename = "isi/modem.h")]
	[Compact]
	public class Modem {
		[CCode (cname = "isi_powerstatus_cb")]
		public delegate void powerstatus_cb(bool power);

		/**
		 * Create modem class
		 * @param interface interface name (e.g. "phonet0")
		 * @param cb callback informing about (un-)successful creation
		 */
		[CCode (cname = "isi_modem_create")]
		public Modem(string iface, subsystem_reachable cb);

		/**
		 * callback will be called if powerstatus changes. This will
		 * overwrite the previous callback.
		 * @param cb callback being called on powerstatus changes
		 */
		[CCode (cname = "isi_modem_set_powerstatus_notification")]
		public void set_powerstatus_cb(powerstatus_cb cb);

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
		[CCode (cname = "net_reg_status", cprefix = "NET_REG_STATUS_")]
		public enum RegistrationStatus {
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
		public enum Technology {
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
		[CCode (cname = "struct network_status")]
		public struct Status {
			/**
			 * Network Registration Status
			 */
			RegistrationStatus status;
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
			Technology technology;
		}

		/**
		 * Data send to network operator callback
		 */
		[CCode (cname = "struct network_operator")]
		public struct Operator {
			/**
			 * Network Operator Name
			 */
			string name;
			/**
			 * Mobile Country Code
			 */
			string mcc;
			/**
			 * Mobile Network Code
			 */
			string mnc;
			/**
			 * Status Code (not set by current_operator method)
			 */
			int status;
		}

		/**
		 * Create network GSM subsystem
		 */
		[CCode (cname = "isi_network_create")]
		public Network(Modem modem, subsystem_reachable cb);

		[CCode (cname = "isi_network_status_cb")]
		public delegate void status_cb(bool error, Status status);

		[CCode (cname = "isi_network_strength_cb")]
		public delegate void strength_cb(bool error, uint8 strength);

		[CCode (cname = "isi_network_register_cb")]
		public delegate void register_cb(bool error);

		[CCode (cname = "isi_network_operator_cb")]
		public delegate void operator_cb(bool error, Operator operator);

		[CCode (cname = "isi_network_operator_list_cb")]
		public delegate void operator_list_cb(bool error, Operator[] operators);

		/**
		 * Request to send notification for current
		 * network status.
		 */
		[CCode (cname = "isi_network_request_status")]
		public void request_status(status_cb cb);

		/**
		 * Subscribe to status changing notifications
		 * Overwrites previous set callback
		 */
		[CCode (cname = "isi_network_subscribe_status")]
		public void subscribe_status(status_cb cb);

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
		public void request_strength(strength_cb cb);

		/**
		 * Subscribe to strength changing notifications
		 * Overwrites previous set callback
		 */
		[CCode (cname = "isi_network_subscribe_strength")]
		public void subscribe_strength(strength_cb cb);

		/**
		 * Unsubscribe from strength changing notifications
		 */
		[CCode (cname = "isi_network_unsubscribe_strength")]
		public void unsubscribe_strength();

		/**
		 * Register to a specific operator
		 */
		[CCode (cname = "isi_network_register_manual")]
		public void register_manual(string mcc, string mnc, register_cb cb);

		/**
		 * Register to a specific operator
		 */
		[CCode (cname = "isi_network_register_auto")]
		public void register_auto(register_cb cb);

		/**
		 * Deregister from the network (''not yet implemented'')
		 */
		[CCode (cname = "isi_network_deregister")]
		public void deregister(register_cb cb);

		/**
		 * Get Information about the current operator
		 */
		[CCode (cname = "isi_network_current_operator")]
		public void current_operator(operator_cb cb);

		/**
		 * Get Information about all nearby operators
		 */
		[CCode (cname = "isi_network_list_operators")]
		public void list_operators(operator_list_cb cb);
	}

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
		public DeviceInfo(Modem modem, subsystem_reachable cb);

		[CCode (cname = "isi_device_info_cb")]
		public delegate void device_info_cb(bool error, string msg);

		/**
		 * Query Manufacturer
		 */
		[CCode (cname = "isi_device_info_query_manufacturer")]
		public void query_manufacturer(device_info_cb cb);

		/**
		 * Query Model
		 */
		[CCode (cname = "isi_device_info_query_model")]
		public void query_model(device_info_cb cb);

		/**
		 * Query Revision
		 */
		[CCode (cname = "isi_device_info_query_revision")]
		public void query_revision(device_info_cb cb);

		/**
		 * Query Serial (IMEI)
		 */
		[CCode (cname = "isi_device_info_query_serial")]
		public void query_serial(device_info_cb cb);
	}

	/**
	 * The SIM subsystem of the GSM modem (''not yet implemented'')
	 */
	[CCode (cname = "struct isi_sim", free_function = "isi_sim_destroy", cheader_filename = "isi/sim.h")]
	[Compact]
	public class SIM {
		/**
		 * Create SIM card GSM subsystem
		 */
		[CCode (cname = "isi_sim_create")]
		public SIM(Modem modem, subsystem_reachable cb);
	}

	/**
	 * The SIM Authentication subsystem of the GSM modem
	 */
	[CCode (cname = "struct isi_sim_auth", free_function = "isi_sim_auth_destroy", cheader_filename = "isi/simauth.h")]
	[Compact]
	public class SIMAuth {
		[CCode (cname = "IsiSimAuthAnswer", cheader_filename = "isi/isi-enum-types.h", has_type_id="ISI_SIM_AUTH_ANSWER_TYPE")]
		public enum Answer {
			OK,
			ERR_UNKNOWN,
			ERR_PIN_TOO_LONG,
			ERR_PUK_TOO_LONG,
			ERR_INVALID,
			ERR_NEED_PUK
		}

		[CCode (cname = "IsiSimAuthStatus", cheader_filename = "isi/isi-enum-types.h", has_type_id="ISI_SIM_AUTH_STATUS_TYPE")]
		public enum Status {
			ERROR,
			NO_SIM,
			NEED_NONE,
			NEED_PIN,
			NEED_PUK,
			VALID_PIN,
			VALID_PUK,
			INVALID_PIN,
			INVALID_PUK,
			AUTHORIZED,
			INITIALIZING,
			PROTECTED,
			UNPROTECTED
		}

		/**
		 * Create SIM auth GSM subsystem
		 */
		[CCode (cname = "isi_sim_auth_create")]
		public SIMAuth(Modem modem);

		[CCode (cname = "isi_sim_auth_cb")]
		public delegate void auth_cb(Answer code);

		[CCode (cname = "isi_sim_auth_status_cb")]
		public delegate void auth_status_cb(Status code);

		/**
		 * Set PIN code
		 * @param pin The PIN code
		 * @param cb The callback with the status feedback
		 */
		[CCode (cname = "isi_sim_auth_set_pin")]
		public void set_pin(string pin, auth_cb cb);

		/**
		 * Set PUK code
		 * @param puk The PUK code
		 * @param pin The new PIN code
		 * @param cb The callback with the status feedback
		 */
		[CCode (cname = "isi_sim_auth_set_puk")]
		public void set_puk(string puk, string pin, auth_cb cb);

		/**
		 * Change the PIN code
		 * @param old_pin The old PIN code
		 * @param new_pin The new PIN code
		 * @param cb The callback with the status feedback
		 */
		[CCode (cname = "isi_sim_auth_update_pin")]
		public void update_pin(string old_pin, string new_pin, auth_cb cb);

		/**
		 * Get PIN Protection Status
		 * @param cb The callback with the status feedback
		 */
		[CCode (cname = "isi_sim_auth_get_pin_protection")]
		public void get_pin_protection(auth_status_cb cb);

		/**
		 * Set PIN Protection Status
		 * @param pin The pin to access this feature
		 * @param status true=enable the protection, false=disable it
		 * @param cb The callback with the status feedback
		 */
		[CCode (cname = "isi_sim_auth_set_pin_protection")]
		public void set_pin_protection(string pin, bool status, auth_status_cb cb);

		/**
		 * Request SIM Auth status
		 * @param cb The callback with the status feedback
		 */
		[CCode (cname = "isi_sim_auth_request_status")]
		public void request_status(auth_status_cb cb);

		/**
		 * Subscribe to status changing notifications
		 * Overwrites previous set callback
		 */
		[CCode (cname = "isi_sim_auth_subscribe_status")]
		public void subscribe_status(auth_status_cb cb);

		/**
		 * Unsubscribe from status changing notifications
		 */
		[CCode (cname = "isi_sim_auth_unsubscribe_status")]
		public void unsubscribe_status();
	}

	/**
	 * The PDP subsystem of the GSM modem (''not yet implemented'')
	 */
	public class GPDS { }

	/**
	 * The Call subsystem of the GSM modem (''not yet implemented'')
	 */
	public class Call { }

	/**
	 * The SMS subsystem of the GSM modem (''not yet implemented'')
	 */
	public class SMS { }

	/**
	 * The USSD subsystem of the GSM modem (''not yet implemented'')
	 */
	public class USSD { }

	/**
	 * The GPS subsystem of the GSM modem (''not yet implemented'')
	 */
	public class GPS { }
}
