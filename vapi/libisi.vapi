/*
 * This file is under BSD license (2 clause)
 * Copyright (C) 2010 Sebastian Reichel
 */

/**
 * lowlevel interface to the GSM modem of the N900
 */
namespace ISI {
	/**
	 * GSM modem
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
		 * Create network GSM subsystem
		 */
		[CCode (cname = "isi_network_create")]
		public Network(GIsiModem *idx);

		[CCode (has_target = false)]
		public delegate void status_callback(status *status, void *user_data);

		[CCode (has_target = false)]
		public delegate void strength_callback(uint8 strength, void *user_data);

		/**
		 * Set user_data, which is delivered in the callbacks
		 */
		[CCode (cname = "isi_network_set_user_data")]
		public void set_user_data(void *user_data);

		/**
		 * Set callback for strength notifications
		 */
		[CCode (cname = "isi_network_set_strength_cb")]
		public void set_strength_cb(strength_callback cb);

		/**
		 * Set callback for status notifications
		 */
		[CCode (cname = "isi_network_set_status_cb")]
		public void set_status_cb(status_callback cb);

		/**
		 * Request to send notification for current
		 * network status.
		 * @return true on sucess, false on failure
		 */
		[CCode (cname = "isi_network_request_status")]
		public bool request_status();

		/**
		 * Subscribe to status changing notifications
		 * @return true on sucess, false on failure
		 */
		[CCode (cname = "isi_network_subscribe_status")]
		public bool subscribe_status();

		/**
		 * Unsubscribe to status changing notifications
		 */
		[CCode (cname = "isi_network_unsubscribe_status")]
		public void unsubscribe_status();

		/**
		 * Request to send notification for current
		 * signal strength.
		 * @return true on sucess, false on failure
		 */
		[CCode (cname = "isi_network_request_strength")]
		public bool request_strength();

		/**
		 * Subscribe to strength changing notifications
		 * @return true on sucess, false on failure
		 */
		[CCode (cname = "isi_network_subscribe_strength")]
		public bool subscribe_strength();

		/**
		 * Unsubscribe from strength changing notifications
		 */
		[CCode (cname = "isi_network_unsubscribe_strength")]
		public void unsubscribe_strength();
	}

	/**
	 * The USSD subsystem of the GSM modem
	 */
	public class USSD { }

	/**
	 * The Device subsystem of the GSM modem
	 */
	public class Device { }

	/**
	 * The SIM subsystem of the GSM modem
	 */
	public class SIM { }

	/**
	 * The Call subsystem of the GSM modem
	 */
	public class Call { }

	/**
	 * The SMS subsystem of the GSM modem
	 */
	public class SMS { }
}
