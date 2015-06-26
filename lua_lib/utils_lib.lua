local utils = {}

--[[
	Helper function to print an array of bytes
	as hex data
]]
function utils.to_hex(data)
	local format = "";
	local i = 1;

	while (i < #data) do
		format = format .. "%02x";
		if( math.fmod(i, 16) == 0 ) then
			format = format .. "\n";
		else
			format = format .. " ";
		end
	i = i +1;
	end

	format = format .. "%02x";

	return string.format(format, unpack(data ));
end

function utils.str_to_hex(str)
	print("str_to_hex");
	print(string.byte(str, 1, #str));
end

function utils.add_bytes(array, ...)
	for i,v in ipairs({...}) do
		table.insert(array, v);
	end
end

function utils.addU16(array, value)
	local b1,b2;

	b1, b2 = utils.break_into_bytes(value, 2);

	table.insert(array, b1);
	table.insert(array, b2);
end

function utils.addU24(array, value)
	local b1,b2,b3,b4;

	b1, b2, b3, b4 = utils.break_into_bytes(value, 4);

	table.insert(array, b2);
	table.insert(array, b3);
	table.insert(array, b4);
end

function utils.addU32(array, value)
	local b1,b2,b3,b4;

	b1, b2, b3, b4 = utils.break_into_bytes(value, 4);

	table.insert(array, b1);
	table.insert(array, b2);
	table.insert(array, b3);
	table.insert(array, b4);
end

function utils.break_into_bytes(value, num_bytes)
	local byte1;
	local byte2;
	local byte3;
	local byte4;
	local value_temp;

	--If the value is greater than the max value that
	--a num_bytes data type can hold then just return
	--max value for every bytes
	if( value > math.pow(2,num_bytes*8) ) then
		if( num_bytes == 2) then
			return 255, 255;
		else
			return 255, 255, 255, 255;
		end
	end

	value_temp = value

	if( num_bytes == 4) then
		byte1 = bit32.extract (value, 24, 8);
		byte2 = bit32.extract (value, 16, 8);
		
		--[[
		byte1 = math.floor(value_temp/math.pow(2,24));
		value_temp = math.fmod(value_temp, math.pow(2,24));

		byte2 = math.floor(value_temp/math.pow(2,16));
		value_temp = math.fmod(value_temp, math.pow(2,16));
		]]
	end

		byte3 = bit32.extract (value, 8, 8);
		byte4 = bit32.extract (value, 0, 8);

--[[
	byte3 = math.floor(value_temp/math.pow(2,8));
	value_temp = math.fmod(value_temp, math.pow(2,8));

	byte4 = value_temp
	]]

	if( num_bytes == 2) then
		return byte3, byte4
	else
		return byte1, byte2, byte3, byte4
	end
end

function utils.combine_bytes(byte1, byte2, byte3, byte4)
	local value;
	if(byte4) then
		value = byte1 * math.pow(2,24);
		value = value + byte2 * math.pow(2,16);
		value = value + byte3 * math.pow(2,8);
		value = value + byte4
	else
		value = byte1 * math.pow(2,8);
		value = value + byte2
	end

	return value
end

function utils.print_message(message)
	local m2pa_version = "N/A";
	local m2pa_spare = "N/A";
	local m2pa_class = "N/A";
	local m2pa_type = "N/A";
	local m2pa_length = "N/A";
	local m2pa_bsn = "N/A";
	local m2pa_fsn = "N/A";
	local m2pa_state = "N/A";
	local m2pa_pri = "N/A";
	local m2pa_spare2 = "N/A";
	local mtp_network = "N/A";
	local mtp_priority = "N/A";
	local mtp_svc_ind = "N/A";
	local mtp_dpc = "N/A";
	local mtp_opc = "N/A";
	local mtp_sls = "N/A";
	local mtp_h0 = "N/A";
	local mtp_h1 = "N/A";
	local mtp_slc = "N/A";
	local mtp_test_len = "N/A";
	local mtp_test_pattern = "N/A";

	if(message.m2pa.version) then
		m2pa_version = message.m2pa.version;
	end

	if(message.m2pa.spare) then
		m2pa_spare = message.m2pa.spare;
	end

	if(message.m2pa.class) then
		m2pa_class = message.m2pa.class;
	end

	if(message.m2pa.type) then
		m2pa_type = message.m2pa.type;
	end

	if(message.m2pa.length) then
		m2pa_length = message.m2pa.length
	end

	if(message.m2pa.bsn) then
		m2pa_bsn = message.m2pa.bsn
	end

	if(message.m2pa.fsn) then
		m2pa_fsn = message.m2pa.fsn
	end

	if(message.m2pa.priority) then
		m2pa_pri = message.m2pa.priority
	end

	if(message.m2pa.spare2) then
		m2pa_spare2 = message.m2pa.spare2
	end

	print(string.format("Ver = %s; Spare = %s; Class = %s; Type = %s;" ..
		"Len = %s; BSN = %s; FSN = %s, Pri = %s, Spare2 = %s", m2pa_version,
		m2pa_spare, m2pa_class, m2pa_type, m2pa_length, m2pa_bsn, m2pa_fsn,
		m2pa_pri, m2pa_spare2));
	--print(string.format("Len = %s; BSN = %s; FSN = %s; Pri = %s, Spare2 = %s",
		--length, bsn, fsn));

	if(message.m2pa.state) then
		print("State = " .. mesage.state);
	end

	if(message.mtp.network) then
		mtp_network = message.mtp.network;
	end

	if(message.mtp.priority) then
		mtp_priority = message.mtp.priority;
	end

	if(message.mtp.service_ind) then
		mtp_svc_ind = message.mtp.service_ind;
	end

	if(message.mtp.dpc and message.mtp.dpc.str) then
		mtp_dpc = message.mtp.dpc.str;
	end

	if(message.mtp.opc.str) then
		mtp_opc = message.mtp.opc.str;
	end

	if(message.mtp.sls) then
		mtp_sls = message.mtp.sls;
	end

	if(message.mtp.h0) then
		mtp_h0 = message.mtp.h0;
	end

	if(message.mtp.h1) then
		mtp_h1 = message.mtp.h1;
	end

	if(message.mtp.slc) then
		mtp_slc = message.mtp.slc;
	end

	if(message.mtp.test_length) then
		mtp_test_len = message.mtp.test_length;
	end

	if(message.mtp.test_pattern) then
		mtp_test_pattern = utils.to_hex(message.mtp.test_pattern);
	end

	print(string.format("NI = %s, Pri = %s, Svc Ind = %s, DPC = %s," ..
		"OPC = %s, SLS  %s, h0 = %s, h1 = %s, SLC = %s, Test Length = %s," ..
		"Test Pattern = %s", mtp_network, mtp_priority, mtp_svc_ind,
		mtp_dpc, mtp_opc, mtp_sls, mtp_h0, mtp_h1, mtp_slc, mtp_test_len,
		mtp_test_pattern));

	if( message.payload and #message.payload > 0) then
		print("Data:" .. utils.to_hex(message.payload));
	end
end

function utils.getU8(data, index)
	return data[index];
end

function utils.getU16(data, index)
	local value;

	value = data[index] * math.pow(2,8);
	value = value + data[index + 1]

	return value;
end

function utils.getU24(data, index)
	local value;

	value = data[index] * math.pow(2,16);
	value = value + data[index + 1] * math.pow(2,8);
	value = value + data[index + 2]

	return value;
end

function utils.getU32(data, index)

	local value = bit32.bor(
		bit32.lshift(data[index], 24),
		bit32.lshift(data[index + 1], 16),
		bit32.lshift(data[index + 2], 8),
		data[index + 3]);

	return value;
end

function utils.getS32(data, index)

	--Get 32 bit value by combining the bits in the array
	local value = bit32.bor(
		bit32.lshift(data[index], 24),
		bit32.lshift(data[index + 1], 16),
		bit32.lshift(data[index + 2], 8),
		data[index + 3]);

	--If the sign bit is set convert the value to negative number
	if( bit32.btest(value, 0x80000000) )then
		--Convert to a negative number by subtracting 1
		--and calculating the 2's complement
		value = value - 1;
		value = bit32.bnot(value);
		value = -value;
	end
	
	return value;
end

function utils.get_bits(byte, bits1, bits2, bits3, bits4, bits5, bits6, bits7, bits8)
	local input_bits = {bits1, bits2, bits3, bits4, bits5, bits6, bits7, bits8}
	local bits = {}
	local stop = {0,0,0,0,0,0,0,0};
	local stop_index;
	local value;
	local ret_vals = {};
	local last_stop;
	local bit_sum;
	local exp;
	local curr;

	stop_index = 1;
	stop[stop_index] = 1;
	
	--no need for stop bit for group 8
	for i=1, 7, 1 do
		if( input_bits[i] ) then
			stop_index = stop_index + input_bits[i];
			if( stop_index <= 8 ) then
				stop[stop_index] = 1;
			else
				break;
			end
		end
	end

	curr = byte;
	local bit_value = 128;
	for i=7, 0, -1 do
		if( curr >= bit_value) then
			table.insert(bits, 1);
			curr = curr - bit_value;
		else
			table.insert(bits, 0);
		end

		bit_value = bit_value / 2;
	end

	exp = 0;
	value = 0;
	for i=8, 1, -1 do
		if(bits[i] == 1) then
			value = value + math.pow(2,exp);
		end

		exp = exp + 1

		if(stop[i] == 1) then
			table.insert(ret_vals,1, value);
			value = 0;
			exp = 0;
		end
	end

	return unpack(ret_vals);
end

function utils.combine_bits(bits_array)
	local value = 0;
	local shift = 8;

	for i=1, 8, 1 do
		if(bits_array[i] and bits_array[i][1] and bits_array[i][2]) then
			shift = shift - bits_array[i][1];
			value = value + bits_array[i][2] * math.pow(2,shift);
		end
	end

	return value;
end

function utils.sleep(ms)

	local pad = nil;
	if( ms < 100 ) then
		pad = "0";
		if( ms < 10 ) then
			pad = "00";
		end
	elseif( ms > 999) then
		ms = 999;
	end

	local cmd = nil;
	if(pad ~= nil) then
		cmd = "sleep ." .. pad .. ms;
	else
		cmd = "sleep ." ..ms;
	end

	local ok, err_type, err_no =
		os.execute(cmd);

	if(ok == nil) then
		if(err_type == "signal" and err_no == 2) then
			--interrupt signal from user. Just quit
			error("Interrupt signal, quiting script");
		end

		error("sleep returned error, " .. err_type .. ", " .. err_no);
	end
end

return utils
