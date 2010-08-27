/*
 * This file is GPLv2
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2010 Sebastian Reichel
 */
#include <glib.h>
#include <stdio.h>

#include "opcodes/network.h"
#include "opcodes/info.h"
#include "opcodes/ss.h"
#include "opcodes/call.h"
#include "opcodes/sms.h"
#include "opcodes/sim.h"
#include "opcodes/simauth.h"
#include "opcodes/mtc.h"
#include "opcodes/gss.h"

#define _(X) case X: return #X

void print_package(const gchar *name, const unsigned char *msg, size_t len) {
	int i;

	printf("PKG: %s: ", name);
	for(i=0; i<len; i++)
		printf("%02x", msg[i]);
	printf("\n");
}

void print_log(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data) {
	switch(log_level) {
		case G_LOG_LEVEL_ERROR:
			printf("[ERR]  "); break;
		case G_LOG_LEVEL_CRITICAL:
			printf("[CRIT] "); break;
		case G_LOG_LEVEL_WARNING:
			printf("[WARN] "); break;
		case G_LOG_LEVEL_MESSAGE:
			printf("[MESG] "); break;
		case G_LOG_LEVEL_INFO:
			printf("[INFO] "); break;
		case G_LOG_LEVEL_DEBUG:
			printf("[DBG]  "); break;
	}

	printf("%s\n", message);
}

const char *pn_resource_name(int value) {
	switch(value) {
		_(PN_NETWORK);
		_(PN_PHONE_INFO);
		_(PN_SS);
		_(PN_CALL);
		_(PN_SMS);
		_(PN_SIM);
		_(PN_SIM_AUTH);
		_(PN_MTC);
		_(PN_GSS);
	}
	return "PN_UNKNOWN";
}

const char *net_status_name(enum net_reg_status value) {
	switch (value) {
		_(NET_REG_STATUS_HOME);
		_(NET_REG_STATUS_ROAM);
		_(NET_REG_STATUS_ROAM_BLINK);
		_(NET_REG_STATUS_NOSERV);
		_(NET_REG_STATUS_NOSERV_SEARCHING);
		_(NET_REG_STATUS_NOSERV_NOTSEARCHING);
		_(NET_REG_STATUS_NOSERV_NOSIM);
		_(NET_REG_STATUS_POWER_OFF);
		_(NET_REG_STATUS_NSPS);
		_(NET_REG_STATUS_NSPS_NO_COVERAGE);
		_(NET_REG_STATUS_NOSERV_SIM_REJECTED_BY_NW);
	}
	return "NET_UNKNOWN";
}

const char *net_subblock_name(enum net_subblock value) {
	switch (value) {
		_(NET_REG_INFO_COMMON);
		_(NET_OPERATOR_INFO_COMMON);
		_(NET_RSSI_CURRENT);
		_(NET_GSM_REG_INFO);
		_(NET_DETAILED_NETWORK_INFO);
		_(NET_GSM_OPERATOR_INFO);
		_(NET_GSM_BAND_INFO);
		_(NET_RAT_INFO);
		_(NET_AVAIL_NETWORK_INFO_COMMON);
		_(NET_OPER_NAME_INFO);
	}
	return "NET_UNKNOWN";
}

const char *net_isi_cause_name(enum net_isi_cause value) {
	switch (value) {
		_(NET_CAUSE_OK);
		_(NET_CAUSE_COMMUNICATION_ERROR);
		_(NET_CAUSE_INVALID_PARAMETER);
		_(NET_CAUSE_NO_SIM);
		_(NET_CAUSE_SIM_NOT_YET_READY);
		_(NET_CAUSE_NET_NOT_FOUND);
		_(NET_CAUSE_REQUEST_NOT_ALLOWED);
		_(NET_CAUSE_CALL_ACTIVE);
		_(NET_CAUSE_SERVER_BUSY);
		_(NET_CAUSE_SECURITY_CODE_REQUIRED);
		_(NET_CAUSE_NOTHING_TO_CANCEL);
		_(NET_CAUSE_UNABLE_TO_CANCEL);
		_(NET_CAUSE_NETWORK_FORBIDDEN);
		_(NET_CAUSE_REQUEST_REJECTED);
		_(NET_CAUSE_CS_NOT_SUPPORTED);
		_(NET_CAUSE_PAR_INFO_NOT_AVAILABLE);
		_(NET_CAUSE_NOT_DONE);
		_(NET_CAUSE_NO_SELECTED_NETWORK);
		_(NET_CAUSE_REQUEST_INTERRUPTED);
		_(NET_CAUSE_TOO_BIG_INDEX);
		_(NET_CAUSE_MEMORY_FULL);
		_(NET_CAUSE_SERVICE_NOT_ALLOWED);
		_(NET_CAUSE_NOT_SUPPORTED_IN_TECH);
	}
	return "NET_UNKNOWN";
}

const char *mtc_modem_state_name(enum mtc_modem_state value) {
	switch (value) {
		_(MTC_POWER_OFF);
		_(MTC_NORMAL);
		_(MTC_CHARGING);
		_(MTC_ALARM);
		_(MTC_TEST);
		_(MTC_LOCAL);
		_(MTC_WARRANTY);
		_(MTC_RELIABILITY);
		_(MTC_SELFTEST_FAIL);
		_(MTC_SWDL);
		_(MTC_RF_INACTIVE);
		_(MTC_ID_WRITE);
		_(MTC_DISCHARGING);
		_(MTC_DISK_WIPE);
		_(MTC_SW_RESET);
		_(MTC_CMT_ONLY_MODE);
	}
	return "MTC_UNKNOWN";
}

const char *isi_device_info_subblock_name(enum info_subblock value) {
	switch (value) {
		_(INFO_SB_PRODUCT_INFO_NAME);
		_(INFO_SB_PRODUCT_INFO_MANUFACTURER);
		_(INFO_SB_SN_IMEI_PLAIN);
		_(INFO_SB_SN_IMEI_SV_TO_NET);
		_(INFO_SB_MCUSW_VERSION);
	}
	return "INFO_<UNKNOWN>";
}

const char *isi_device_info_isi_cause_name(enum info_isi_cause value) {
	switch (value) {
		_(INFO_OK);
		_(INFO_FAIL);
		_(INFO_NO_NUMBER);
		_(INFO_NOT_SUPPORTED);
	}
	return "INFO_<UNKNOWN>";
}
