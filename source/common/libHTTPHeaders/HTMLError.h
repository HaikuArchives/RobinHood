#ifndef HTML_ERROR_H
#define HTML_ERROR_H

#include "LibHTTPBuild.h"

#include <DataIO.h>

class HTMLError : public BMallocIO
{
	public:
		HTMLError( const char *message );
};

#endif
