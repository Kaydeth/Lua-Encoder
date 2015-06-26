/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	SocketLuaLibrary.cpp
 * \version	$Revision:/main/cr48624a/CHECKEDOUT $
 * \date	$Date:23-Jun-2015 14:53:37 $
 * \product	DSC test tools
 * \brief	expose the C socket API to the Lua environment
 */

#include "SocketLuaLibrary.h"
#include "SocketManager.h"
#include <string>
#include <cstring>
#include <lauxlib.h>
#include <lualib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>

using namespace std;

/*
 * Loads all the functions defined in this library into the Lua environment
 * \param lua_state - the state to load all the functions
 * \param socket_mgr - upvalue pointer used by some of the socket functions
 */
void SocketLuaLibrary::loadLibrary(lua_State* lua_state, SocketManager *socket_mgr)
{
	SocketManager **temp;

	temp = (SocketManager **)lua_newuserdata(lua_state, sizeof(SocketManager*));
	*temp = socket_mgr;
	lua_pushcclosure(lua_state, SocketLuaLibrary::luaSocket, 1);
	lua_setglobal(lua_state, "socket");

	lua_pushcfunction(lua_state, SocketLuaLibrary::luaListen);
	lua_setglobal(lua_state, "listen");

	lua_pushcfunction(lua_state, SocketLuaLibrary::luaBind);
	lua_setglobal(lua_state, "bind");

	temp = (SocketManager **)lua_newuserdata(lua_state, sizeof(SocketManager*));
	*temp = socket_mgr;
	lua_pushcclosure(lua_state, SocketLuaLibrary::luaAccept, 1);
	lua_setglobal(lua_state, "accept");

	lua_pushcfunction(lua_state, SocketLuaLibrary::luaConnect);
	lua_setglobal(lua_state, "connect");

	lua_pushcfunction(lua_state, SocketLuaLibrary::luaSend);
	lua_setglobal(lua_state, "send");

	lua_pushcfunction(lua_state, SocketLuaLibrary::luaRecv);
	lua_setglobal(lua_state, "recv");

	lua_pushcfunction(lua_state, SocketLuaLibrary::luaPoll);
	lua_setglobal(lua_state, "poll");

	lua_pushcfunction(lua_state, SocketLuaLibrary::luaShutdown);
	lua_setglobal(lua_state, "shutdown");

	temp = (SocketManager **)lua_newuserdata(lua_state, sizeof(SocketManager*));
	*temp = socket_mgr;
	lua_pushcclosure(lua_state, SocketLuaLibrary::luaClose, 1);
	lua_setglobal(lua_state, "close");

	lua_pushcfunction(lua_state, SocketLuaLibrary::sockopt_recv_timeout);
	lua_setglobal(lua_state, "sockopt_recv_timeout");

	lua_pushcfunction(lua_state, SocketLuaLibrary::time_ms);
	lua_setglobal(lua_state, "time_ms");

	lua_pushinteger(lua_state, AF_INET);
	lua_setglobal(lua_state, "AF_INET");

	lua_pushinteger(lua_state, AF_INET6);
	lua_setglobal(lua_state, "AF_INET6");

	lua_pushinteger(lua_state, POLLIN);
	lua_setglobal(lua_state, "POLLIN");
	lua_pushinteger(lua_state, POLLPRI);
	lua_setglobal(lua_state, "POLLPRI");
	lua_pushinteger(lua_state, POLLOUT);
	lua_setglobal(lua_state, "POLLOUT");
	lua_pushinteger(lua_state, POLLRDHUP);
	lua_setglobal(lua_state, "POLLRDHUP");
	lua_pushinteger(lua_state, POLLERR);
	lua_setglobal(lua_state, "POLLERR");
	lua_pushinteger(lua_state, POLLHUP);
	lua_setglobal(lua_state, "POLLHUP");
	lua_pushinteger(lua_state, POLLNVAL);
	lua_setglobal(lua_state, "POLLNVAL");

	lua_pushinteger(lua_state, EAGAIN);
	lua_setglobal(lua_state, "EAGAIN");

}

/*
 * Expose the socket C api. The functions signature is identical to the
 * 	Linux socket function.
 * Upvalue 1 - expects a socket manager to be in the first upvalue
 * 	position.
 * Arg 1 - Lua number for the family type of the socket, mandatory
 * Arg 2 - Lua number for the socket type of the socket, defaults to SOCK_STREAM
 * Arg 3 - Lua number for the protocol of the socket, defaults to 0
 * Returns a Lua number the value of the file descriptor if the socket is
 * 	successfully opened. Interrupts execution on error.
 */
