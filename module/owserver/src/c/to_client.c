/*
$Id$
    OW_HTML -- OWFS used for the web
    OW -- One-Wire filesystem

    Written 2004 Paul H Alfille

 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* owserver -- responds to requests over a network socket, and processes them on the 1-wire bus/
         Basic idea: control the 1-wire bus and answer queries over a network socket
         Clients can be owperl, owfs, owhttpd, etc...
         Clients can be local or remote
                 Eventually will also allow bounce servers.

         syntax:
                 owserver
                 -u (usb)
                 -d /dev/ttyS1 (serial)
                 -p tcp port
                 e.g. 3001 or 10.183.180.101:3001 or /tmp/1wire
*/

#include "owserver.h"

/* Send fully configured message back to client.
   data is optional and length depends on "payload"
 */
int ToClient(int fd, struct client_msg *cm, char *data)
{
	// note data should be (const char *) but iovec complains about const arguments 
	int nio = 1;
	int ret;
	struct iovec io[] = {
		{cm, sizeof(struct client_msg),},
		{data, cm->payload,},
	};

	/* If payload==0, no extra data
	   If payload <0, flag to show a delay message, again no data
	 */
	if (data && cm->payload > 0) {
		++nio;
	}
	LEVEL_DEBUG("ToClient payload=%d size=%d, ret=%d, sg=%X offset=%d \n",
				cm->payload, cm->size, cm->ret, cm->sg, cm->offset);
	//printf(">%.4d|%.4d\n",cm->ret,cm->payload);
	//printf("Scale=%s\n", TemperatureScaleName(SGTemperatureScale(cm->sg)));

	cm->payload = htonl(cm->payload);
	cm->size = htonl(cm->size);
	cm->offset = htonl(cm->offset);
	cm->ret = htonl(cm->ret);
	cm->sg = htonl(cm->sg);

	ret = writev(fd, io, nio) != (ssize_t) (io[0].iov_len + io[1].iov_len);

	cm->payload = ntohl(cm->payload);
	cm->size = ntohl(cm->size);
	cm->offset = ntohl(cm->offset);
	cm->ret = ntohl(cm->ret);
	cm->sg = ntohl(cm->sg);

	return ret;
}