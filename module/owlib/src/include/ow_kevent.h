/*
    OWFS -- One-Wire filesystem
    OWHTTPD -- One-Wire Web Server
    Written 2003 Paul H Alfille
	email: paul.alfille@gmail.com
	Released under the GPL
	See the header file: ow.h for full attribution
	1wire/iButton system from Dallas Semiconductor
*/

#ifndef OW_KEVENT_H
#define OW_KEVENT_H

#ifndef OWFS_CONFIG_H
#error Please make sure owfs_config.h is included *before* this header file
#endif

#ifdef WE_HAVE_KEVENT
// kevent based monitor for configuration file changes
// usually OSX and BSD systems

#include <sys/event.h>
void Config_Monitor_Watch( void ) ;
void Config_Monitor_Add( const char * file ) ;

#endif /* WE_HAVE_KEVENT */

#endif							/* OW_KEVENT_H */