int SocketLuaLibrary::luaSocket(lua_State* lua_state)
{
	SocketManager* socket_manager = *(SocketManager **)
		(lua_touserdata(lua_state, lua_upvalueindex(1)));

	if(socket_manager == NULL)
	{
		return luaL_argerror(lua_state, LUA_FIRST_ARG,
			"expected upvalue not found");
	}

	int socket_family = luaL_checkint(lua_state, LUA_FIRST_ARG);

	int socket_type = luaL_optint(lua_state, LUA_SECOND_ARG, SOCK_STREAM);

	int protocol = luaL_optint(lua_state, LUA_THIRD_ARG, 0);;

	int fd = socket(socket_family, socket_type, protocol);

	if( fd == -1)
	{
		luaL_error(lua_state, "socket failed for family %d, type %d, "
			"protocol %d, errno = %d(%s)",
			socket_family, socket_type, protocol, errno, strerror(errno));
	}

	socket_manager->addSocket(fd);

	lua_pushinteger(lua_state, fd);

	return 1;
}

/*
 * Expose the bind C api.
 * Upvalue 1 - expects a socket manager to be in the first upvalue
 * 	position.
 * Arg 1 - Lua number for socket to bind to, mandatory
 * Arg 2 - string for the address to bind to, mandatory. Address can be a
 * 	host name or ip address.
 * Arg 3 - Lua number for the port to bind to, mandatory
 * Arg 4 - Lua number for the socket family to use, defaults to SOCK_STREAM
 * Returns 0 on success. Interrupts execution on error.
 */
int SocketLuaLibrary::luaBind(lua_State* lua_state)
{
	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	const char *address = luaL_checkstring(lua_state, LUA_SECOND_ARG);
	const char* port = luaL_checkstring(lua_state, LUA_THIRD_ARG);
	int socket_family = luaL_optint(lua_state, LUA_FOURTH_ARG, AF_INET);

	struct addrinfo *pmulti;
	struct addrinfo hint;

	// set hint to the family being used
	memset(&hint, 0, sizeof(hint));
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_family = socket_family;
	hint.ai_flags = AI_NUMERICSERV ;

	printf("LSDEBUG: get addrinfo for address %s\n", address);
	int get_addr_rc = getaddrinfo(address, port, &hint, &pmulti);

	if(get_addr_rc != 0)
	{
		return luaL_error(lua_state, "getaddrinfo failed for socket %d, "
			"address %s, port %s, family %d, error = %d(%s)",
			fd, address, port, socket_family, get_addr_rc, gai_strerror(get_addr_rc) );
	}

	int bind_rc = 0;
	int count = 0;
	struct addrinfo* counter = pmulti;
	while(counter != NULL)
	{
		count++;

		int buf_len = INET_ADDRSTRLEN;
		char buffer[buf_len];

		inet_ntop(counter->ai_family, &((sockaddr_in*)counter->ai_addr)->sin_addr, buffer, buf_len);
		printf("found %s\n", buffer);

		counter = counter->ai_next;
	}
	printf("getaddrinfo found %d\n", count);

	if(pmulti->ai_family == AF_INET)
	{
		bind_rc = bind(fd, pmulti->ai_addr, sizeof(sockaddr_in));
	}
	else if(pmulti->ai_family == AF_INET6)
	{
		bind_rc = bind(fd, pmulti->ai_addr, sizeof(sockaddr_in6));
	}
	else
	{
		freeaddrinfo(pmulti);
		return luaL_error(lua_state, "getaddrinfo returned invalid family (%d) "
			"for socket %d, address %s,port %s, family %d",
			pmulti->ai_family, fd, address, port, socket_family);
	}

	freeaddrinfo(pmulti);

	lua_pushinteger(lua_state, bind_rc);

	return 1;
}

/*
 * Expose the listen C api.
 * Arg 1 - file descriptor for the socket
 * Arg 2 - backlog value used for this socket.
 * Returns nothing. Interrupts script execution on error.
 */
