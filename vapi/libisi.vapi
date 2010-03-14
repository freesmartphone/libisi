/*
 * This file is under BSD license (2 clause)
 * Copyright (C) 2010 Sebastian Reichel
 */

/**
 * ISI is a lowlevel interface to the GSM modem of the N900
 */
namespace ISI {
	/**
	 * The Modem object represents the whole GSM modem
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
	 * FIXME: define enums and use them here
	 */
	public struct network_status {
		int status;
		uint16 lac;
		uint16 cid;
		int technology;
	}

	/**
	 * The Network object represents the network subsystem of the GSM modem
	 */
	[CCode (cname = "isi_network", free_function = "isi_network_destroy")]
	public class Network {
		/**
		 * FIXME: idx is not int* but GIsiModem*
		 */
		[CCode (cname = "isi_network_create")]
		public Network(int *idx);

		[CCode (has_target = false)]
		public delegate void status_callback(network_status *status, void *user_data);

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
		 */
		[CCode (cname = "isi_network_request_status")]
		public bool request_status();

		/**
		 * Subscribe to status changing notifications
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
		 */
		[CCode (cname = "isi_network_request_strength")]
		public bool request_strength();

		/**
		 * Subscribe to strength changing notifications
		 */
		[CCode (cname = "isi_network_subscribe_strength")]
		public bool subscribe_strength();

		/**
		 * Unsubscribe from strength changing notifications
		 */
		[CCode (cname = "isi_network_unsubscribe_strength")]
		public void unsubscribe_strength();
	}

	public class USSD { }

	public class Device { }

	public class SIM { }

	public class Call { }

	public class SMS { }
}
