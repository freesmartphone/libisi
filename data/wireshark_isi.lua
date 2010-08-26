-- Copyright (c) 2010, Sebastian Reichel <sre@ring0.de>
--
-- Permission to use, copy, modify, and/or distribute this software for any
-- purpose with or without fee is hereby granted, provided that the above
-- copyright notice and this permission notice appear in all copies.
--
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
--
--
-- This is a lua plugin for wireshark adding support for the Phonet/ISI protocol

phonet = Proto("phonet", "Phonet Protocol")
isi = Proto("isi", "ISI Modem Control Protocol")

local fields    = phonet.fields
local isifields = isi.fields

local device = { [0x00] = "Modem", [0x6c] = "Host", [0xFF] = "Any" }
local resource = {
	[0x01] = "Call",
	[0x02] = "SMS",
	[0x06] = "Subscriber Services",
	[0x08] = "SIM AUTH",
	[0x09] = "SIM",
	[0x0A] = "Network",
	[0x10] = "Indication",
	[0x15] = "MTC",
	[0x1B] = "Phone Info",
	[0x31] = "GPRS",
	[0x62] = "EPOC Info"
}

local mtc_msg_ids = {
	[0x02] = "MTC_STATE_QUERY_REQ",
	[0x03] = "MTC_POWER_OFF_REQ",
	[0x04] = "MTC_POWER_ON_REQ",
	[0x65] = "MTC_STATE_QUERY_RESP",
	[0x66] = "MTC_POWER_OFF_RESP",
	[0x67] = "MTC_POWER_ON_RESP",
	[0xC0] = "MTC_STATE_INFO_IND",
	[0xF0] = "MTC_COMMON_MESSAGE"
}

local phone_info_msg_ids = {
	[0x00] = "INFO_SERIAL_NUMBER_READ_REQ",
	[0x01] = "INFO_SERIAL_NUMBER_READ_RESP",
	[0x07] = "INFO_VERSION_READ_REQ",
	[0x08] = "INFO_VERSION_READ_RESP",
	[0x15] = "INFO_PRODUCT_INFO_READ_REQ",
	[0x16] = "INFO_PRODUCT_INFO_READ_RESP",
	[0xF0] = "INFO_COMMON_MESSAGE"
}

local common_msg_sub_ids = {
	[0x12] = "COMM_ISI_VERSION_GET_REQ",
	[0x13] = "COMM_ISI_VERSION_GET_RESP",
	[0x14] = "COMM_ISA_ENTITY_NOT_REACHABLE_RESP"
}

local phone_info_sub_ids = {
	[0x01] = "INFO_PRODUCT_INFO_NAME",
	[0x07] = "INFO_PRODUCT_INFO_MANUFACTURER",
}

local sim_auth_msg_ids = {
	[0x07] = "SIM_AUTH_REQ",
	[0x08] = "SIM_AUTH_SUCCESS_RESP",
	[0x09] = "SIM_AUTH_FAIL_RESP"
}

local sim_auth_req = {
	[0x02] = "SIM_AUTH_REQ_PIN",
	[0x03] = "SIM_AUTH_REQ_PUK"
}

fields.rdev = ProtoField.uint8("phonet.receiver", "Receiver Device ID", base.HEX, device)
fields.sdev = ProtoField.uint8("phonet.sender", "Sender Device ID", base.HEX, device)
fields.res  = ProtoField.uint8("phonet.resource", "Resource ID", base.HEX, resource)
fields.len  = ProtoField.uint16("phonet.length", "Data Length")
fields.robj = ProtoField.uint8("phonet.receiver_object", "Receiver Object ID", base.HEX)
fields.sobj = ProtoField.uint8("phonet.sender_object", "Sender Object ID", base.HEX)
fields.id   = ProtoField.uint8("phonet.id", "ID")

isifields.MTC      = ProtoField.uint8("isi.cmd", "Command", base.HEX, mtc_msg_ids)
isifields.INFO     = ProtoField.uint8("isi.cmd", "Command", base.HEX, phone_info_msg_ids)
isifields.SIM_AUTH = ProtoField.uint8("isi.cmd", "Command", base.HEX, sim_auth_msg_ids)

isifields.COMMON       = ProtoField.uint8("isi.subcmd", "Subinstruction", base.HEX, common_msg_sub_ids)
isifields.INFO_REQ     = ProtoField.uint8("isi.subcmd", "Subinstruction", base.HEX, phone_info_sub_ids)
isifields.SIM_AUTH_REQ = ProtoField.uint8("isi.subcmd", "Subinstruction", base.HEX, sim_auth_req)