int SocketLuaLibrary::luaListen(lua_State* lua_state)
{
	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	//TODO: what should the default backlog be?
	int backlog = luaL_optint(lua_state, LUA_SECOND_ARG, 1);

	if (listen(fd, backlog) == -1)
	{
		luaL_error(lua_state, "listen failed for fd = %d, errno = %d(%s)",
			fd, errno, strerror(errno));
	}

	return 0;
}


/*
 * Expose the accept C api.
 * Upvalue 1 - expects a socket manager to be in the first upvalue
 * 	position.
 * Arg 1 - Lua number for socket that is accepting, mandatory
 * Arg 2 - string for the remote address to expect, mandatory. Address can be a
 * 	host name or ip address.
 * Arg 3 - Lua number for the remote port to expect, mandatory
 * Arg 4 - Lua number for the socket family to use, defaults to SOCK_STREAM
 * Returns the new file descriptor created by the accept call.
 * 	Interrupts script execution on error.
 */
int SocketLuaLibrary::luaAccept(lua_State* lua_state)
{
	 SocketManager* socket_manager = *(SocketManager **)
		(lua_touserdata(lua_state, lua_upvalueindex(1)));

	if(socket_manager == NULL)
	{
		return luaL_argerror(lua_state, LUA_FIRST_ARG,
			"expected upvalue not found");
	}

	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	const char *address = luaL_checkstring(lua_state, LUA_SECOND_ARG);
	int port = luaL_checkint(lua_state, LUA_THIRD_ARG);
	int socket_family = luaL_optint(lua_state, LUA_FOURTH_ARG, AF_INET);

	int new_fd = 0;

	while(new_fd == 0)
	{
		sockaddr_storage addr_ret;
		socklen_t size_ret = sizeof(addr_ret);
		new_fd = accept(fd, (sockaddr*)&addr_ret, &size_ret);

		if (new_fd == -1)
		{
			luaL_error(lua_state, "accept failed for fd = %d, errno = %d(%s)",
				fd, errno, strerror(errno));
		}

		int incoming_port = 0;
		if(addr_ret.ss_family == AF_INET)
		{
			sockaddr_in* sockaddr4 = (sockaddr_in*)&(addr_ret);

			 incoming_port = ntohs(sockaddr4->sin_port);
		}
		else if(addr_ret.ss_family == AF_INET6)
		{
			sockaddr_in6* sockaddr6 = (sockaddr_in6*)&(addr_ret);

			 incoming_port = ntohs(sockaddr6->sin6_port);
		}
		else
		{
			return luaL_error(lua_state, "accept returned invalid family (%d) "
				"for socket %d", addr_ret.ss_family, fd);
		}

		if( incoming_port != port)
		{
			printf("LSDEBUG: rejecting connection from port %d\n", incoming_port);
			int rc = shutdown(new_fd, SHUT_RDWR);
			if( rc == -1)
			{
				luaL_error(lua_state,
					"shutdown failed for fd = %d, errno = %d(%s)", fd, errno,
					strerror(errno));
			}

			rc = close(new_fd);
			if( rc == -1)
			{
				return luaL_error(lua_state,
					"close failed for fd = %d, errno = %d(%s)", fd, errno,
					strerror(errno));
			}

			new_fd = 0;
		}
	}


	socket_manager->addSocket(new_fd);

	lua_pushinteger(lua_state, new_fd);

	return 1;
}

/*
 * Expose the connect C api.
 * Arg 1 - lua integer for file descriptor of socket
 * Arg 2 - lua string for remote host name to connect to
 * Arg 3 - lua integer for remote port number to connect to
 * Arg 4 - lua number for the family type of the socket
 * 	Returns nothing. Interrupts script execution on error.
 */
