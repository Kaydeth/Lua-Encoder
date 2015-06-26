/******************************************************************************

                                  Ulticom, Inc.
                      Copyright 2010 All Rights Reserved.

        These computer program listings and specifications, herein,
        are the property of Ulticom, Inc. and shall not be
        reproduced or copied or used in whole or in part as the basis
        for manufacture or sale of items without written permission.


******************************************************************************/

/*! \file	SocketManager.cpp
 * \version	$Revision:/main/2 $
 * \date	$Date:19-Jun-2015 10:22:29 $
 * \product	DSC test tools
 * \brief	keeps track of all open sockets in order to closed them all
 * 		cleanly when a script exits on error.
 */

#include "SocketManager.h"
#include <sys/socket.h>
#include <cstdio>
#include <unistd.h>

using namespace std;

SocketManager::SocketManager()
{
}

SocketManager::~SocketManager()
{
	list<int>::iterator iter;

	for(iter = socketFileDescriptors.begin();
		iter != socketFileDescriptors.end(); iter++)
	{
		printf("cleaning up socket %d\n", *iter);
		shutdown(*iter, SHUT_RDWR);
		close(*iter);
	}

	socketFileDescriptors.clear();
}

void SocketManager::addSocket(int fd)
{
	socketFileDescriptors.push_back(fd);
}

void SocketManager::removeSocket(int fd)
{
	for(list<int>::iterator it = socketFileDescriptors.begin();
		it != socketFileDescriptors.end(); ++it)
	{
		if(*it == fd)
		{
			socketFileDescriptors.erase(it);
			break;
		}
	}
}
