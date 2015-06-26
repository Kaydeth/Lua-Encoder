--[[
	Name: diam_lib
	Description:
		Functions related to encoding/decoding Diameter messages.
]]

--[[ Message structure
	message (table)
]]

local diam = {};

local utils = require "utils_lib";

local DIAM_VERSION = 1;

diam.REQUEST_FLAG = 128;
diam.PROXY_FLAG = 64;
diam.ERROR_FLAG = 32;
diam.RETRANS_FLAG = 16
diam.VENDOR_FLAG = 128;
diam.MANDATORY_FLAG = 64;
--diam.VENDOR_AND_MAN_FLAG = bit32.bor(diam.VENDOR_FLAG, diam.MANDATORY_FLAG);
diam.VENDOR_AND_MAN_FLAG = 192;
diam.SECURITY_FLAG = 32;

diam.avp_codes = {};
diam.avp_codes.USER_NAME = 1;
diam.avp_codes.FRAMED_IP_ADDRESS = 8;
diam.avp_codes.CALLED_STATION_ID = 30;
diam.avp_codes.PROXY_STATE = 33;
diam.avp_codes.HOST_IP_ADDRESS = 257;
diam.avp_codes.AUTH_APPLICATION_ID = 258;
diam.avp_codes.VENDOR_SPECIFIC_APPLICATION_ID = 260;
diam.avp_codes.REDIRECT_HOST_USAGE = 261;
diam.avp_codes.REDIRECT_MAX_CACHE_TIME = 262;
diam.avp_codes.SESSION_ID = 263;
diam.avp_codes.ORIGIN_HOST = 264;
diam.avp_codes.VENDOR_ID = 266;
diam.avp_codes.RESULT_CODE = 268;
diam.avp_codes.PRODUCT_NAME = 269;
diam.avp_codes.ORIGIN_STATE_ID = 278;
diam.avp_codes.PROXY_HOST = 280;
diam.avp_codes.ROUTE_RECORD = 282;
diam.avp_codes.DESTINATION_REALM = 283;
diam.avp_codes.PROXY_INFO = 284;
diam.avp_codes.REDIRECT_HOST = 292;
diam.avp_codes.DESTINATION_HOST = 293;
diam.avp_codes.ERROR_REPORTING_HOST = 293;
diam.avp_codes.ORIGIN_REALM = 296;
diam.avp_codes.EXPERIMENTAL_RESULT = 297;
diam.avp_codes.EXPERIMENTAL_RESULT_CODE = 298;
diam.avp_codes.INBAND_SECURITY_ID = 299;
diam.avp_codes.CC_REQUEST_NUMBER = 415;
diam.avp_codes.CC_REQUEST_TYPE = 416;
diam.avp_codes.SUBSCRIPTION_ID = 443;
diam.avp_codes.SUBSCRIPTION_ID_DATA = 444;
diam.avp_codes.SUBSCRIPTION_ID_TYPE = 450;
diam.avp_codes.SUPPORTED_FEATURES = 628;
diam.avp_codes.EPS_LOCATION_INFORMATION = 1496;

diam.vendor_avp_dict = {};

