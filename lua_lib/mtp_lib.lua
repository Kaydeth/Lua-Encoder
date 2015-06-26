local mtp = {}

local m2pa = require "m2pa_lib";

function mtp.link(local_host, local_port, remote_host, remote_port)
	local fd = m2pa.client(local_host, local_port, remote_host, remote_port);

	m2pa.m2pa_recv(fd);
end

function mtp.encode(message, dest)
	local ni;
	local pri;
	local si;
	local pc = {};
	local bytes;

	if( dest ) then
		if( type(dest) == "table" ) then
			bytes = dest;
		else
			error("bad arg", 2);
		end
	else
		bytes = {}
	end
	
    if( tonumber(message.mtp.network) ) then
		ni = message.mtp.network;
	else
		ni = 2;
	end

	if( tonumber(message.mtp.priority) ) then
		pri = message.mtp.priority;
	else
		pri = 0;
	end

	if( tonumber(message.mtp.service_ind) ) then
		si = message.mtp.service_ind;
	else
		si = 0;
	end

	table.insert(bytes, utils.combine_bits({{2,ni},{2,pri},{4,si}}) );

	if( tonumber(message.mtp.dpc.member) ) then
		pc.member = message.mtp.dpc.member;
	else
		pc.member = 0;
	end

	if( tonumber(message.mtp.dpc.cluster) ) then
		pc.cluster = message.mtp.dpc.cluster;
	else
		pc.cluster = 0;
	end

	if( tonumber(message.mtp.dpc.network) ) then
		pc.network = message.mtp.dpc.network;
	else
		pc.network = 0;
	end

	table.insert(bytes, pc.member);
	table.insert(bytes, pc.cluster);
	table.insert(bytes, pc.network);

	if( tonumber(message.mtp.opc.member) ) then
		pc.member = message.mtp.opc.member;
	else
		pc.member = 0;
	end

	if( tonumber(message.mtp.opc.cluster) ) then
		pc.cluster = message.mtp.opc.cluster;
	else
		pc.cluster = 0;
	end

	if( tonumber(message.mtp.opc.network) ) then
		pc.network = message.mtp.opc.network;
	else
		pc.network = 0;
	end

	table.insert(bytes, pc.member);
	table.insert(bytes, pc.cluster);
	table.insert(bytes, pc.network);

	if( tonumber(message.mtp.sls) ) then
		table.insert(bytes, message.mtp.sls);
	else
		table.insert(bytes, 0);
	end

	local h1, h0;
	if( tonumber(message.mtp.h1) ) then
		h1 = message.mtp.h1;
	else
		h1 = 0;
	end

	if( tonumber(message.mtp.h0) ) then
		h0 = message.mtp.h0;
	else
		h0 = 0;
	end

	table.insert(bytes, utils.combine_bits({{4,h1},{4,h0}}) );

	local test_len, slc;
	if( tonumber(message.mtp.test_length) ) then
		test_len = message.mtp.test_length;
	else
		test_len = 2;
	end

	if( tonumber(message.mtp.slc) ) then
		slc = message.mtp.slc;
	else
		slc = 0;
	end

	table.insert(bytes, utils.combine_bits({{4,test_len},{4,slc}}) );

	if(type(message.mtp.test_pattern) == "table" ) then
		for i=1, #message.mtp.test_pattern, 1 do
			if(tonumber(message.mtp.test_pattern[i])) then
				table.insert(bytes, message.mtp.test_pattern[i]);
			else
				error("Bad argument #2: found non-number value in table");
			end
		end
	end

	return bytes;
end

function mtp.decode(bytes)
	local mtp_hdr = {};
	local payload = nil;
	local i = 1;

	mtp_hdr.network, mtp_hdr.priority, mtp_hdr.service_ind =
		utils.get_bits(utils.getU8(bytes, i), 2, 2, 4);
	i = i + 1;

	mtp_hdr.dpc = {};
	mtp_hdr.dpc.member = utils.getU8(bytes, i);
	i = i + 1;
	mtp_hdr.dpc.cluster = utils.getU8(bytes, i);
	i = i + 1;
	mtp_hdr.dpc.network = utils.getU8(bytes, i);
	i = i + 1;
	mtp_hdr.dpc.str = string.format("%d-%d-%d", mtp_hdr.dpc.network,
		mtp_hdr.dpc.cluster, mtp_hdr.dpc.member);

	mtp_hdr.opc = {};
	mtp_hdr.opc.member = utils.getU8(bytes, i);
	i = i + 1;
	mtp_hdr.opc.cluster = utils.getU8(bytes, i);
	i = i + 1;
	mtp_hdr.opc.network = utils.getU8(bytes, i);
	i = i + 1;
	mtp_hdr.opc.str = string.format("%d-%d-%d", mtp_hdr.opc.network,
		mtp_hdr.opc.cluster, mtp_hdr.opc.member);

	mtp_hdr.sls = utils.getU8(bytes, i);
	i = i + 1;

	mtp_hdr.h1, mtp_hdr.h0 = utils.get_bits(
		utils.getU8(bytes, i), 4, 4);
	i = i + 1;

	mtp_hdr.test_length, mtp_hdr.slc = utils.get_bits(
		utils.getU8(bytes, i), 4, 4);
	i = i + 1;

	mtp_hdr.test_pattern = {};
	for j = i, i + mtp_hdr.test_length - 1, 1 do
		table.insert(mtp_hdr.test_pattern, bytes[i]);
		i = i + 1;
	end

	if( i <= #bytes ) then
		payload = {};
		for j = i, #bytes, 1 do
			table.insert(payload, bytes[i]);
			i = i + 1;
		end
	end

	return mtp_hdr, payload;
end

return mtp
