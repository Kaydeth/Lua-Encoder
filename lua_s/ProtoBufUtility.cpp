/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	ProtoBufUtility.h
 * \version	$Revision:/main/4 $
 * \date	$Date:6-Dec-2013 16:14:39 $
 * \product	DSC test tools
 * \brief	helper functions encode/decode a protocol message to and from
 * 		a Lua table.
 */

#include "ProtoBufUtility.h"
#include "ClusterRoutingMessage.pb.h"
#include "DraMessagePB.pb.h"
#include "LuaDefines.h"
#include <lauxlib.h>
#include <lualib.h>
#include <arpa/inet.h>

/* Since Lua is single threaded we can get away with using static objects here
 * instead of using a real factory class */
UlticomADC::ClusterRoutingMessage ProtoBufUtility::clu_route_msg;
UlticomADC::DraMessagePB ProtoBufUtility::dra_msg;

const string ProtoBufUtility::HEADER = "header";
const string ProtoBufUtility::VERSION = "version";
const string ProtoBufUtility::MESSAGE_TYPE = "message_type";
const string ProtoBufUtility::HELLO_PAYLOAD = "hello_payload";
const string ProtoBufUtility::ORIGIN_INSTANCE_NAME = "origin_instance_name";
const string ProtoBufUtility::CLUSTER_NAME = "cluster_name";
const string ProtoBufUtility::DATABASE_DESCRIPTION = "database_description";
const string ProtoBufUtility::INIT_FLAG = "init_flag";
const string ProtoBufUtility::MORE_FLAG = "more_flag";
const string ProtoBufUtility::MASTER_FLAG = "master_flag";
const string ProtoBufUtility::DD_SEQUENCE_NUM = "dd_sequence_num";
const string ProtoBufUtility::LSA_HEADER_LIST = "lsa_header_list";
const string ProtoBufUtility::REQUEST_PAYLOAD = "request_payload";
const string ProtoBufUtility::UPDATE_PAYLOAD = "update_payload";
const string ProtoBufUtility::LSA_LIST = "lsa_list";
const string ProtoBufUtility::LSA_HEADER = "lsa_header";
const string ProtoBufUtility::LSA_AGE = "lsa_age";
const string ProtoBufUtility::ADVERTISING_INSTANCE = "advertising_instance";
const string ProtoBufUtility::LSA_SEQUENCE_NUMBER = "lsa_sequence_number";
const string ProtoBufUtility::LINK_STATE_LIST = "link_state_list";
const string ProtoBufUtility::LINK_TYPE = "link_type";
const string ProtoBufUtility::LINK_ID = "link_id";
const string ProtoBufUtility::LINK_COST = "link_cost";
const string ProtoBufUtility::CONGESTION_STATUS = "congestion_status";
const string ProtoBufUtility::ACK_PAYLOAD = "ack_payload";
const string ProtoBufUtility::USER_PAYLOAD = "user_payload";
const string ProtoBufUtility::SSID = "ssid";
const string ProtoBufUtility::PRI_KEY = "pri_key";
const string ProtoBufUtility::SEC_KEY = "sec_key";
const string ProtoBufUtility::USER_META = "user_meta";
const string ProtoBufUtility::LOAD_SHARE_HASH_KEY = "load_share_hash_key";
const string ProtoBufUtility::DATA = "data";


/*
 * Pushes the values of all Procotol constants into the Lua environment. Expects
 * the "pb" table to at the top of the stack.
 */