diam.avp_dict = {};
diam.avp_dict[diam.avp_codes.USER_NAME] =
	{name = "User-Name", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.FRAMED_IP_ADDRESS] =
	{name = "Framed-IP-Address", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.CALLED_STATION_ID] =
	{name = "Called-Station-Id", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.PROXY_STATE] =
	{name = "Proxy-State", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.HOST_IP_ADDRESS] =
	{name = "Host-IP-Address", data_type = "address", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.AUTH_APPLICATION_ID] =
	{name = "Auth-Application-Id", data_type = "int32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.VENDOR_SPECIFIC_APPLICATION_ID] =
	{name = "Vendor-Specific-Application-Id", data_type = "grouped", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.REDIRECT_HOST_USAGE] =
	{name = "Redirect-Host-Usage", data_type = "int32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.REDIRECT_MAX_CACHE_TIME] =
	{name = "Redirect-Max-Cache-Time", data_type = "uint32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.SESSION_ID] =
	{name = "Session-Id", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.ORIGIN_HOST] =
	{name = "Origin-Host", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.VENDOR_ID] =
	{name = "Vendor-Id", data_type = "uint32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.RESULT_CODE] =
	{name = "Result-Code", data_type = "int32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.PRODUCT_NAME] =
	{name = "Product-Name", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.ORIGIN_STATE_ID] =
	{name = "Origin-State-Id", data_type = "uint32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.PROXY_HOST] =
	{name = "Proxy-Host", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.ROUTE_RECORD] =
	{name = "Route-Record", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.DESTINATION_REALM] =
	{name = "Destination-Realm", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.PROXY_INFO] =
	{name = "Proxy-Info", data_type = "grouped", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.REDIRECT_HOST] =
	{name = "Redirect-Host", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.DESTINATION_HOST] =
	{name = "Destination-Host", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.ERROR_REPORTING_HOST] =
	{name = "Error-Reporting-Host", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.ORIGIN_REALM] =
	{name = "Origin-Realm", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.EXPERIMENTAL_RESULT] =
	{name = "Experimental-Result", data_type = "grouped", flags = 0};
diam.avp_dict[diam.avp_codes.EXPERIMENTAL_RESULT_CODE] =
	{name = "Experimentail-Result-Code", data_type = "int32", flags = 0};
diam.avp_dict[diam.avp_codes.INBAND_SECURITY_ID] =
	{name = "Inband-Security-Id", data_type = "int32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.CC_REQUEST_NUMBER] =
	{name = "CC-Request-Number", data_type = "int32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.CC_REQUEST_TYPE] =
	{name = "CC-Request-Type", data_type = "int32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.SUBSCRIPTION_ID] =
	{name = "Subscription-Id", data_type = "grouped", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.SUBSCRIPTION_ID_DATA] =
	{name = "Subscription-Id-Data", data_type = "string", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.SUBSCRIPTION_ID_TYPE] =
	{name = "Subscription-Id-Type", data_type = "int32", flags = diam.MANDATORY_FLAG};
diam.avp_dict[diam.avp_codes.SUPPORTED_FEATURES] =
	{name = "Supported-Features", data_type = "grouped", flags = diam.VENDOR_AND_MAN_FLAG};
diam.avp_dict[diam.avp_codes.EPS_LOCATION_INFORMATION] =
	{name = "EPS-Location-Information", data_type = "grouped", flags = diam.VENDOR_FLAG};

function diam.recv_msg_decode(fd, type)
	local WATCH_DOG = 280;
	while (true) do
		local byte_buf = diam.recv_msg(fd, type);

		if(byte_buf == nil) then
			return nil;
		end

		local diam_msg = diam.decode(byte_buf);

		if( diam_msg.com_code == WATCH_DOG ) then
			diam_msg.flags.req = 0;
			local bytes = diam.encode(diam_msg);
			send(fd, bytes);
		else
			return diam_msg;
		end
	end
end

function diam.recv_msg(fd, type)
	local recv_buf = nil;
	local recv_flags = 0;
	local length_bytes = 4;

	if( type == nil or type == "HEX") then
		type = "HEX";
		recv_buf = {};
	end

	--Get the message length
	recv_buf, bytes_recv, errno = recv(fd, recv_buf, length_bytes, recv_flags,
		nil, type);

	if(bytes_recv ~= length_bytes) then
		if(bytes_recv == -1 and errno == EAGAIN) then
			return nil;
		elseif(bytes_recv == -1) then
			error("Error on recv for fd " .. fd .. ". Errno = " .. errno);
		elseif(bytes_recv == 0) then
			error("Connection closed by remote fd = " .. fd);
		else
			error("Could not get length bytes for diameter message, " .. bytes_recv);
		end
	end

	local msg_len = 0;
	if( type == "HEX") then
		msg_len = utils.getU24(recv_buf, 2); --first byte is version so ignore it
	else
		msg_len = utils.combine_bytes(0, string.byte(recv_buf, 2, 4));
	end

	local total_bytes = bytes_recv;
	local expected_total_bytes = msg_len;
	--print("expecting bytes " .. expected_total_bytes);
	while( total_bytes ~= expected_total_bytes) do
		expected_bytes = expected_total_bytes - total_bytes;
		local errrno, errno_str;
		local out_buf = nil;

		if( type == "HEX") then
			recv_buf, bytes_recv, errno, errno_str = recv(fd, recv_buf, expected_bytes,
				recv_flags, total_bytes + 1, type);
		else
			out_buf, bytes_recv, errno, errno_str = recv(fd, nil, expected_bytes,
				recv_flags, total_bytes + 1, type);

			recv_buf = recv_buf .. out_buf;
		end

		
		if( bytes_recv == -1 ) then
			error("Recv failed " .. errno ", " .. errno_str);
		elseif( bytes_recv ~= expected_bytes ) then
			--print("only got " .. bytes_recv .. ", expected " .. expected_bytes);
			--if( type == "HEX" ) then
				--print(utils.to_hex(recv_buf));
			--else
				--print(utils.str_to_hex(recv_buf));
			--end
		end
		total_bytes = total_bytes + bytes_recv;
	end


	return recv_buf;
end

--[[
diam.reverse_dict = {};
for k,v in pairs(diam.avp_dict) do 
	diam.reverse_dict[v.key] = {code = k, data_type = v.data_type, flags = v.flags};
end
]]

function diam.add_avp(message, avp_code, value, vendor_id)
	if(type(message) ~= "table") then
		print("add_avp called without valid message");
		return;
	end

	if(message.avps == nil) then
		message.avps = {};
	elseif(type(message.avps) ~= "table") then
		print("add_avp called with message.avps not a table");
		return;
	end

	if(avp_code == nil) then
		error("add_avp, avp code is nil");
	end

	local i = #message.avps + 1;
	message.avps[i] = {};
	message.avps[i].avp_code = avp_code;
	message.avps[i].value = value;

	if( vendor_id ~= nil ) then
		message.avps[i].vendor_id = vendor_id;
	end

	return i;
end

function diam.create_avp(avp_code, value, vendor_id)
	local avp = {};
	avp.avp_code = avp_code;
	avp.value = value;

	if(avp_code == nil) then
		error("create_avp, avp code is nil");
	end

	if( vendor_id ~= nil ) then
		avp.vendor_id = vendor_id;
	end

	return avp;
end

function diam.find_avp_value (message, avp_code)
	for k,v in ipairs(message.avps) do
		if( v.avp_code == avp_code) then
			return v.value;
		end
	end

	return nil;
end

function diam.find_avp(message, avp_code)
	for k,v in ipairs(message.avps) do
		--print("LSDEBUG: find avp at key/code " .. k .. "/" .. v.avp_code);
		if( v.avp_code == avp_code) then
			--print("LSDEBUG: return found avp");
			return v,k;
		end
	end

	return nil;
end

function diam.decode_header(bytes)
end

--[[
Decodes the given array of bytes. The decoded information is return in a
table with the message structure.
Args:
    bytes - table. array of bytes.
Return:
    table - message structure with decoded data.
    table - an array of bytes representing payload that could not be decoded
]]
function diam.decode(bytes)
--{
	local i = 1;
	local diam_msg = {};
	local payload = nil;

	if type(bytes) ~= "table" then
		error("bytes arg is wrong type, " .. type(bytes));
	end

	diam_msg.version = utils.getU8(bytes, i);
	i = i + 1;

	diam_msg.length = utils.getU24(bytes, i);
	i = i + 3;

	diam_msg.flags = {};
	diam_msg.flags.req, diam_msg.flags.proxy, diam_msg.flags.error,
		diam_msg.flags.retran, diam_msg.flags.spare1, diam_msg.flags.spare2,
		diam_msg.flags.spare3, diam_msg.flags.spare4 = 
		utils.get_bits(bytes[i], 1,1,1,1,1,1,1,1);
	i = i + 1;
		
	diam_msg.com_code = utils.getU24(bytes, i);
	i = i + 3;

	diam_msg.app_id = utils.getU32(bytes, i);
	i = i + 4;

	diam_msg.hop_id = utils.getU32(bytes, i);
	i = i + 4;

	diam_msg.end_id = utils.getU32(bytes, i);
	i = i + 4;

	diam_msg.avps = {};
	local avp;
	local avp_count = 1;
	local fmod;
	while( i <= #bytes) do
		avp = diam.decode_avp(bytes, i);
		i = i + avp.length;

		--skip padding bytes
		fmod = math.fmod(avp.length, 4);
		if( fmod ~= 0 ) then
			i = i + 4 - fmod;
		end

		diam_msg.avps[avp_count] = avp;
		avp_count = avp_count + 1;
	end

	return diam_msg
--}
end

function diam.decode_avp(bytes, i)
	local avp = {}
	local start = i;

	avp.avp_code = utils.getU32(bytes, i);
	i = i + 4;
	--print("LSDEBUG: decoding AVP " .. avp.avp_code);

	avp.flags = {};
	avp.flags.vendor,avp.flags.mandatory, avp.flags.protected,
		avp.flags.spare1, avp.flags.spare2, avp.flags.spare3,
		avp.flags.spare4, avp.flags.spare5 =
		utils.get_bits(bytes[i], 1,1,1,1,1,1,1,1);
	i = i + 1;

	avp.length = utils.getU24(bytes, i);
	i = i + 3;

	if( avp.flags.vendor == 1) then
		avp.vendor_id = utils.getU32(bytes, i);
		i = i + 4;
	end

--	print("LSDEBUG: found AVP ".. avp.avp_code);
	local data_type = diam.lookup_avp(avp.avp_code);

	if( data_type ~= nil) then
		if( data_type == "string" ) then
			avp.value = string.char(unpack(bytes,i, start + avp.length - 1));
		elseif( data_type == "address" ) then
			avp.value = {family = 1, addr = "1.1.1.1"};
		elseif( data_type == "int32" ) then
			--print("LSDEBUG: avp code " .. avp.avp_code .. " is an int32");
			avp.value = utils.getS32(bytes, i);
		elseif( data_type == "uint32" ) then
			avp.value = utils.getU32(bytes, i);
		elseif( data_type == "grouped" ) then
			--A grouped avp is decoded into a list of AVPs
--			avp.value = {}
--			
--			print("LSDEUBG: group start is ".. start);
--			print("LSDEBUG: group avp length is " .. avp.length);
--			local child_avp_count = 1;
--			while( i < start + avp.length) do
--				print("Data start is " .. i);
--				local child_avp = diam.decode_avp(bytes, i);
--				i = i + avp.length;
--		
--				--skip padding bytes
--				local fmod = math.fmod(avp.length, 4);
--				if( fmod ~= 0 ) then
--					i = i + 4 - fmod;
--				end
--		
--				avp.value[child_avp_count] = avp;
--				child_avp_count = child_avp_count + 1;
--			end
			
			avp.data = {};
			
			local data_start = i - 1;
			for i=i, start + avp.length - 1, 1 do
				avp.data[i - data_start] = bytes[i];
			end
		else
			error("invalid avp data type, data_type = " .. data_type);
		end
	else
		--print("LSDEBUG: avp code " .. avp.avp_code .. " data type is not found");
		avp.data = {};
		
		local data_start = i - 1;
		for i=i, start + avp.length - 1, 1 do
			avp.data[i - data_start] = bytes[i];
		end
	end

	return avp;
end

function diam.encode_str(message)
--{
	local bytes = diam.encode(message);

	return string.char(table.unpack(bytes));
--}
end

function diam.encode(message)
--{
	local bytes = {};
	local flags = {};
	local length_index;
	local app_id = 0;
	local com_code = 0;
	
	if(type(message) ~= "table") then
		encode_error("Invalid message. Must be table.");
	end

	if( tonumber(message.version) ) then
		table.insert(bytes, message.version);
	else
		table.insert(bytes, DIAM_VERSION);
	end

	if( tonumber(message.length) ) then
		--print("LSDEBUG: setting message length to " .. message.length);
		utils.addU24(bytes, message.length);
	else
		--Fill in length at the end;
		--print("LSDEBUG: setting length_index to " .. #bytes + 1);
		length_index = #bytes + 1;
		utils.addU24(bytes, 0);
	end

	-- assume no flags in message means 0 byte
	local flags = 0;
	if(message.flags ~= nil) then
		if(message.flags.req == nil) then
			message.flags.req = 0;
		end
		if(message.flags.proxy == nil) then
			message.flags.proxy = 0;
		end
		if(message.flags.error == nil) then
			message.flags.error = 0;
		end
		if(message.flags.retran == nil) then
			message.flags.retran = 0;
		end
		if(message.flags.spare1 == nil) then
			message.flags.spare1 = 0;
		end
		if(message.flags.spare2 == nil) then
			message.flags.spare2 = 0;
		end
		if(message.flags.spare3 == nil) then
			message.flags.spare3 = 0;
		end
		if(message.flags.spare4 == nil) then
			message.flags.spare4 = 0;
		end
		
		local lsh = bit32.lshift;
		flags = bit32.bor(lsh(message.flags.req, 7),
			lsh(message.flags.proxy, 6),
			lsh(message.flags.error, 5),
			lsh(message.flags.retran, 4),
			lsh(message.flags.spare1, 3),
			lsh(message.flags.spare2, 2),
			lsh(message.flags.spare3, 1),
			lsh(message.flags.spare4, 0) );
	end

	table.insert(bytes, flags);

	if( tonumber(message.com_code) ) then
		com_code = message.com_code
	end
	utils.addU24(bytes, com_code);

	if( tonumber(message.app_id) ) then
		app_id = message.app_id;
	end
	utils.addU32(bytes, app_id);
		

	if( tonumber(message.hop_id) ) then
		utils.addU32(bytes, message.hop_id);
	else
		utils.addU32(bytes, 0);
	end

	if( tonumber(message.end_id) ) then
		--print("LSDEBUG: encode end to end " .. message.end_id);
		utils.addU32(bytes, message.end_id);
	else
		utils.addU32(bytes, 0);
	end

	if(type(message.avps) == "table") then
		for i,avp in ipairs(message.avps) do 
				encode_avp(bytes, avp, app_id, com_code)
		end
	end

	--print("LSDEBUG: encode final size is " .. #bytes);
	--If the user specified the message length then don't
	--calculate a new one
	if ( length_index ) then
		local b1,b2,b3,b4 = utils.break_into_bytes(#bytes, 4);
		bytes[length_index] = b2;
		bytes[length_index + 1] = b3;
		bytes[length_index + 2] = b4;
	end

	return bytes;
--}
end

function encode_avp(bytes, avp, app_id, com_code)
--{
		local avp_length = 0;
		local padding_length = 0;

		if(type(avp) ~= "table") then
			if(type(avp) == "string" and avp == "deleted") then
				--do nothing. This avp was deleted so skip it
				return
			else
				encode_error("Invalid avp format, type = " .. type(avp)
					.. ", " .. tostring(avp), app_id, com_code);
			end
		end
		
		--encode avp code first
		if( avp.avp_code == nil) then
			encode_error("Avp code is nil", app_id, com_code);
			return
		end
		utils.addU32(bytes, avp.avp_code);

		if( avp.data ~= nil ) then
			--print("LSDEBUG: avp " .. avp.avp_code .. " has binary data");

			if (avp.flags ~= nil) then
				local lsh = bit32.lshift;
				local flags = bit32.bor(lsh(avp.flags.vendor, 7),
					lsh(avp.flags.mandatory, 6),
					lsh(avp.flags.protected, 5),
					lsh(avp.flags.spare1, 4),
					lsh(avp.flags.spare2, 3),
					lsh(avp.flags.spare3, 2),
					lsh(avp.flags.spare4, 1),
					lsh(avp.flags.spare5, 0) );
				table.insert(bytes, flags);
			else
				table.insert(bytes, 0);
				return
			end

			if(avp.vendor_id == nil) then
				avp_length = #avp.data + 8;
				utils.addU24(bytes, avp_length); --avp length byte
			else
				avp_length = #avp.data + 12;
				utils.addU24(bytes, avp_length); --avp length byte
				utils.addU32(bytes, avp.vendor_id);
			end

			--We should copy the data from a byte table here
			local offset = #bytes;
			for i = 1, #avp.data, 1 do
				bytes[i + offset] = avp.data[i];
			end

			--add padding
			padding_length = diam.add_avp_padding(bytes, #avp.data);
		else
			--print("LSDEBUG: avp " .. avp.avp_code .. " has readable data ");
			local dict_entry = nil
			if( avp.vendor_id ~= nil ) then
				dict_entry = diam.vendor_avp_dict[avp.vendor_id][avp.avp_code];
			else
				dict_entry = diam.avp_dict[avp.avp_code];
			end

			if(dict_entry == nil) then
				error("AVP code is not in dictionary, code =  " ..
					avp.avp_code .. ", vendor id = " .. avp.vendor_id);
			end

			--Get the AVP flags from the user if provided. Otherwise use the
			--dictionary definition
			if avp.flags ~= nil then
				local lsh = bit32.lshift;
				local flags = bit32.bor(lsh(avp.flags.vendor, 7),
					lsh(avp.flags.mandatory, 6),
					lsh(avp.flags.protected, 5),
					lsh(avp.flags.spare1, 4),
					lsh(avp.flags.spare2, 3),
					lsh(avp.flags.spare3, 2),
					lsh(avp.flags.spare4, 1),
					lsh(avp.flags.spare5, 0) );
				table.insert(bytes, flags);
			else
				table.insert(bytes, dict_entry.flags);
				--print("LSDEBUG: added flags " .. dict_entry.flags .. " at " .. #bytes);
			end

			local length_index = #bytes + 1;
			utils.addU24(bytes, 0); --place holder for length
			if(avp.vendor_id == nil) then
				avp_length = 8;
			else
				avp_length = 12;
				utils.addU32(bytes, avp.vendor_id);
			end

			if( dict_entry.data_type == "string" ) then
				if(type(avp.value) ~= "string") then
					encode_error("Expected string value, avp value is " ..
						type(avp.value), app_id, com_code, avp.avp_code);
				end

				local data_len = string.len(avp.value);
				avp_length = avp_length + data_len;
				utils.add_bytes(bytes, string.byte(avp.value,1,data_len));
				--add padding
				padding_length = diam.add_avp_padding(bytes, data_len)
				
			elseif( dict_entry.data_type == "int32" ) then
				--print("LSDEBUG: avp " .. avp.avp_code .. " value is " .. avp.value);
				avp_length = avp_length + 4;
				utils.addU32(bytes, avp.value);

			elseif( dict_entry.data_type == "uint32" ) then
				--print("LSDEBUG: avp " .. avp.avp_code .. " value is " .. avp.value);
				avp_length = avp_length + 4;
				utils.addU32(bytes, avp.value);
				
			elseif( dict_entry.data_type == "address" ) then
				avp_length = avp_length + 6;

				utils.addU16(bytes, avp.value.family);

				for match in string.gmatch(avp.value.addr, "%d+") do
					--print("LSDEBUG: encoding address");
					--print(tonumber(match));
					table.insert(bytes, tonumber(match));
				end
				
				--add padding
				table.insert(bytes, 0);
				table.insert(bytes, 0);
				padding_length = 2;
				
			elseif( dict_entry.data_type == "grouped") then

				for i, child_avp in ipairs(avp.value) do
					avp_length = avp_length + 
						encode_avp(bytes, child_avp, app_id, com_code);
				end
				
				--padding is done in the child avps so no need for
				--calculating padding on the parent
			end

			local b1,b2,b3,b4 = utils.break_into_bytes(avp_length, 4);
			bytes[length_index] = b2;
			bytes[length_index + 1] = b3;
			bytes[length_index + 2] = b4;
		end
		
		return avp_length + padding_length;
--}
end

function encode_error(error_text, app_id, com_code, avp_code)
	error(string.format("Encode error. App ID %s : Com %s : AVP %s : %s",
		app_id, com_code, avp_code, error_text));
end

function diam.add_avp_padding(bytes, avp_length)
	local bytes_padded = 0;
	local fmod = math.fmod(avp_length, 4);
	if( fmod ~= 0 ) then
		bytes_padded = 4 - fmod;
		for i=1, bytes_padded, 1 do
			table.insert(bytes, 0);
		end
	end

	return bytes_padded;
end

function diam.lookup_avp(avp_code)
	if( diam.avp_dict[avp_code] ) then
		return diam.avp_dict[avp_code].data_type;
	end

	return nil
end

function diam.print_message(message)
	local print_buf = {};

	diam.set_print_value(print_buf, "version", message);
	diam.set_print_value(print_buf, "length", message);

	print_buf.flags = {};
	diam.set_print_value(print_buf.flags, "req", message.flags);
	diam.set_print_value(print_buf.flags, "proxy", message.flags);
	diam.set_print_value(print_buf.flags, "error", message.flags);
	diam.set_print_value(print_buf.flags, "retrans", message.flags);
	diam.set_print_value(print_buf.flags, "spare1", message.flags);
	diam.set_print_value(print_buf.flags, "spare2", message.flags);
	diam.set_print_value(print_buf.flags, "spare3", message.flags);
	diam.set_print_value(print_buf.flags, "spare4", message.flags);

	diam.set_print_value(print_buf, "app_id", message);
	diam.set_print_value(print_buf, "com_code", message);
	diam.set_print_value(print_buf, "hop_id", message);
	diam.set_print_value(print_buf, "end_id", message);
	diam.set_print_value(print_buf, "origin_host", message);
	diam.set_print_value(print_buf, "origin_realm", message);

	print(string.format("Ver = %s; Len = %s; Ccode = %s; AppId = %s; " ..
		"HopId = %s; EndId = %s; Origin Host = %s; Origin Realm = %s",
		print_buf.version, print_buf.length,
		print_buf.com_code, print_buf.app_id, print_buf.hop_id,
		print_buf.end_id, print_buf.origin_host, print_buf.origin_realm) );

	print_buf.avps = {};
	for k,v in ipairs(message.avps) do
		print_buf.avps[k] = {};
		diam.set_print_value(print_buf.avps[k], "avp_code", message.avps[k]);
		diam.set_print_value(print_buf.avps[k], "length", message.avps[k]);
		diam.set_print_value(print_buf.avps[k], "vendor_id", message.avps[k]);

		if(message.avps[k].data) then
			print_buf.avps[k].data = utils.to_hex(message.avps[k].data);
		elseif(message.avps[k].value) then
			print_buf.avps[k].data = message.avps[k].value;
		else
			print_buf.avps[k].data = "N/A";
		end

		print(string.format("Avp Code = %s; Avp Len = %s; Vendor Id = %s;\n" ..
			"data:\n%s", print_buf.avps[k].avp_code, print_buf.avps[k].length,
			print_buf.avps[k].vendor_id, print_buf.avps[k].data));
	end
end

function diam.set_print_value(print_buf, key, source_buf)
	if( source_buf[key] ) then
		print_buf[key] = source_buf[key];
	else
		print_buf[key] = "N/A";
	end
end

--[[
Attempts to complete a CER/CEA exchange over the given socket. The socket should
already be established. 
Args:
	sock - a socket with an established connection
	type - if type is "client" then this function will intiate the CER. If type
		is "server" then this function will wait for the remote side to the send
		the CER.

Return:
	none
]]
function diam.cer_cea_exchange(sock, origin_host, origin_realm, type)
	if( type == "server" ) then
		--Get CER
		local recv_bytes = diam.recv_msg(sock);

		local decoded_msg = diam.decode(recv_bytes);

		--Convert to CEA
		decoded_msg.flags.req = 0;
		decoded_msg.length = nil; --let the length get recalculated since I'm adding an avp
		diam.add_avp(decoded_msg, diam.avp_codes.RESULT_CODE, 2001);
		local origin_host_avp = diam.find_avp(decoded_msg, diam.avp_codes.ORIGIN_HOST);
		origin_host_avp.value = origin_host;

		local origin_realm_avp = diam.find_avp(decoded_msg, diam.avp_codes.ORIGIN_REALM);
		origin_realm_avp.value = origin_realm;

		--Send CEA back
		local bytes = diam.encode(decoded_msg);

		local send_rc = send(sock, bytes);

	elseif( type == "client") then
		--Create CER
		local cer_msg = {};

		cer_msg.com_code = 257
		cer_msg.flags = {req=1};
		diam.add_avp(cer_msg, diam.avp_codes.ORIGIN_HOST, origin_host);
		diam.add_avp(cer_msg, diam.avp_codes.ORIGIN_REALM, origin_realm);
		diam.add_avp(cer_msg, diam.avp_codes.HOST_IP_ADDRESS, {family=1, addr="10.2.89.8"});
		diam.add_avp(cer_msg, diam.avp_codes.VENDOR_ID, 6629);
		diam.add_avp(cer_msg, diam.avp_codes.PRODUCT_NAME, "Ulticom Diameter Lite");
		diam.add_avp(cer_msg, diam.avp_codes.AUTH_APPLICATION_ID, -1);
		diam.add_avp(cer_msg, diam.avp_codes.INBAND_SECURITY_ID, 0);

		--Send CER
		local bytes = diam.encode(cer_msg);
		local send_rc = send(sock, bytes);

		--Get CEA
		local recv_msg, recv_bytes = recv(sock, nil, 10000, 0, nil, "HEX");

		if( recv_bytes == 0 ) then
			error("CER/CEA exchange failed. Connection closed by remote.");
		end
	else
		error("Invalid type given for cer/cea exchange, " .. type ..
			". Expected server or client");
	end
end

function diam.parse_realm(fqdn)
	local i = string.find(fqdn, "%.");
	local len = #string;

	
	if( i ~= nil and i ~= len) then
		return string.sub(fqdn, i + 1, -1);
	else
		return fqdn
	end

end

return diam;
