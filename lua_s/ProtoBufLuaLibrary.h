/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	ProtoBufLuaLibrary.h
 * \version	$Revision:/main/1 $
 * \date	$Date:3-Sep-2013 10:14:35 $
 * \product	DSC test tools
 * \brief	exposes send and received functions that will accept and return
 * 		a Lua table that defines a message based on a generated Protocol Buffer
 * 		class.
 */

#ifndef _PROTO_BUF_LUA_LIBRARY_H_
#define _PROTO_BUF_LUA_LIBRARY_H_

#include <lua.h>

class ProtoBufLuaLibrary
{

public:
	typedef enum{
		RECV_GOT_MSG,
		RECV_GOT_NOTHING,
		RECV_ERROR,
		RECV_CONNECTION_CLOSED
	} RecvRet;

	static void loadLibrary(lua_State* lua_state);

	static int recvMsg(lua_State* lua_state);
	static int sendMsg(lua_State* lua_state);

	static int MAX_MESSAGE_SIZE;
};

#endif /* _PROTO_BUF_LUA_LIBRARY_H_ */