int SocketLuaLibrary::luaConnect(lua_State* lua_state)
{
	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	const char* address = luaL_checkstring(lua_state, LUA_SECOND_ARG);
	const char* port = luaL_checkstring(lua_state, LUA_THIRD_ARG);
	int socket_family = luaL_optint(lua_state, LUA_FOURTH_ARG, AF_INET);

	struct addrinfo *pmulti;
	struct sockaddr_in6 *paddr = NULL;
	struct addrinfo hint;

	// set hint to the family being used
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = socket_family;
	hint.ai_flags = AI_NUMERICSERV ;

	int get_addr_rc = getaddrinfo(address, port, &hint, &pmulti);

	if(get_addr_rc != 0)
	{
		return luaL_error(lua_state, "getaddrinfo failed for socket %d, "
			"address %s, port %s, family %d, error = %d(%s)",
			fd, address, port, socket_family, get_addr_rc,
			gai_strerror(get_addr_rc) );
	}

	int connect_rc = 0;
	if(pmulti->ai_family == AF_INET)
	{
		connect_rc = connect(fd, pmulti->ai_addr, sizeof(sockaddr_in));
	}
	else if(pmulti->ai_family == AF_INET6)
	{
		connect_rc = connect(fd, pmulti->ai_addr, sizeof(sockaddr_in6));
	}
	else
	{
		freeaddrinfo(pmulti);
		return luaL_error(lua_state, "getaddrinfo returned invalid family (%d) "
			"for socket %d, address %s, port %s, family %d",
			pmulti->ai_family, fd, address, port, socket_family);
	}

	if( connect_rc == -1)
	{
		luaL_error(lua_state, "connect failed for fd = %d, errno = %d(%s)",
			fd, errno, strerror(errno));
	}

	lua_pushinteger(lua_state, connect_rc);

	return 1;
}

/*
 * Expose the send C api.
 * Arg 1 - lua integer for file descriptor of socket
 * Arg 2 - lua string or table of lua numbers for the data to send. If the
 * 		argument is a table, each table entry should represent a single byte
 * 		of the data to be sent and the table should be an array using the
 * 		numerical keys from 1 to maximum bytes in the data.
 * Arg 3 - lua number for the send flags, defaults to 0
 * Returns nothing. Interrupts script execution on error.
 */
