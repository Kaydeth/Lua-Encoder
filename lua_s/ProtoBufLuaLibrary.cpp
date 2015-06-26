/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	ProtoBufLuaLibrary.cpp
 * \version	$Revision:/main/2 $
 * \date	$Date:19-Jun-2015 10:22:25 $
 * \product	DSC test tools
 * \brief	exposes send and received functions that will accept and return
 * 		a Lua table that defines a message based on a generated Protocol Buffer
 * 		class.
 */

#include "ProtoBufLuaLibrary.h"
#include "ProtoBufUtility.h"
#include "LuaDefines.h"
#include <lauxlib.h>
#include <lualib.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sstream>
#include <iomanip>
#include <cstring>

using namespace std;

int ProtoBufLuaLibrary::MAX_MESSAGE_SIZE = 50000;

/*
 * Loads all the functions defined in this library into the Lua environment
 * \param lua_state - the state to load all the functions
 */
void ProtoBufLuaLibrary::loadLibrary(lua_State* lua_state)
{
	lua_newtable(lua_state);

	lua_pushcfunction(lua_state, ProtoBufLuaLibrary::recvMsg);
	lua_setfield(lua_state, LUA_TOP - 1, "recv_msg");

	lua_pushcfunction(lua_state, ProtoBufLuaLibrary::sendMsg);
	lua_setfield(lua_state, LUA_TOP - 1, "send_msg");

	lua_pushinteger(lua_state, ProtoBufUtility::CLUSTER_ROUTING_MESSAGE);
	lua_setfield(lua_state, LUA_TOP - 1, "CLUSTER_ROUTING_MESSAGE");

	lua_pushinteger(lua_state, ProtoBufUtility::DRA_MESSAGE_PB);
	lua_setfield(lua_state, LUA_TOP - 1, "DRA_MESSAGE");

	ProtoBufUtility::pushConstants(lua_state);

	lua_setglobal(lua_state, "pb");
}

/*
 * Receives and decodes one proto buf message from a TCP socket.
 * Arg 1 - file descriptor for the socket
 * Arg 2 - int value for recv flags
 * Returns a table with the decoded contents of the message on success.
 * 		Also returns the size in bytes of the received message as the second
 * 		return value. Interrupts script execution on error. If nothing is
 * 		received then nil is returned.
 */
int ProtoBufLuaLibrary::recvMsg(lua_State *lua_state)
{
	char recv_buf[MAX_MESSAGE_SIZE];
	bool keep_trying = true;
	int max_timeouts = 5;
	int timeout_count = 0;
	char *input_p = recv_buf;
	bool got_something = false;
	int msg_len = 0;
	int bytes_left = sizeof(msg_len); //first bytes to get are the length bytes
	int ret_val = 0;
	int ret_errno = 0;

	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	int proto_buf_type = luaL_checkint(lua_state, LUA_SECOND_ARG);
	int recv_flags = luaL_optint(lua_state, LUA_THIRD_ARG, 0);

	while(keep_trying)
	{
		int recv_rc = recv(fd, input_p, bytes_left, recv_flags);

		if( recv_rc > 0)
		{
			got_something = true;
			bytes_left -= recv_rc;
			input_p += recv_rc;

			if(bytes_left == 0)
			{
				if(msg_len == 0)
				{
					memcpy(&msg_len, recv_buf, sizeof(msg_len));
					msg_len = ntohl(msg_len);
					bytes_left = msg_len;
					input_p = recv_buf; //Throw away the message length received

					if( msg_len <= 0 || msg_len > MAX_MESSAGE_SIZE)
					{
						return luaL_error(lua_state,
							"received a message with invalid size, msg_size = %d, max_size = %d",
							msg_len, MAX_MESSAGE_SIZE);
					}
				}
				else
				{
					//Message is done so we can return it to the caller
					keep_trying = false;
					ret_val = RECV_GOT_MSG;
				}
			}
		}
		else if(recv_rc == 0)
		{
			return luaL_error(lua_state, "recv failed, connection closed by remote");
		}
		else
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				//When the recv call times out it's usually because we have
				//nothing no the wire. Some defensive programming here to make
				//sure we don't loop forever if a problem occurs. So
				//If we timeout 5 times after we've received a partial message
				//then we return and error to the connection can be torn down
				if(got_something)
				{
					return luaL_error(lua_state,
						"recv timedout but only got a partial message");
				}
				else
				{
					//return control to caller if this is a "nothing to receive"
					//time out
					ret_val = RECV_GOT_NOTHING;
					ret_errno = errno;
				}
			}
			else if( errno == EINTR)
			{
				//Ignore interrupts just keep going
			}
			else
			{
				char errno_str[256];
				return luaL_error(lua_state,
					"recv failed with errno(%d) = %d",
					errno, strerror_r(errno, errno_str, sizeof(errno_str)));
			}
		}
	}

	if(ret_val == RECV_GOT_MSG)
	{
		bool rc = ProtoBufUtility::parseFromArrayToLua(lua_state,
			(ProtoBufUtility::ProtoBufType)proto_buf_type, recv_buf, msg_len);

		if(rc == false)
		{
			stringstream hex_dump;
			hex_dump << hex;
			hex_dump << setfill('0');
			unsigned char *byte = (unsigned char *)recv_buf;
			for(int i = 0; i < msg_len; i++)
			{
				//mimic wireshark output
				if( i % 16 == 0)
				{
					hex_dump << endl;
				}
				else if( i % 8 == 0)
				{
					hex_dump << " ";
				}

				hex_dump << setw(2) << (unsigned short)byte [i]<< " ";
			}
			return luaL_error(lua_state, "Cannot parse received message: %s", hex_dump.str().c_str());
		}
	}
	else
	{
		lua_pushnil(lua_state);
	}

	lua_pushinteger(lua_state, msg_len);
	return 2;
}

/*
 * Encodes and send one protocol buffer message to the given socket.
 * Arg 1 - file descriptor for the socket
 * Arg 2 - table that contains the non encoded values of the message.
 * 		This must following a strict format based on the .proto file that
 * 		defines the message.
 * Returns the number of bytes successfully sent. Interrupts script execution on error.

 */
int ProtoBufLuaLibrary::sendMsg(lua_State *lua_state)
{
	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	luaL_checktype(lua_state, LUA_SECOND_ARG, LUA_TTABLE);
	int proto_buf_type = luaL_checkint(lua_state, LUA_THIRD_ARG);
	int send_flags = luaL_optint(lua_state, LUA_FOURTH_ARG, 0);

	lua_settop(lua_state, 2); //serialize expectes the message to be at the top so make it so

	string send_buf;
	send_buf.reserve(MAX_MESSAGE_SIZE);
	bool rc = ProtoBufUtility::serializeFromLuaToString(lua_state,
		(ProtoBufUtility::ProtoBufType)proto_buf_type, send_buf);

	if( rc == false)
	{
		return luaL_error(lua_state, "Could not serialize message");
	}

	int bytes_sent = send(fd, send_buf.data(), send_buf.size(), send_flags);
	if( bytes_sent == -1)
	{
		char errno_str[256];
		return luaL_error(lua_state, "Send failed with error(%d) = %s",
			errno, strerror_r(errno, errno_str, sizeof(errno_str)));
	}

	lua_pushinteger(lua_state, bytes_sent);
	return 1;
}
