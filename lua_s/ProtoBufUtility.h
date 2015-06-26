/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	ProtoBufUtility.h
 * \version	$Revision:/main/3 $
 * \date	$Date:6-Dec-2013 10:35:49 $
 * \product	DSC test tools
 * \brief	helper functions encode/decode a protocol message to and from
 * 		a Lua table.
 */

#ifndef _PROTO_BUF_UTILITY_H
#define _PROTO_BUF_UTILITY_H

#include <string>

namespace google
{
	namespace protobuf
	{
		class Message;
	}
};

namespace UlticomADC
{
	class ClusterRoutingMessage;
	class DraMessagePB;
}

struct lua_State;

using namespace std;

class ProtoBufUtility
{
public:
	typedef enum
	{
		CLUSTER_ROUTING_MESSAGE,
		DRA_MESSAGE_PB
	}ProtoBufType;

	static void pushConstants(lua_State* lua_state);

	static bool parseFromArrayToLua(lua_State* lua_state, ProtoBufType type, char* buf, int buf_len);
	static bool serializeFromLuaToString(lua_State* lua_state, ProtoBufType type, string& out_buf);

private:
	static void pushClusterRoutingMessage(lua_State* lua_state);
	static void pullClusterRoutingMessage(lua_State* lua_state);

public:
	const static string HEADER;
	const static string VERSION;
	const static string MESSAGE_TYPE;
	const static string HELLO_PAYLOAD;
	const static string ORIGIN_INSTANCE_NAME;
	const static string CLUSTER_NAME;
	const static string DATABASE_DESCRIPTION;
	const static string INIT_FLAG;
	const static string MORE_FLAG;
	const static string MASTER_FLAG;
	const static string DD_SEQUENCE_NUM;
	const static string LSA_HEADER_LIST;
	const static string REQUEST_PAYLOAD;
	const static string UPDATE_PAYLOAD;
	const static string LSA_LIST;
	const static string LSA_HEADER;
	const static string LSA_AGE;
	const static string ADVERTISING_INSTANCE;
	const static string LSA_SEQUENCE_NUMBER;
	const static string LINK_STATE_LIST;
	const static string LINK_TYPE;
	const static string LINK_ID;
	const static string LINK_COST;
	const static string CONGESTION_STATUS;
	const static string ACK_PAYLOAD;
	const static string USER_PAYLOAD;
	const static string SSID;
	const static string PRI_KEY;
	const static string SEC_KEY;
	const static string USER_META;
	const static string LOAD_SHARE_HASH_KEY;
	const static string DATA;
private:
	static UlticomADC::ClusterRoutingMessage clu_route_msg;
	static UlticomADC::DraMessagePB dra_msg;
};

#endif /* _PROTO_BUF_UTILITY_H */
