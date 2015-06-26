/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	FlexSctpLuaLibrary.h
 * \version	$Revision:/main/1 $
 * \date	$Date:3-Sep-2013 10:14:25 $
 * \product	DSC test tools
 * \brief	expose SCTP API to open and manage SCTP sockets through the
 * 		flexible SCTP C API.
 */

#ifndef _FLEX_SCTP_LUA_LIBRARY_H_
#define _FLEX_SCTP_LUA_LIBRARY_H_

#include "LuaDefines.h"
#include <lua.h>

class SocketManager;

class FlexSctpLuaLibrary
{
public:
	static void loadLibrary(lua_State* lua_state, SocketManager *socket_mgr);

	static int luaSctpSocket(lua_State* lua_state);
	static int luaSctpListen(lua_State* lua_state);
	static int luaSctpAccept(lua_State* lua_state);
	static int luaSctpConnect(lua_State* lua_state);
	static int luaSctpSend(lua_State* lua_state);
	static int luaSctpRecv(lua_State* lua_state);
	static int luaSctpShutdown(lua_State* lua_state);
	static int luaSctpAbort(lua_State* lua_state);
};

#endif /* _FLEX_SCTP_LUA_LIBRARY_H_ */