void ProtoBufUtility::pushConstants(lua_State* lua_state)
{
	//assumes that pb table is on top

	lua_newtable(lua_state);

	lua_pushinteger(lua_state, UlticomADC::ClusterRoutingHeader::CURRENT_VERSION);
	lua_setfield(lua_state, LUA_TOP - 1, "CURRENT_VERSION");

	lua_pushinteger(lua_state, UlticomADC::ClusterRoutingHeader::MSG_HELLO);
	lua_setfield(lua_state, LUA_TOP - 1, "MSG_HELLO");

	lua_pushinteger(lua_state, UlticomADC::ClusterRoutingHeader::MSG_HELLO_ACK);
	lua_setfield(lua_state, LUA_TOP - 1, "MSG_HELLO_ACK");

	lua_pushinteger(lua_state, UlticomADC::ClusterRoutingHeader::MSG_SHUTDOWN);
	lua_setfield(lua_state, LUA_TOP - 1, "MSG_SHUTDOWN");

	lua_pushinteger(lua_state, UlticomADC::ClusterRoutingHeader::MSG_DB_DESCRIPTION);
	lua_setfield(lua_state, LUA_TOP - 1, "MSG_DB_DESCRIPTION");

	lua_pushinteger(lua_state, UlticomADC::ClusterRoutingHeader::MSG_LINK_STATE_REQUEST);
	lua_setfield(lua_state, LUA_TOP - 1, "MSG_LINK_STATE_REQUEST");

	lua_pushinteger(lua_state, UlticomADC::ClusterRoutingHeader::MSG_LINK_STATE_UPDATE);
	lua_setfield(lua_state, LUA_TOP - 1, "MSG_LINK_STATE_UPDATE");

	lua_pushinteger(lua_state, UlticomADC::ClusterRoutingHeader::MSG_LINK_STATE_ACK);
	lua_setfield(lua_state, LUA_TOP - 1, "MSG_LINK_STATE_ACK");

	lua_pushinteger(lua_state, UlticomADC::ClusterRoutingHeader::MSG_USER_MESSAGE);
	lua_setfield(lua_state, LUA_TOP - 1, "MSG_USER_MESSAGE");

	lua_setfield(lua_state, LUA_TOP -1, "crm");
}


/*
 * Attempts to decode the message in the given buffer and push the decoded
 * 	values into a table in the Lua environment
 * Arg 1 - lua_state - Lua enviroment
 * Arg 2 - type - the protobuf type of the encoded message
 * Arg 3 - buf - the buffer that contains the encoded message
 * Arg 4 - msg_len - the length of the message, should not exceed length of buffer
 * Returns pushes a table onto the Lua stack with the decoded contents of the
 * 	message on success and returns true. Returns false on failure.
 */
bool ProtoBufUtility::parseFromArrayToLua(lua_State* lua_state,
	ProtoBufType type, char* buf, int msg_len)
{
	switch(type)
	{
	case CLUSTER_ROUTING_MESSAGE:
	{
		bool rc = clu_route_msg.ParseFromArray(buf, msg_len);
		if(rc == false)
		{
			//We'll fall here if we end up connecting to somebody wierd who sends
			//us garbage
			return false;
		}

		pushClusterRoutingMessage(lua_state);
		break;
	}
	case DRA_MESSAGE_PB:
	{
		bool rc = dra_msg.ParseFromArray(buf, msg_len);
		if(rc == false)
		{
			//We'll fall here if we end up connecting to somebody wierd who sends
			//us garbage
			return false;
		}
		break;
	}
	default:
		return false;
	}

	return true;
}

/*
 * Attempts to encode a message to the given buffer. The input for the message
 * 	is assumed to be a Lua table at the top of the Lua stack
 * Arg 1 - lua_state - Lua enviroment
 * Arg 2 - type - the protobuf type of the encoded message
 * Arg 3 - out_buf - buffer to write the encoded bytes
 * 	Returns true of success. Interrupts script exection or returns false on error.
 */
bool ProtoBufUtility::serializeFromLuaToString(lua_State* lua_state, ProtoBufType type, string& out_buf)
{
	::google::protobuf::Message* msg = NULL;

	 switch(type)
	{
	case CLUSTER_ROUTING_MESSAGE:
	{
		msg = &clu_route_msg;
		msg->Clear();
		pullClusterRoutingMessage(lua_state);
		break;
	}
	case DRA_MESSAGE_PB:
	{
		msg = &dra_msg;
		msg->Clear();
		break;
	}
	default:
		return luaL_error(lua_state, "Invalid protocol buffer type given, %d", type);
	}

	if(!msg->IsInitialized())
	{
		return false;
	}

	int len = msg->ByteSize();
	int size_check = sizeof(len) + len;

	if(size_check <= 0 || size_check > out_buf.capacity())
	{
		return luaL_error(lua_state, "Message too big to send, msg_size = %d, max_size = %d",
			size_check, out_buf.capacity());
	}

	len = htonl(len);
	out_buf.assign((const char *)&len, sizeof(len));

	if( msg->AppendToString(&out_buf) == false)
	{
		return false;
	}

	return true;
}


