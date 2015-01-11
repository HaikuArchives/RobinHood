// RH Log - A command line console for the Robin Hood Web Server
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

#include <stdio.h>
#include <stdlib.h>
#include "RHLogApp.h"

int main(  int argc, char **argv  )
{
	if( (argc > 1) && (strcmp( argv[1], "--help" ) == 0) )
	{
		printf( "Usage: rhlog [OPTIONS]\n" );
		printf( "  -m MASK\n" );
		printf( "MASK controls which log field-types are displayed.\n" );
		printf( "  o Open\n" );
		printf( "  c Close\n" );
		printf( "  r Request-Line\n" );
		printf( "  h Header\n" );
		printf( "  t Status-Line\n" );
		printf( "  s Sending\n" );
		printf( "  S Sent\n" );
		printf( "  x Exec\n" );
		printf( "  O Other\n" );
		return 0;
	}
	new RHLogApp();
	be_app->Run();
	delete( be_app );
	return 0;
}