int SocketLuaLibrary::luaSend(lua_State *lua_state)
{
	int bytes_sent;
	const char* data;
	char buf[10000]; //TODO: what is the max buffer size?
	int value;
	int data_size;

	memset(buf, 0, sizeof(buf));

	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	int send_flags = luaL_optint(lua_state, LUA_THIRD_ARG, 0);

	if( lua_isstring(lua_state, LUA_SECOND_ARG) )
	{
		data = lua_tostring(lua_state, LUA_SECOND_ARG);
		lua_len(lua_state, LUA_SECOND_ARG);
		data_size = lua_tointeger(lua_state, LUA_TOP);
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
			if( !lua_isnumber(lua_state, LUA_TOP -1) )
			{
				luaL_error(lua_state, "BAD KEY");
			}

			key = (int)lua_tonumber(lua_state, LUA_TOP -1);
			/* Lua tables start with index 1 */
			if( (key - 1) < sizeof(buf))
			{
				/* If the table has missing entries then those indices
				 * default to zero */
				if( !lua_isnumber(lua_state, LUA_TOP) )
				{
					luaL_error(lua_state, "BAD VALUE");
				}

				buf[key - 1] = (int)lua_tonumber(lua_state, LUA_TOP);

				if( key > max_key)
				{
					max_key = key;
				}
			}
			else
			{
				return luaL_error(lua_state, "send fails for fd = %d, data length "
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
		luaL_argerror(lua_state, LUA_SECOND_ARG, "expected String or Table value");
	}

	bytes_sent = send(fd, data, data_size, send_flags);
	if( bytes_sent == -1)
	{
		return luaL_error(lua_state,
			"send fails for fd = %d, data_size = %d, errno = %d(%s)", fd,
			data_size, errno, strerror(errno));
	}

	lua_pushinteger(lua_state, bytes_sent);

	return 1;
}

/*
 * Expose the recv C api.
 * Arg 1 - lua integer for file descriptor of socket
 * Arg 2 - lua number for the receive flags, defaults to 0
 * Arg 2 - lua string for the return format of the data received. "STRING"
 * 		will return the data as an ASCII string. "HEX" will return the data
 * 		as an array of integer values.
 * Return 1 - A lua string or table of lua numbers(see Arg 2)
 * 		which is the data received. The table format return is an array
 * 		with keys starting from 1 to the maximum bytes in the message.
 * Return 2 - A lua number that is the number of bytes received. -1 indicates
 * 		and error occured. 0 indicates that the association was shutdown by
 * 		the peer
 * Return 3 - A lua number that is the errno value if an error occurred
 * Return 4 - A lua string that is the text representation of the errno
 */
int SocketLuaLibrary::luaRecv(lua_State *lua_state)
{
	int bytes_received;
	int recv_errno;

	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);

	bool user_buffer = false;
	int buf_start = 1; // Lua tables start at index 1
	if(!lua_isnil(lua_state, LUA_SECOND_ARG))
	{
		luaL_checktype(lua_state, LUA_SECOND_ARG, LUA_TTABLE);
		user_buffer = true;
		//Put the user buffer at the top of the stack so it can be easily
		//returned
		lua_pushvalue(lua_state, LUA_SECOND_ARG);

		buf_start = luaL_optint(lua_state, LUA_FIFTH_ARG, buf_start);
	}

	int buf_size = luaL_checkint(lua_state, LUA_THIRD_ARG);
	int recv_flags = luaL_optint(lua_state, LUA_FOURTH_ARG, 0);
	string return_format = luaL_optstring(lua_state, LUA_SIXTH_ARG, "HEX");

	if(return_format != "HEX" && return_format != "STRING")
	{
		luaL_argerror(lua_state, LUA_SECOND_ARG,
			"valid values are \"HEX\" and \"STRING\"");
	}

	char buf[buf_size + 1];

	bytes_received = recv(fd, buf, buf_size, recv_flags);
	recv_errno = errno;

	if( bytes_received == -1)
	{
		buf[0] = '\0';
	}
	else
	{
		buf[bytes_received] = '\0';
	}

	if(return_format == "STRING")
	{
		lua_pushlstring(lua_state, buf, bytes_received);
	}
	else /* return_format == "HEX" */
	{
		int i = 0;

		int table_index = LUA_TOP - 2;
		if(!user_buffer)
		{
			lua_newtable(lua_state);
		}
		//else the user buffer has already been push to the top of the stack

		for(i = 0; i < bytes_received; i ++)
		{
			/* Lua arrays start at 1!!! */
			lua_pushinteger(lua_state, i+buf_start);
			lua_pushinteger(lua_state, (unsigned char)buf[i]);

			lua_settable(lua_state, LUA_TOP - 2);
		}
	}

	lua_pushinteger(lua_state,bytes_received);

	if( bytes_received == -1 )
	{
		lua_pushinteger(lua_state, recv_errno);
		lua_pushstring(lua_state, strerror(recv_errno));

		return 4;
	}

	return 2;
}

/*
 * Expose the poll C api
 * Arg1 - table that mirrors the structure of a pollfd array
 * Arg2 - timeout
 */
int SocketLuaLibrary::luaPoll(lua_State *lua_state)
{
	static string fd_mem = "fd";
	static string events_mem = "events";
	static string revents_mem = "revents";

	luaL_checktype(lua_state, LUA_FIRST_ARG, LUA_TTABLE);

	int timeout_ms = luaL_checkint(lua_state, LUA_SECOND_ARG);
	int num_fds = luaL_len(lua_state, LUA_FIRST_ARG);

	if(num_fds <= 0)
	{
		return luaL_argerror(lua_state, LUA_SECOND_ARG,
			"Must give at least one socket");
	}

	struct pollfd poll_array[num_fds];
	memset(poll_array, 0, sizeof(pollfd) * num_fds);

	for(int i = 1; i <= num_fds; i++)
	{
		lua_pushinteger(lua_state, i);
		lua_gettable(lua_state, LUA_FIRST_ARG);
		luaL_checktype(lua_state, LUA_TOP, LUA_TTABLE);

		lua_getfield(lua_state, LUA_TOP, fd_mem.c_str());
		if(luaL_checkint(lua_state, LUA_TOP))
		{
			poll_array[i - 1].fd = lua_tointeger(lua_state, LUA_TOP);
		}
		lua_pop(lua_state, 1); //pop fd off stack

		lua_getfield(lua_state, LUA_TOP, events_mem.c_str());
		if(luaL_checkint(lua_state, LUA_TOP))
		{
			poll_array[i - 1].events = lua_tointeger(lua_state, LUA_TOP);
		}

		lua_pop(lua_state, 2); // pop the events value and table off the stack
	}

	int poll_rc = poll(poll_array, num_fds, timeout_ms);
	int errno_val = errno;

	lua_pushinteger(lua_state, poll_rc);

	if(poll_rc == 0)
	{
		return 1;
	}
	else if(poll_rc < 0)
	{
		lua_pushinteger(lua_state, errno);
		lua_pushstring(lua_state, strerror(errno));
		return 3;
	}

	//Find all fds that have an event are push that into the Lua table
	int j = 0; //number of descriptors with an event
	for(int i =0; i < num_fds && j < poll_rc; i++)
	{
		if(poll_array[i].revents != 0)
		{
			lua_pushinteger(lua_state, i + 1);
			lua_gettable(lua_state, LUA_FIRST_ARG);
			lua_pushinteger(lua_state, poll_array[i].revents);
			//Type has already been checked earlier
			lua_setfield(lua_state, LUA_TOP - 1, revents_mem.c_str());

			lua_pop(lua_state, 1);

			j++;
		}
	}

	return 1;
}

/*
 * Expose the sctp_shutdown C api.
 * Arg 1 - file descriptor for the socket
 * Returns nothing. Interrupts script execution on error.
 */
int SocketLuaLibrary::luaShutdown(lua_State *lua_state)
{
	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	int how = luaL_optint(lua_state, LUA_SECOND_ARG, SHUT_RDWR);

	printf("LSDEBUG: shutting down fd = %d\n", fd);
	int rc = shutdown(fd, how);
	printf("LSDEBUG: sctp_shutdown returned %d, errno = %d\n", rc, errno);
	if( rc == -1)
	{
		printf("LSDEBUG: sctp_shutdown returned -1, errno = %d\n", errno);
		luaL_error(lua_state,
			"shutdown fails for fd = %d, errno = %d(%s)", fd, errno,
			strerror(errno));
	}

	lua_pushinteger(lua_state, rc);

	return 1;
}

/*
 * Expose the close C api.
 * Arg 1 - file descriptor for the socket
 * Returns nothing. Interrupts script execution on error.
 */
int SocketLuaLibrary::luaClose(lua_State *lua_state)
{
	void* temp_ptr = lua_touserdata(lua_state, lua_upvalueindex(1));

	if(temp_ptr == NULL)
	{
		return luaL_error(lua_state, "expected upvalue not found, "
			"nothing at index (1)");
	}

	SocketManager* socket_manager = *(SocketManager **)
		(lua_touserdata(lua_state, lua_upvalueindex(1)));

	if(socket_manager == NULL)
	{
		return luaL_error(lua_state,
			"expected upvalue not found at index (1), upvalue pointer is NULL");
	}

	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);

	int rc = close(fd);

	if( rc == -1)
	{
		return luaL_error(lua_state,
			"close failed for fd = %d, errno = %d(%s)", fd, errno,
			strerror(errno));
	}

	socket_manager->removeSocket(fd);
}