/*
 * Takes the contents of the global clu_route_msg and pushes the values
 * into a Lua table that represents the message contents.
 * Arg 1 - lua_state - Lua enviroment
 * 	Returns a new table is pushed on the Lua stack with the message contents
 */
void ProtoBufUtility::pushClusterRoutingMessage(lua_State* lua_state)
{
	//push table to hold message
	lua_newtable(lua_state);

	//push table to hold header
	lua_newtable(lua_state);

	lua_pushinteger(lua_state, clu_route_msg.header().version());
	lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::VERSION.c_str());

	lua_pushinteger(lua_state, clu_route_msg.header().message_type());
	lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::MESSAGE_TYPE.c_str());

	//header is done. set it to the message
	lua_setfield(lua_state, LUA_TOP -1, ProtoBufUtility::HEADER.c_str());

	if(clu_route_msg.has_hello_payload())
	{
		//push table for hello_payload;
		lua_newtable(lua_state);

		lua_pushstring(lua_state, clu_route_msg.hello_payload().origin_instance_name().c_str());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::ORIGIN_INSTANCE_NAME.c_str());

		lua_pushstring(lua_state, clu_route_msg.hello_payload().cluster_name().c_str());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::CLUSTER_NAME.c_str());

		//hello_payload is done. set it to the message
		lua_setfield(lua_state, LUA_TOP -1, ProtoBufUtility::HELLO_PAYLOAD.c_str());
	}

	if(clu_route_msg.has_database_description())
	{
		const UlticomADC::DatabaseDescription& dd = clu_route_msg.database_description();

		//push table for database_description;
		lua_newtable(lua_state);

		lua_pushboolean(lua_state, dd.init_flag());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::INIT_FLAG.c_str());

		lua_pushboolean(lua_state, dd.more_flag());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::MORE_FLAG.c_str());

		lua_pushboolean(lua_state, dd.master_flag());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::MASTER_FLAG.c_str());

		lua_pushinteger(lua_state, dd.dd_sequence_num());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::DD_SEQUENCE_NUM.c_str());

		int lsa_count = dd.lsa_header_list_size();
		if(lsa_count != 0)
		{
			//push table for lsa_header_list
			lua_newtable(lua_state);
			for(int i = 0; i < lsa_count; i++)
			{
				//set each LSA header at a numerical index starting at 1
				lua_pushinteger(lua_state, i +1);

				//push table for LSA header
				lua_newtable(lua_state);

				lua_pushinteger(lua_state, dd.lsa_header_list(i).lsa_age());
				lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LSA_AGE.c_str());

				lua_pushstring(lua_state, dd.lsa_header_list(i).advertising_instance().c_str());
				lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::ADVERTISING_INSTANCE.c_str());

				lua_pushinteger(lua_state, dd.lsa_header_list(i).lsa_sequence_number());
				lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LSA_SEQUENCE_NUMBER.c_str());

				lua_settable(lua_state, LUA_TOP - 2);
			}

			lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LSA_HEADER_LIST.c_str());
		}

		//database description is done. set it to the message
		lua_setfield(lua_state, LUA_TOP -1, ProtoBufUtility::DATABASE_DESCRIPTION.c_str());
	}

	if(clu_route_msg.has_request_payload())
	{
		//push table for request_payload
		lua_newtable(lua_state);

		const UlticomADC::LinkStateRequest& lsr = clu_route_msg.request_payload();

		int count = lsr.advertising_instance_size();
		if(count != 0)
		{
			//push table for advertising_instance
			lua_newtable(lua_state);
			for(int i = 0; i < count; i++)
			{
				//set values at numerical indices starting at 1
				lua_pushinteger(lua_state, i + 1);
				lua_pushstring(lua_state, lsr.advertising_instance(i).c_str());
				lua_settable(lua_state, LUA_TOP - 2);
			}
			lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::ADVERTISING_INSTANCE.c_str());
		}

		//database description is done. set it to the message
		lua_setfield(lua_state, LUA_TOP -1, ProtoBufUtility::REQUEST_PAYLOAD.c_str());
	}

	if(clu_route_msg.has_update_payload())
	{
		const UlticomADC::LinkStateUpdate& lsu = clu_route_msg.update_payload();

		//push table for update_payload;
		lua_newtable(lua_state);

		int lsa_count = lsu.lsa_list_size();
		if(lsa_count != 0)
		{
			//push table for lsa_list
			lua_newtable(lua_state);
			for(int i = 0; i < lsa_count; i++)
			{
				//set each LSA at a numerical index starting at 1
				lua_pushinteger(lua_state, i + 1);
				//push table for LSA
				lua_newtable(lua_state);
				//push table for lsa_header
				lua_newtable(lua_state);

				const UlticomADC::LSAHeader& header = lsu.lsa_list(i).lsa_header();

				lua_pushinteger(lua_state, (int)header.lsa_age());
				lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LSA_AGE.c_str());

				lua_pushstring(lua_state, header.advertising_instance().c_str());
				lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::ADVERTISING_INSTANCE.c_str());

				lua_pushinteger(lua_state, (int)header.lsa_sequence_number());
				lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LSA_SEQUENCE_NUMBER.c_str());

				lua_setfield(lua_state, LUA_TOP -1, ProtoBufUtility::LSA_HEADER.c_str());

				int link_state_count = lsu.lsa_list(i).link_state_list_size();
				if(link_state_count != 0)
				{
					//push table for array of LinkState values
					lua_newtable(lua_state);
					for(int j = 0; j < link_state_count; j++)
					{
						//push index for this LinkState data
						lua_pushinteger(lua_state, j+1);
						//push table for LinkState data
						lua_newtable(lua_state);
						const UlticomADC::LinkState& link_state = lsu.lsa_list(i).link_state_list(j);

						lua_pushinteger(lua_state, link_state.link_type());
						lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LINK_TYPE.c_str());

						lua_pushstring(lua_state, link_state.link_id().c_str());
						lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LINK_ID.c_str());

						lua_pushinteger(lua_state, (int)link_state.link_cost());
						lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LINK_COST.c_str());

						lua_pushinteger(lua_state, (int)link_state.congestion_status());
						lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::CONGESTION_STATUS.c_str());
						
						//Set the the LinkState data at the this index
						lua_settable(lua_state, LUA_TOP - 2);
					}

					lua_setfield(lua_state, LUA_TOP -1, ProtoBufUtility::LINK_STATE_LIST.c_str());
				}

				//Set the full LSA at the appropriate index
				lua_settable(lua_state, LUA_TOP - 2);
			}

			lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LSA_LIST.c_str());
		}

		//database description is done. set it to the message
		lua_setfield(lua_state, LUA_TOP -1, ProtoBufUtility::UPDATE_PAYLOAD.c_str());
	}

	if(clu_route_msg.has_ack_payload())
	{
		const UlticomADC::DatabaseDescription& dd = clu_route_msg.database_description();

		//push table for ack_payload;
		lua_newtable(lua_state);

		int lsa_count = dd.lsa_header_list_size();
		if(lsa_count != 0)
		{
			//push table for lsa_header_list
			lua_newtable(lua_state);
			for(int i = 0; i < lsa_count; i++)
			{
				//set each LSA header at a numerical index starting at 1
				lua_pushinteger(lua_state, i +1);

				//push table for LSA header
				lua_newtable(lua_state);

				lua_pushinteger(lua_state, dd.lsa_header_list(i).lsa_age());
				lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LSA_AGE.c_str());

				lua_pushstring(lua_state, dd.lsa_header_list(i).advertising_instance().c_str());
				lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::ADVERTISING_INSTANCE.c_str());

				lua_pushinteger(lua_state, dd.lsa_header_list(i).lsa_sequence_number());
				lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LSA_SEQUENCE_NUMBER.c_str());

				lua_settable(lua_state, LUA_TOP - 2);
			}

			lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LSA_HEADER_LIST.c_str());
		}

		//database description is done. set it to the message
		lua_setfield(lua_state, LUA_TOP -1, ProtoBufUtility::ACK_PAYLOAD.c_str());
	}

	if(clu_route_msg.has_user_payload())
	{
		//push table for user_payload
		lua_newtable(lua_state);

		const UlticomADC::UserMessage& user_msg = clu_route_msg.user_payload();

		lua_pushinteger(lua_state, user_msg.ssid());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::SSID.c_str());

		lua_pushstring(lua_state, user_msg.pri_key().c_str());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::PRI_KEY.c_str());

		lua_pushstring(lua_state, user_msg.sec_key().c_str());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::SEC_KEY.c_str());

		lua_pushstring(lua_state, user_msg.user_meta().c_str());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::USER_META.c_str());

		lua_pushinteger(lua_state, (int)user_msg.load_share_hash_key());
		lua_setfield(lua_state, LUA_TOP - 1, ProtoBufUtility::LOAD_SHARE_HASH_KEY.c_str());

		//TODO: get data bytes

		//hello_payload is done. set it to the message
		lua_setfield(lua_state, LUA_TOP -1, ProtoBufUtility::USER_PAYLOAD.c_str());
	}
}

