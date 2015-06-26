/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	SocketLuaLibrary.h
 * \version	$Revision:/main/cr48624a/CHECKEDOUT $
 * \date	$Date:23-Jun-2015 15:15:08 $
 * \product	DSC test tools
 * \brief	expose the C socket API to the Lua environment
 */

#ifndef _SOCKET_LUA_LIBRARY_H_
#define _SOCKET_LUA_LIBRARY_H_

#include "LuaDefines.h"
#include <lua.h>

class SocketManager;

class SocketLuaLibrary
{
public:
	static void loadLibrary(lua_State* lua_state, SocketManager *socket_mgr);

	static int luaSocket(lua_State* lua_state);
	static int luaBind(lua_State* lua_state);
	static int luaListen(lua_State* lua_state);
	static int luaAccept(lua_State* lua_state);
	static int luaConnect(lua_State* lua_state);
	static int luaSend(lua_State* lua_state);
	static int luaRecv(lua_State* lua_state);
	static int luaPoll(lua_State *lua_state);
	static int luaShutdown(lua_State* lua_state);
	static int luaClose(lua_State* lua_state);

	static int sockopt_recv_timeout(lua_State* lua_state);

	static int time_ms(lua_State* lua_state);
};

#endif /* _SOCKET_LUA_LIBRARY_H_ */
