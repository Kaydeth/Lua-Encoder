--[[
	Name: m2pa_lib
	Description:
		Functions related to encoding/decoding M2PA message.
]]
	
local m2pa = {};

local utils = require "utils_lib";

--Default values for M2PA header fields
local M2PA_VERSION = 1;
local M2PA_SPARE = 0;
local M2PA_CLASS = 11;
local M2PA_USER_DATA_TYPE = 1;
local M2PA_LINK_STATUS_TYPE = 2;
local M2PA_DEFAULT_BSN = 0xFFFFFF
local M2PA_DEFAULT_FSN = 0

--[[
Create a client socket for an M2PA endpoint. Automatically aligns the
created endpoint at the M2PA level.
Args:
	local_host = string. ip address or host name for the local ethernet interface to use
	local_port = number. local port number to use
	remote_host = string. ip address or host name for remote end
	remote_port = number. remote port number to use
Return:
	table - endpoint structure
]]
function m2pa.m2pa_client(local_host, local_port, remote_host, remote_port)
	local recv_buf;
	local endpoint = {["fd"] = 0, ["fsn"] = M2PA_DEFAULT_FSN,
		["bsn"] = M2PA_DEFAULT_BSN, ["auto_ack"] = true};

	endpoint.fd = sctp_socket(local_host, local_port);

	print("sctp_connect to " .. remote_host .. ":" .. remote_port);
	sctp_connect(endpoint.fd, remote_host, remote_port);

	--Get OOS 
	print("Get OOS");
	recv_buf = sctp_recv(endpoint.fd);
	--Get Alignment
	print("Get Alignment");
	recv_buf = sctp_recv(endpoint.fd);

	--Send Alignment
	m2pa.send_m2pa_link_status(endpoint, 1);

	--Get Proving
	recv_buf = sctp_recv(endpoint.fd);
	print("Got Proving");

	--Send Proving
	m2pa.send_m2pa_link_status(endpoint, 2);

	while( recv_buf[20] == 2 or recv_buf[20] == 3 ) do
		recv_buf = sctp_recv(endpoint.fd);
	end
	print("Got Ready");

	--Link Status Ready should break us out
	--Send Link Status Ready
	m2pa.send_m2pa_link_status(endpoint, 4);

	return endpoint
end

--[[
Blocking recieve on the socket. Decodes the M2PA header of the received
message.
Args:
	endpoint - table. endpoint structure for M2PA socket to use
Return:
	table. returns a message structure with the decoded M2PA header in the
	m2pa subtable and any extra payload in the payload subtable
]]
function m2pa.m2pa_recv(endpoint)
	local data;
	local message = {};
	local payload;

	data = sctp_recv(endpoint.fd);

	message.m2pa, message.payload = m2pa.decode(data);
	endpoint.bsn = message.m2pa.fsn;

	if(endpoint.auto_ack == true) then
		local ack = {m2pa = {}}
	end


	return message;
end

--[[
Encodes and sends the given message to the peer. Will automatically
handle FSN and BSN if these values are nil in the message.
Args:
	endpoint - endpoint structure
	message - message structure
Return:
	No return value
]]
function m2pa.send(endpoint, message)
	local bytes;

	if(message.m2pa.type == M2PA_USER_DATA and message.payload) then
		endpoint.fsn = endpoint.fsn + 1;
	end

	bytes = m2pa.encode(message, endpoint);

	sctp_send(endpoint.fd, bytes);
end


--[[
Creates and sends a link status message with the given state value.
Args:
	endpoint - table. M2PA endpoint structure
	state - number. U32 value.
]]
function m2pa.send_m2pa_link_status(endpoint, state)
	local message = {};
	local bytes;

	message.m2pa = {};
	message.m2pa.type = M2PA_LINK_STATUS_TYPE;
	message.m2pa.state = state;

	m2pa.send(endpoint, message);
end