/*
 * Takes the contents of the Lua table at the top of the Lua stack and copies
 * then to the clu_route_msg buffer.
 * Arg 1 - lua_state - Lua enviroment
 * 	Returns a new table is pushed on the Lua stack with the message contents
 */
void ProtoBufUtility::pullClusterRoutingMessage(lua_State* lua_state)
{
	UlticomADC::ClusterRoutingHeader* header = clu_route_msg.mutable_header();
	lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::HEADER.c_str());
	lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::VERSION.c_str());
	header->set_version((UlticomADC::ClusterRoutingHeader_Version)lua_tointeger(lua_state, LUA_TOP));
	lua_pop(lua_state, 1); //pop version

	lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::MESSAGE_TYPE.c_str());
	header->set_message_type((UlticomADC::ClusterRoutingHeader_MessageType)lua_tointeger(lua_state, LUA_TOP));
	lua_pop(lua_state, 1); //pop message_type

	lua_pop(lua_state, 1); //pop header

	lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::HELLO_PAYLOAD.c_str());
	if(lua_istable(lua_state, LUA_TOP))
	{
		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::ORIGIN_INSTANCE_NAME.c_str());
		if(lua_isstring(lua_state, LUA_TOP))
		{
			UlticomADC::HelloPayload* hello = clu_route_msg.mutable_hello_payload();

			hello->set_origin_instance_name(lua_tostring(lua_state, LUA_TOP));
		}
		lua_pop(lua_state, 1); //pop origin_instance_name

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::CLUSTER_NAME.c_str());
		if(lua_isstring(lua_state, LUA_TOP))
		{
			UlticomADC::HelloPayload* hello = clu_route_msg.mutable_hello_payload();

			hello->set_cluster_name(lua_tostring(lua_state, LUA_TOP));
		}
		lua_pop(lua_state, 1); //pop cluster_name
	}
	lua_pop(lua_state, 1); //pop hello_payload

	lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::DATABASE_DESCRIPTION.c_str());
	if(lua_istable(lua_state, LUA_TOP))
	{
		UlticomADC::DatabaseDescription* dd = clu_route_msg.mutable_database_description();

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::INIT_FLAG.c_str());
		if(lua_isboolean(lua_state, LUA_TOP))
		{
			dd->set_init_flag(lua_toboolean(lua_state, LUA_TOP));
		}
		lua_pop(lua_state, 1);

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::MORE_FLAG.c_str());
		if(lua_isboolean(lua_state, LUA_TOP))
		{
			dd->set_more_flag(lua_toboolean(lua_state, LUA_TOP));
		}
		lua_pop(lua_state, 1);

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::MASTER_FLAG.c_str());
		if(lua_isboolean(lua_state, LUA_TOP))
		{
			dd->set_master_flag(lua_toboolean(lua_state, LUA_TOP));
		}
		lua_pop(lua_state, 1);

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::DD_SEQUENCE_NUM.c_str());
		if(lua_isnumber(lua_state, LUA_TOP))
		{
			dd->set_dd_sequence_num(lua_tointeger(lua_state, LUA_TOP));
		}
		lua_pop(lua_state, 1);

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_HEADER_LIST.c_str());
		if(lua_istable(lua_state, LUA_TOP))
		{
			int i = 1; //Lua indices start at 1
			lua_pushinteger(lua_state, i); //push index
			lua_gettable(lua_state, LUA_TOP -1 );

			while(lua_istable(lua_state, LUA_TOP))
			{
				UlticomADC::LSAHeader *header = dd->add_lsa_header_list();

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_AGE.c_str());
				header->set_lsa_age(lua_tointeger(lua_state, LUA_TOP));
				lua_pop(lua_state, 1);

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::ADVERTISING_INSTANCE.c_str());
				header->set_advertising_instance(lua_tostring(lua_state, LUA_TOP));
				lua_pop(lua_state, 1);

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_SEQUENCE_NUMBER.c_str());
				header->set_lsa_sequence_number(lua_tointeger(lua_state, LUA_TOP));
				lua_pop(lua_state, 1);

				lua_pop(lua_state, 1); //pop off table
				lua_pushinteger(lua_state, ++i);
				lua_gettable(lua_state, LUA_TOP - 1);
			}
		}
		lua_pop(lua_state, 1); //pop lsa_header_list
	}
	lua_pop(lua_state, 1); //pop database_description

	lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::REQUEST_PAYLOAD.c_str());
	if(lua_istable(lua_state, LUA_TOP))
	{
		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::ADVERTISING_INSTANCE.c_str());
		if(lua_istable(lua_state, LUA_TOP))
		{
			UlticomADC::LinkStateRequest* lsr = clu_route_msg.mutable_request_payload();
			int i = 1; //Lua indices start at 1
			lua_pushinteger(lua_state, i); //push index
			lua_gettable(lua_state, LUA_TOP -1 );

			while(lua_isstring(lua_state, LUA_TOP))
			{
				lsr->add_advertising_instance(lua_tostring(lua_state, LUA_TOP));

				lua_pop(lua_state, 1); //pop off string value
				lua_pushinteger(lua_state, ++i);
				lua_gettable(lua_state, LUA_TOP -1 );
			}
		}
		lua_pop(lua_state, 1); //pop advertising_instance
	}
	lua_pop(lua_state, 1); // pop request_payload


	lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::UPDATE_PAYLOAD.c_str());
	if(lua_istable(lua_state, LUA_TOP))
	{
		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_LIST.c_str());
		if(lua_istable(lua_state, LUA_TOP))
		{
			UlticomADC::LinkStateUpdate* lsu = clu_route_msg.mutable_update_payload();
			int i = 1; //Lua indicies start at 1
			lua_pushinteger(lua_state, i);//push index
			lua_gettable(lua_state, LUA_TOP -1 );

			while(lua_istable(lua_state, LUA_TOP))
			{
				UlticomADC::LSA* lsa = lsu->add_lsa_list();
				UlticomADC::LSAHeader* header = lsa->mutable_lsa_header();
				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_HEADER.c_str());

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_AGE.c_str());
				header->set_lsa_age(lua_tointeger(lua_state, LUA_TOP));
				lua_pop(lua_state, 1);

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::ADVERTISING_INSTANCE.c_str());
				header->set_advertising_instance(lua_tostring(lua_state, LUA_TOP));
				lua_pop(lua_state, 1);

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_SEQUENCE_NUMBER.c_str());
				header->set_lsa_sequence_number(lua_tointeger(lua_state, LUA_TOP));
				lua_pop(lua_state, 1);

				lua_pop(lua_state, 1); //pop lsa_header

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LINK_STATE_LIST.c_str());
				if(lua_istable(lua_state, LUA_TOP))
				{
					int i = 1; //Lua indicies start at 1
					lua_pushinteger(lua_state, i);//push index
					lua_gettable(lua_state, LUA_TOP -1 );

					while(lua_istable(lua_state, LUA_TOP))
					{
						UlticomADC::LinkState* link_state = lsa->add_link_state_list();

						lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LINK_TYPE.c_str());
						link_state->set_link_type((UlticomADC::LinkState_LinkType)
							lua_tointeger(lua_state, LUA_TOP));
						lua_pop(lua_state, 1);

						lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LINK_ID.c_str());
						link_state->set_link_id(lua_tostring(lua_state, LUA_TOP));
						lua_pop(lua_state, 1);

						lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LINK_COST.c_str());
						link_state->set_link_cost(lua_tointeger(lua_state, LUA_TOP));
						lua_pop(lua_state, 1);

						lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::CONGESTION_STATUS.c_str());
						link_state->set_congestion_status(
							(UlticomADC::LinkState_CongestionLevel)lua_tointeger(lua_state, LUA_TOP));
						lua_pop(lua_state, 1);
						
						lua_pop(lua_state, 1); //pop off table
						lua_pushinteger(lua_state, ++i);//push index
						lua_gettable(lua_state, LUA_TOP -1 );
					}
				}

				lua_pop(lua_state, 1); //pop off table
				lua_pushinteger(lua_state, ++i);
				lua_gettable(lua_state, LUA_TOP -1 );
			}
		}
	}
	lua_pop(lua_state, 1); //pop update_payload

	lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::ACK_PAYLOAD.c_str());
	if(lua_istable(lua_state, LUA_TOP))
	{
		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_HEADER_LIST.c_str());

		if(lua_istable(lua_state, LUA_TOP))
		{
			UlticomADC::LinkStateAcknowledgment* lsack = clu_route_msg.mutable_ack_payload();
			int i = 1; //Lua indices start at 1
			lua_pushinteger(lua_state, i); //push index
			lua_gettable(lua_state, LUA_TOP -1 );

			while(lua_istable(lua_state, LUA_TOP))
			{
				UlticomADC::LSAHeader *header = lsack->add_lsa_header_list();

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_AGE.c_str());
				header->set_lsa_age(lua_tointeger(lua_state, LUA_TOP));
				lua_pop(lua_state, 1);

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::ADVERTISING_INSTANCE.c_str());
				header->set_advertising_instance(lua_tostring(lua_state, LUA_TOP));
				lua_pop(lua_state, 1);

				lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LSA_SEQUENCE_NUMBER.c_str());
				header->set_lsa_sequence_number(lua_tointeger(lua_state, LUA_TOP));
				lua_pop(lua_state, 1);

				lua_pop(lua_state, 1); //pop off table
				lua_pushinteger(lua_state, ++i);
				lua_gettable(lua_state, LUA_TOP -1 );
			}
		}

		lua_pop(lua_state, 1); //pop lsa_header_list
	}
	lua_pop(lua_state, 1);

	lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::USER_PAYLOAD.c_str());
	if(lua_istable(lua_state, LUA_TOP))
	{
		UlticomADC::UserMessage* msg = clu_route_msg.mutable_user_payload();

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::SSID.c_str());
		msg->set_ssid((UlticomADC::UserMessage_SubsystemId)lua_tointeger(lua_state, LUA_TOP));
		lua_pop(lua_state, 1);

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::PRI_KEY.c_str());
		msg->set_pri_key(lua_tostring(lua_state, LUA_TOP));
		lua_pop(lua_state, 1);

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::SEC_KEY.c_str());
		msg->set_sec_key(lua_tostring(lua_state, LUA_TOP));
		lua_pop(lua_state, 1);

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::USER_META.c_str());
		msg->set_user_meta(lua_tostring(lua_state, LUA_TOP));
		lua_pop(lua_state, 1);

		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::LOAD_SHARE_HASH_KEY.c_str());
		msg->set_load_share_hash_key(lua_tointeger(lua_state, LUA_TOP));
		lua_pop(lua_state, 1);

//		lua_getfield(lua_state, LUA_TOP, ProtoBufUtility::DATA.c_str());
		//TODO: add data to message
	}

	lua_pop(lua_state, 1);
}
