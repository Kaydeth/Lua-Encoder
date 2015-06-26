/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	SocketManager.h
 * \version	$Revision:/main/1 $
 * \date	$Date:3-Sep-2013 10:14:51 $
 * \product	DSC test tools
 * \brief	keeps track of all open sockets in order to closed them all
 * 		cleanly when a script exits on error.
 */

#ifndef _SOCKET_MANAGER_H_
#define _SOCKET_MANAGER_H_

#include <list>

using namespace std;

class SocketManager
{
public:
	SocketManager();
	virtual ~SocketManager();

	void addSocket(int fd);
	void removeSocket(int fd);

private:
	list<int> socketFileDescriptors;
};

#endif /* _SOCKET_MANAGER_H_ */
