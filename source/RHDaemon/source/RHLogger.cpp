// Robin Hood Web Server for BeOS
// Copyright (C) 1999-2001
// The Robin Hood Development Team

// This program is free software; you can redistribute it and/or 
// modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 
// of the License, or (at your option) any later version. 

// This program is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
// GNU General Public License for more details. 

// You should have received a copy of the GNU General Public License 
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <Application.h>
#include <stdio.h>
#include <stdarg.h>
#include "RHLogger.h"
#include "RHMessages.h"

int log_printf( const char *format, ... )
{
	char		s[4096];
	int 		n;
	va_list 	argList;
	va_start( argList, format );
	n = vsprintf( s, format, argList );
	va_end( argList );
	
	BMessage	msg( MSG_LOG );
	msg.AddString( kMSG_LOG_STRING, s );
	be_app_messenger.SendMessage( &msg );
	return n;
}
