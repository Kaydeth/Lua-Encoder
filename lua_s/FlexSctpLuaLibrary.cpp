/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	FlexSctpLuaLibrary.cpp
 * \version	$Revision:/main/2 $
 * \date	$Date:19-Jun-2015 10:22:23 $
 * \product	DSC test tools
 * \brief	expose SCTP API to open and manage SCTP sockets through the
 * 		flexible SCTP C API.
 */

#include "FlexSctpLuaLibrary.h"
#include <ulcm_flex_sctp.h>
#include <ulcm_ip_addrs.h>
#include <string>
#include <lauxlib.h>
#include <lualib.h>
#include "SocketManager.h"
#include <cstring>

using namespace std;

/*
 * Loads all the functions defined in this library into the Lua environment
 * \param lua_state - the state to load all the functions
 * \param socket_mgr - upvalue pointer used by some of the socket functions
 */
void FlexSctpLuaLibrary::loadLibrary(lua_State* lua_state, SocketManager *socket_mgr)
{
	SocketManager **temp;

	temp = (SocketManager **)lua_newuserdata(lua_state, sizeof(SocketManager*));
	*temp = socket_mgr;
	lua_pushcclosure(lua_state, FlexSctpLuaLibrary::luaSctpSocket, 1);
	lua_setglobal(lua_state, "sctp_socket");

	lua_pushcfunction(lua_state, FlexSctpLuaLibrary::luaSctpListen);
	lua_setglobal(lua_state, "sctp_listen");

	temp = (SocketManager **)lua_newuserdata(lua_state, sizeof(SocketManager*));
	*temp = socket_mgr;
	lua_pushcclosure(lua_state, FlexSctpLuaLibrary::luaSctpAccept, 1);
	lua_setglobal(lua_state, "sctp_accept");

	lua_pushcfunction(lua_state, FlexSctpLuaLibrary::luaSctpConnect);
	lua_setglobal(lua_state, "sctp_connect");

	lua_pushcfunction(lua_state, FlexSctpLuaLibrary::luaSctpSend);
	lua_setglobal(lua_state, "sctp_send");

	lua_pushcfunction(lua_state, FlexSctpLuaLibrary::luaSctpRecv);
	lua_setglobal(lua_state, "sctp_recv");

	lua_pushcfunction(lua_state, FlexSctpLuaLibrary::luaSctpShutdown);
	lua_setglobal(lua_state, "sctp_shutdown");

	lua_pushcfunction(lua_state, FlexSctpLuaLibrary::luaSctpAbort);
	lua_setglobal(lua_state, "sctp_abort");
}

/*+
 * Expose the sctp_socket C api.
 * Upvalue 1 - expects a socket manager to be in the first upvalue
 * 	position. The socket manager is used to create the socket.
 * Arg 1 - lua string for host name to locally bind for.
 * Arg 2 - lua integer for port number to locally bind for.
 * Returns the value of the file descriptor if the socket is successfully
 * 	opened. This file desciptor is used for all further sctp calls.
 * 	Interrupts script execution on error.
 */
int FlexSctpLuaLibrary::luaSctpSocket(lua_State* lua_state)
{
	SocketManager* socket_manager;

	socket_manager = *(SocketManager **)
		(lua_touserdata(lua_state, lua_upvalueindex(1)));

	if(socket_manager == NULL)
	{
		return luaL_argerror(lua_state, LUA_FIRST_ARG,
			"expected upvalue not found");
	}

	const char* host_name = luaL_optstring(lua_state, LUA_FIRST_ARG, NULL);
	int port = luaL_optint(lua_state, LUA_SECOND_ARG, 30000);

	struct sockaddr_in6 *ip_addrs = NULL;
	int num_addrs = ulcm_get_ip_addrs(host_name, port, &ip_addrs);
	if( ip_addrs == NULL)
	{
		return luaL_error(lua_state, "ulcm_get_ip_addrs failed, "
			"returned NULL. Address = %s, port = %d, errno = %d(%s)",
			host_name, port, -num_addrs, strerror(-num_addrs));
	}
	else if(num_addrs == 0)
	{
		ulcm_free_ip_addrs(ip_addrs);
		return luaL_error(lua_state, "ulcm_get_ip_addrs returned no addrs, "
			"the address lookup failed. Address = %s, port = %d",
			host_name, port);
	}

	int fd = ulcm_flex_sctp_socket(ip_addrs);

	if( ip_addrs != NULL)
	{
		ulcm_free_ip_addrs(ip_addrs);
	}

	if(fd < 0)
	{
		luaL_error(lua_state, "ulcm_flex_sctp_socket failed for %s(%d), "
			"errno = %d(%s)",
			host_name, port, -fd, strerror(-fd));
	}

	socket_manager->addSocket(fd);

	lua_pushinteger(lua_state, fd);

	return 1;
}