--[[
Encodes the given message structure into a byte array. Uses the endpoint
structure to auto set certain parameters such as FSN or BSN if those values
are not given in the message
Args:
	message - table. message structure
	endpoint - table. endpoint structure
	dest - table. Array of bytes to add the encoded data. If nil then new
		table is created
Return:
	table - if dest is not nil then dest is returned with the encoded data
		added to the end of the array. If dest is nil then a new array is
		created and returned.
]]
function m2pa.encode(message, endpoint, dest)
	local msg_type;
	local bytes;
	local length_index;

    if( dest ) then
        if( type(dest) == "table" ) then
            bytes = dest;
        else
            error("bad arg", 2);
        end
    else
        bytes = {};
    end

	if( tonumber(message.m2pa.version) ) then
		table.insert(bytes, message.m2pa.version);
	else
		table.insert(bytes, M2PA_VERSION);
	end

	if( tonumber(message.m2pa.spare) ) then
		table.insert(bytes, message.m2pa.spare);
	else
		table.insert(bytes, M2PA_SPARE);
	end

	if( tonumber(message.m2pa.class) ) then
		table.insert(bytes, message.m2pa.class);
	else
		table.insert(bytes, M2PA_CLASS);
	end

	if( tonumber(message.m2pa.type) ) then
		msg_type = message.m2pa.type;
	else
		msg_type = M2PA_USER_DATA_TYPE;
	end
	table.insert(bytes, msg_type);

	if( tonumber(message.m2pa.length) ) then
		utils.addU32(bytes, message.m2pa.length);
	else
		--Fill in the length at the end;
		length_index = #bytes + 1;
		utils.addU32(bytes, 0);
	end

	if( tonumber(message.m2pa.bsn) ) then
		utils.addU32(bytes, message.m2pa.bsn);
	elseif( endpoint and tonumber(endpoint.bsn)) then
		utils.addU32(bytes, endpoint.bsn);
	else
		utils.addU32(bytes, M2PA_DEFAULT_BSN);
	end

	if( tonumber(message.m2pa.fsn) ) then
		utils.addU32(bytes, message.m2pa.fsn);
	elseif( endpoint and tonumber(endpoint.fsn)) then
		utils.addU32(bytes, endpoint.fsn);
	else
		utils.addU32(bytes, M2PA_DEFAULT_FSN);
	end

	if(msg_type == M2PA_LINK_STATUS_TYPE and tonumber(message.m2pa.state) ) then
		utils.addU32(bytes, message.m2pa.state);
	end

	if(msg_type == M2PA_USER_DATA_TYPE) then
		local pri;
		local spare;
		if( tonumber(message.m2pa.priority)) then
			pri = message.m2pa.priority;
		else
			pri = 0;
		end

		if( tonumber(message.m2pa.spare2)) then
			spare = message.m2pa.spare2;
		else
			spare = 0;
		end

		table.insert(bytes, utils.combine_bits({{2,pri},{6,spare}}));
	end

	if( type(message.payload) == "table" ) then
		for i=1, #message.payload, 1 do
			if(tonumber(message.payload[i])) then
				table.insert(bytes, message.payload[i]);
			else
				error("Bad argument #2: found non-number value in table");
			end
		end
	end

	if( length_index ) then
		local b1,b2,b3,b4;
		b1,b2,b3,b4 = utils.break_into_bytes(#bytes,4);
		bytes[length_index] = b1;
		length_index = length_index + 1;
		bytes[length_index] = b2;
		length_index = length_index + 1;
		bytes[length_index] = b3;
		length_index = length_index + 1;
		bytes[length_index] = b4;
	end

	--print(utils.to_hex(bytes));
	return bytes;
end

--[[
Decodes the given array of bytes. The decoded information is return in a
table with the message structure.
Args:
	bytes - table. array of bytes received from sctp.
Return:
	table - message structure with decoded data.
	table - an array of bytes representing the M2PA data payload 
]]
function m2pa.decode(bytes)
	local i = 1;
	local m2pa_hdr = {};
	local payload = nil;

	m2pa_hdr.version = utils.getU8(bytes, i);
	i = i + 1;

	m2pa_hdr.spare = utils.getU8(bytes, i);
	i = i + 1;

	m2pa_hdr.class = utils.getU8(bytes, i);
	i = i + 1;

	m2pa_hdr.type = utils.getU8(bytes, i);
	i = i + 1;

	m2pa_hdr.length = utils.getU32(bytes, i);
	i = i + 4;

	m2pa_hdr.bsn = utils.getU32(bytes, i);
	i = i + 4;

	m2pa_hdr.fsn = utils.getU32(bytes, i);
	i = i + 4;

	if( m2pa_hdr.type == M2PA_LINK_STATUS_TYPE ) then
		m2pa_hdr.state = utils.getU32(bytes, i);
		i = i + 4;
	else
		m2pa_hdr.priority, m2pa_hdr.spare2 =
			utils.get_bits(utils.getU8(bytes,i),2,6);
		i = i + 1;
	end

	if( i <= #bytes ) then
		payload = {};
		for j = i, #bytes, 1 do
			table.insert(payload, bytes[i]);
			i = i + 1;
		end
	end

	return m2pa_hdr, payload;
end

return m2pa;
