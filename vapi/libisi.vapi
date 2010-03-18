/*
 * This file is under BSD license (2 clause)
 * Copyright (C) 2010 Sebastian Reichel
 */

/**
 * lowlevel interface to the GSM modem of the N900
 */
namespace ISI {
	/**
	 * Send as answer to the creation of a subsystem
	 */
	[CCode (has_target = false)]
	public delegate void subsystem_reachable(bool error, void *data);

	/**
	 * GSM modem (''WIP'')
	 */
	[CCode (cname = "isi_modem", free_function = "isi_modem_free")]
	public class Modem {
		[CCode (cname = "isi_modem_create")]
		public Modem();

		[CCode (cname = "isi_modem_enable")]
		public void enable();

		[CCode (cname = "isi_modem_disable")]
		public void disable();
	}

	/**
	 * FIXME: not sure if this will stay in the vapi binding
	 * or if it will be abstracted by the Modem class
	 */
	[CCode (cname = "GIsiModem")]
	public struct GIsiModem { }

	/**
	 * The network subsystem of the GSM modem
	 */
	[CCode (cname = "isi_network", free_function = "isi_network_destroy")]
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
		public Network(GIsiModem *idx, subsystem_reachable cb, void *user_data);

		[CCode (has_target = false)]
		public delegate void status_callback(status *status, void *user_data);

		[CCode (has_target = false)]
		public delegate void strength_callback(uint8 strength, void *user_data);

		[CCode (has_target = false)]
		public delegate void register_cb(bool error, void *user_data);

		[CCode (has_target = false)]
		public delegate void operator_cb(bool error, operator *operator, void *user_data);

		[CCode (has_target = false)]
		public delegate void operator_list_cb(bool error, operator[] operators, void *user_data);

		/**
		 * Request to send notification for current
		 * network status.
		 */
		[CCode (cname = "isi_network_request_status")]
		public bool request_status(status_callback cb, void *data);

		/**
		 * Subscribe to status changing notifications
		 * Overwrites previous set callback
		 */
		[CCode (cname = "isi_network_subscribe_status")]
		public bool subscribe_status(status_callback cb, void *data);

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
		public void request_strength(strength_callback cb, void *data);

		/**
		 * Subscribe to strength changing notifications
		 * Overwrites previous set callback
		 */
		[CCode (cname = "isi_network_subscribe_strength")]
		public void subscribe_strength(strength_callback cb, void *data);

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
		 * Deregister from the network (FIXME: ''not yet implemented'')
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
	 * The Device subsystem of the GSM modem (''not yet implemented'')
	 */
	public class Device { }

	/**
	 * The SIM subsystem of the GSM modem (''not yet implemented'')
	 */
	public class SIM { }

	/**
	 * The Call subsystem of the GSM modem (''not yet implemented'')
	 */
	public class Call { }

	/**
	 * The SMS subsystem of the GSM modem (''not yet implemented'')
	 */
	public class SMS { }
}