/*
 * Expose the sctp_listen C api.
 * Arg 1 - file descriptor for the socket
 * Returns nothing. Interrupts script execution on error.
 */
int FlexSctpLuaLibrary::luaSctpListen(lua_State* lua_state)
{
	int fd;

	fd = luaL_checkint(lua_state, LUA_TOP);

	int ret_code = ulcm_flex_sctp_listen(fd);

	if(ret_code < 0)
	{
		luaL_error(lua_state, "sctp_listen failed for fd = %d, errno = %d(%s)",
			fd, -ret_code, strerror(-ret_code));
	}

	return 0;
}


/*
 * Expose the sctp_accept C api.
 * Upvalue 1 - expects a socket manager to be in the first upvalue
 * 	position. Since sctp_accept may create a new file descriptor the
 * 	socket manager must be used to keep track of each descriptor created.
 * Arg 1 - file descriptor for the socket
 * Returns the new file descriptor created by the accept call.
 * 	Interrupts script execution on error.
 */
int FlexSctpLuaLibrary::luaSctpAccept(lua_State* lua_state)
{
	SocketManager* socket_manager = *(SocketManager **)
		(lua_touserdata(lua_state, lua_upvalueindex(1)));

	if(socket_manager == NULL)
	{
		return luaL_argerror(lua_state, LUA_FIRST_ARG,
			"expected upvalue not found");
	}

	int fd = luaL_checkint(lua_state, LUA_TOP);

	int new_fd = ulcm_flex_sctp_accept(fd);

	if(new_fd < 0)
	{
		luaL_error(lua_state, "sctp_accept failed for fd = %d, errno = %d(%s)",
			fd, -new_fd, strerror(-new_fd));
	}

	socket_manager->addSocket(new_fd);

	lua_pushinteger(lua_state, new_fd);

	return 1;
}

/*
 * Expose the sctp_connect C api.
 * Arg 1 - lua integer for file descriptor of socket
 * Arg 2 - lua string for remote host name to connect to
 * Arg 3 - lua integer for remote port number to connect to
 * Returns nothing. Interrupts script execution on error.
 */
int FlexSctpLuaLibrary::luaSctpConnect(lua_State* lua_state)
{
	int fd = 0;
	const char* host_name;
	int port;

	fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	host_name = luaL_checkstring(lua_state, LUA_SECOND_ARG);
	port = luaL_checkint(lua_state, LUA_THIRD_ARG);

	struct sockaddr_in6 *ip_addrs = NULL;
	int num_addrs = ulcm_get_ip_addrs(host_name, port, &ip_addrs);
	if( ip_addrs == NULL)
	{
		luaL_error(lua_state, "NULL returned when trying to lookup "
			"address for %s(%d), ret_val = %d",
			host_name, port, num_addrs);
	}
	else if(num_addrs == 0)
	{
		luaL_error(lua_state, "Cannot find address for %s(%d)",
			host_name, port);
	}

	int ret_code = ulcm_flex_sctp_connect(fd, ip_addrs);
	if( ret_code < 0)
	{
		luaL_error(lua_state, "sctp_connect failed for fd = %d, errno = %d(%s)",
			fd, -ret_code, strerror(-ret_code));
	}

	ulcm_free_ip_addrs(ip_addrs);

	return ret_code;
}

/*
 * Expose the sctp_send C api.
 * Arg 1 - lua integer for file descriptor of socket
 * Arg 2 - lua string or table of lua numbers for the data to send. If the
 * 		argument is a table, each table entry should represent a single byte
 * 		of the data to be sent and the table should be an array using the
 * 		numerical keys from 1 to maximum bytes in the data.
 * Returns nothing. Interrupts script execution on error.
 */
