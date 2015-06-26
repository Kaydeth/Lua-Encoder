--[[
	This script will connect to an inter-instance link and
send the proper protocol messages that will fully establish the
link such that the instance thinks it is connected to another instance.
The cluster_name and my_instance_name must be updated with names
that match the DSC provisioning.
]]--

local cluster_name = "ClusterName";
local my_instance_name = "MyName";
local fd = socket(AF_INET);
local pb_type = pb.CLUSTER_ROUTING_MESSAGE;

print("fd is " .. fd);

connect(fd,"vw03-3", 6363, AF_INET);

local init_hello = {};
init_hello.header = {};
init_hello.hello_payload = {};
init_hello.header.version = 1; 
init_hello.header.message_type = pb.crm.MSG_HELLO; 
init_hello.hello_payload.origin_instance_name = my_instance_name; 
init_hello.hello_payload.cluster_name = cluster_name; 

local send_rc = pb.send_msg(fd, init_hello, pb_type);

local loop = true;
local expected_msg = 1;

while ( loop == true ) do
	local msg, other = pb.recv_msg(fd, pb_type);
	--print("message_type = " .. msg.header.message_type);

	if( msg.header.message_type == expected_msg) then
		if( msg.header.message_type == pb.crm.MSG_HELLO_ACK ) then --HELLO_ACK
			--Got hello ack expect DD init next
			print("Got hello ack");
			expected_msg = pb.crm.MSG_DB_DESCRIPTION;
			local dd_init = {};
			dd_init.header = {};
			dd_init.database_description = {};
			dd_init.header.version = 1;
			dd_init.header.message_type = pb.crm.MSG_DB_DESCRIPTION;
			dd_init.database_description.init_flag = true;
			dd_init.database_description.more_flag = true;
			dd_init.database_description.master_flag = true;
			dd_init.database_description.dd_sequence_num = 0;
			pb.send_msg(fd, dd_init, pb_type);
		elseif( msg.header.message_type == pb.crm.MSG_DB_DESCRIPTION ) then
			if(msg.database_description.init_flag == true) then
				print("Got DD init");
			else
				print("Got DD " .. msg.database_description.dd_sequence_num);
			end
	
			if(msg.database_description.more_flag == false) then
				--expect one link state request message
				expected_msg = pb.crm.MSG_LINK_STATE_REQUEST;
			end

			--reply to DD with empty database
			msg.database_description.init_flag = false;
			msg.database_description.master_flag = false;
			msg.database_description.more_flag = false;
			msg.database_description.lsa_header_list = nil;

			pb.send_msg(fd, msg, pb_type);

			--Don't have to send any link state requests
		elseif( msg.header.message_type == pb.crm.MSG_LINK_STATE_REQUEST ) then
			msg.request_payload = nil;

			local lsa_header = {};
			lsa_header.lsa_age = 1;
			lsa_header.advertising_instance = my_instance_name
			lsa_header.lsa_sequence_number = 1;

			msg.update_payload = {};
			msg.update_payload.lsa_list = {};
			msg.update_payload.lsa_list[1] = {};
			msg.update_payload.lsa_list[1].lsa_header = lsa_header;
			--no link_state_list

			msg.header.message_type = pb.crm.MSG_LINK_STATE_UPDATE;

			pb.send_msg(fd, msg, pb_type);

			expected_msg = pb.crm.MSG_LINK_STATE_ACK; --expect ack next
		elseif( msg.header.message_type == pb.crm.MSG_LINK_STATE_UPDATE) then
			print("Got Link State Update");
			--must ack the update
			msg.ack_payload = {};
			msg.ack_payload.lsa_header_list = {}
			msg.ack_payload.lsa_header_list[1] = msg.update_payload.lsa_list[1].lsa_header;
			msg.update_payload = nil;
			msg.header.message_type = pb.crm.MSG_LINK_STATE_ACK;

			pb.send_msg(fd, msg, pb_type);
		elseif( msg.header.message_type == pb.crm.MSG_LINK_STATE_ACK) then
			--Nothing to do for ack
			print("Got Link State Ack");
			expected_msg = pb.crm.MSG_LINK_STATE_UPDATE;
		end
	elseif( msg.header.message_type == pb.crm.MSG_HELLO ) then --hello message
		--Got a hello message. Just ack it
		print("Got Hello");
		msg.hello_payload.origin_instance_name = my_instance_name;
		msg.header.message_type = pb.crm.MSG_HELLO_ACK;

		local rc = pb.send_msg(fd, msg, pb_type);
		--print("send hello ack rc = " .. rc);
	else
		print("Unexpected message, type = " .. msg.header.message_type);
	end
end