isifields.PIN = ProtoField.string("isi.sim.pin", "PIN")
isifields.PUK = ProtoField.string("isi.sim.puk", "PUK")

isifields.data = ProtoField.bytes("isi.data", "Remaining Data")

local packet_counter

function name_device(id)
	return device[id] or "0x" .. string.format("%02X ", id)
end

function name_resource(id)
	return resource[id] or "0x" .. string.format("%02X ", id)
end

function skip()
	offset = offset + 1
	dataLength = dataLength - 1
end

function analyze_common(buffer, subtree)
	subtree:add(isifields.COMMON,buffer(offset, 1))
	skip()
end

function analyze_sim_auth(buffer, subtree)
	local msgid = buffer(offset, 1):uint()
	subtree:add(isifields.SIM_AUTH, buffer(offset, 1))
	info = info .. (sim_auth_msg_ids[msgid] or "")
	skip()

	if msgid == 0x07 then
		local subcmd = buffer(offset, 1):uint()
		subtree:add(isifields.SIM_AUTH_REQ, buffer(offset, 1))
		skip()

		if subcmd == 0x03 then
			subtree:add(isifields.PUK, buffer(offset, 8))
			offset = offset + 11
			dataLength = dataLength - 11
		end

		if subcmd == 0x02 or subcmd == 0x03 then
			subtree:add(isifields.PIN, buffer(offset, 8))
			offset = offset + 11
			dataLength = dataLength - 11

			if subcmd == 0x02 then
				offset = offset + 11
				dataLength = dataLength - 11
			end
		end
	end
end

function analyze_mtc(buffer, subtree)
	local msgid = buffer(offset, 1):uint()
	subtree:add(isifields.MTC,buffer(offset, 1))
	info = info .. (mtc_msg_ids[msgid] or "")
	skip()

	if msgid == 0xF0 then
		analyze_common(buffer, subtree)
	end
end

function analyze_phone_info(buffer, subtree)
	local msgid = buffer(offset, 1):uint()
	subtree:add(isifields.INFO,buffer(offset, 1))
	info = info .. (phone_info_msg_ids[msgid] or "")
	skip()

	if msgid == 0xF0 then
		analyze_common(buffer, subtree)
	else
		if msgid == 0x15 then
			local subid = buffer(offset, 1):uint()
			subtree:add(isifields.INFO_REQ,buffer(offset, 1))
			skip()
		end
	end
end

function phonet.dissector(buffer, pinfo, tree)
	info = ""
	offset = 0

	local subtree = tree:add(phonet, buffer())
	pinfo.cols.protocol = "Phonet"

	local destination = buffer(offset, 1):uint()
	subtree:add(fields.rdev,buffer(offset, 1))
	offset = offset + 1

	local source = buffer(offset, 1):uint()
	subtree:add(fields.sdev,buffer(offset, 1))
	offset = offset + 1

	local resource = buffer(offset, 1):uint()

	subtree:add(fields.res,buffer(offset, 1))
	offset = offset + 1

	dataLength = buffer(offset, 2):uint() - 3

	if buffer:len() ~= pinfo.len - 16 then
		info = "[BROKEN LENGTH]"
	end

	subtree:add(fields.len,buffer(offset, 2))
	offset = offset + 2

	subtree:add(fields.robj,buffer(offset, 1))
	offset = offset + 1

	subtree:add(fields.sobj,buffer(offset, 1))
	offset = offset + 1

	local subtree2 = subtree:add(isi, buffer(offset))
	pinfo.cols.protocol = "ISI"

	subtree2:add(fields.id,buffer(offset, 1))
	offset = offset + 1

	pinfo.cols.src = name_device(source)
	pinfo.cols.dst = name_device(destination)

	if resource == 0x08 then -- SIM AUTH
		analyze_sim_auth(buffer, subtree2)
	end

	if resource == 0x15 then -- MTC
		analyze_mtc(buffer, subtree2)
	end

	if resource == 0x1B then -- Phone Info
		analyze_phone_info(buffer, subtree2)
	end
	
	-- add remaining data, if any
	if dataLength > 0 then
		subtree2:add(isifields.data,buffer(offset))
	end

	pinfo.cols.info = info

end

function phonet.init()

	packet_counter = 0

end

sll_table = DissectorTable.get("sll.ltype")
sll_table:add(0x00f5,phonet)