int FlexSctpLuaLibrary::luaSctpSend(lua_State *lua_state)
{
	int fd = 0;
	int bytes_sent;
	const char* data;
	char buf[64 * 1024];  //SCTP_MAX_DATA_SIZE
	int value;
	int data_size;

	memset(buf, 0, sizeof(buf));

	fd = luaL_checkint(lua_state, LUA_FIRST_ARG);

	if( lua_isstring(lua_state, LUA_SECOND_ARG) )
	{
		data = lua_tostring(lua_state, LUA_SECOND_ARG);
		data_size = strlen(data);
	}
	else if( lua_istable(lua_state, LUA_SECOND_ARG) )
	{
		int key, max_key = 0;

		lua_pushnil(lua_state);
		while( lua_next(lua_state, LUA_TOP - 1) != 0)
		{
			/* The integer keys of the table may not come in order. But we
			 * want the table indices to tell use which byte of data the
			 * value should be placed in */
			key = luaL_checkinteger(lua_state, LUA_TOP - 1);
			/* Lua tables start with index 1 */
			if( (key - 1) < sizeof(buf))
			{
				/* If the table has missing entries then those indices
				 * default to zero */
				buf[key - 1] = luaL_checkinteger(lua_state, LUA_TOP);
				if( key > max_key)
				{
					max_key = key;
				}
			}
			else
			{
				luaL_error(lua_state, "sctp_send fails for fd = %d, data length "
					"was too long ( > %d), data = %s", fd, sizeof(buf), buf);
			}

			/* Pop the value off the stack */
			lua_pop(lua_state, 1);
		}

		data = buf;
		data_size = max_key;
	}
	else
	{
		luaL_argerror(lua_state, LUA_SECOND_ARG, "expected String or Table");
	}

	bytes_sent = ulcm_flex_sctp_send(fd, data, data_size);
	if( bytes_sent < 0)
	{
		luaL_error(lua_state,
			"ulcm_flex_sctp_send fails for fd = %d, data = %s, "
			"errno = %d(%s)", fd,
			data, -bytes_sent, strerror(-bytes_sent));
	}

	lua_pushinteger(lua_state, bytes_sent);

	return 1;
}

/*
 * Expose the sctp_recv C api.
 * Arg 1 - lua integer for file descriptor of socket
 * Arg 2 - lua string for the return format of the data received. "STRING"
 * 		will return the data as an ASCII string. "HEX" will return the data
 * 		as an array of integer values.
 * Return 1 - A lua string or table of lua numbers(see Arg 2)
 * 		which is the data received. The table format return is an array
 * 		with keys starting from 1 to the maximum bytes in the message.
 * Return 2 - A lua number that is the number of bytes received. -1 indicates
 * 		and error occured. 0 indicates that the association was shutdown by
 * 		the peer
 * Return 3 - A lua number that is the errno value if an error occured
 * Return 4 - A lua string that is the text representation of the errno
 */
int FlexSctpLuaLibrary::luaSctpRecv(lua_State *lua_state)
{
	int fd = 0;
	string return_format;
	char buf[64 * 1024]; //SCTP_MAX_DATA_SIZE
	int bytes_received;
	int recv_errno;

	fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	return_format = luaL_optstring(lua_state, LUA_SECOND_ARG, "HEX");

	if(return_format != "HEX" && return_format != "STRING")
	{
		luaL_argerror(lua_state, LUA_SECOND_ARG,
			"valid values are \"HEX\" and \"STRING\"");
	}

	bytes_received = ulcm_flex_sctp_recv(fd, buf, sizeof(buf)-1);
	recv_errno = errno;

	if( bytes_received < 0)
	{
		buf[0] = '\0';
	}
	else
	{
		buf[bytes_received] = '\0';
	}

	if(return_format == "STRING")
	{
		lua_pushstring(lua_state, buf);
	}
	else /* return_format == "HEX" */
	{
		int i = 0;
		lua_newtable(lua_state);

		for(i = 0; i < bytes_received; i ++)
		{
			/* Lua arrays start at 1!!! */
			lua_pushinteger(lua_state, i+1);
			lua_pushinteger(lua_state, (unsigned char)buf[i]);
			lua_settable(lua_state, LUA_TOP - 2);
		}
	}

	lua_pushinteger(lua_state,bytes_received);

	if( bytes_received < 0 )
	{
		lua_pushinteger(lua_state, -bytes_received);
		lua_pushstring(lua_state, strerror(-bytes_received));

		return 4;
	}

	return 2;
}

/*
 * Expose the sctp_shutdown C api.
 * Arg 1 - file descriptor for the socket
 * Returns nothing. Interrupts script execution on error.
 */
int FlexSctpLuaLibrary::luaSctpShutdown(lua_State *lua_state)
{
	int fd = -1;
	int ret_code = 0;

	fd = luaL_checkint(lua_state, LUA_FIRST_ARG);

	ret_code = shutdown(fd, SHUT_RDWR);
	if( ret_code == -1)
	{
		luaL_error(lua_state,
			"shutdown failed for fd = %d, errno = %d(%s)", fd, errno,
			strerror(errno));
	}
}

/*
 * Expose the sctp_abort C api.
 * Arg 1 - file descriptor for the socket
 * Returns nothing. Interrupts script execution on error.
 */
int FlexSctpLuaLibrary::luaSctpAbort(lua_State *lua_state)
{
	int fd = -1;
	int ret_code = 0;

	fd = luaL_checkint(lua_state, LUA_FIRST_ARG);

	ret_code = ulcm_flex_sctp_abort(fd);
	if( ret_code < 0)
	{
		luaL_error(lua_state,
			"sctp_abort fails for fd = %d, errno = %d(%s)", fd, -ret_code,
			strerror(-ret_code));
	}
}