/*
 * Set the SO_RCVTIMEO socket option through the setsockopt API
 */
int SocketLuaLibrary::sockopt_recv_timeout(lua_State *lua_state)
{
	int fd = luaL_checkint(lua_state, LUA_FIRST_ARG);
	int timeout_ms = 0;

	if(lua_gettop(lua_state) > 1)
	{
		timeout_ms = luaL_checkinteger(lua_state, LUA_SECOND_ARG);
	}

	struct timeval timeout = {0};

	int rc = 0;
	if(timeout_ms != 0)
	{
		if(timeout_ms >= 1000)
		{
			timeout.tv_sec = timeout_ms / 1000;
		}
		timeout.tv_usec = (timeout_ms % 1000) * 1000;


		rc = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		lua_pushinteger(lua_state, rc);
	}
	else
	{
		socklen_t len = sizeof(timeout);
		rc = getsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, &len);

		if( rc == -1)
		{
			lua_pushnil(lua_state);
		}
		else
		{
			lua_pushinteger(lua_state, timeout.tv_sec * 1000 + timeout.tv_usec / 1000);
		}
	}

	if(rc == -1)
	{
		int error = errno;
		lua_pushinteger(lua_state, error);
		lua_pushstring(lua_state, strerror(error));

		return 3;
	}

	return 1;
}

/*
 * Get monotonic time in milliseconds
 * Returns a number, monotonic time in milliseconds
 */
int SocketLuaLibrary::time_ms(lua_State *lua_state)
{
	struct timespec tp = { 0,0 } ; /* tv_sec, tv_nsec */

	/* get uptime */
	clock_gettime( CLOCK_MONOTONIC , &tp );

	uint64_t milliseconds = tp.tv_sec * 1000;
	milliseconds += (tp.tv_nsec / (uint64_t)1000000);

	lua_pushnumber(lua_state, milliseconds);

	return 1;
}
