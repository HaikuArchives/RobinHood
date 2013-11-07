#ifndef RHMSG_FILTER_H
#define RHMSG_FILTER_H

#include <OS.h>

enum {
	M_OPEN = 1,
	M_CLOSE = 2,
	M_REQUEST_LINE = 4,
	M_HEADER = 8,
	M_STATUS_LINE = 16,
	M_SENDING = 32,
	M_SENT = 64,
	M_EXEC = 128,
	M_OTHER = 1073741824
};

enum {
	H_USER_AGENT = 1,
	H_ACCEPT = 2,
	H_HOST = 4,
	H_AUTHORIZATION = 8,
	H_ACCEPT_LANGUAGE = 16,
	H_ACCEPT_ENCODING = 32,
	H_RANGE = 64,
	H_VIA = 128,
	H_REFERER = 256,
	H_CONNECTION = 512,
	H_EXTENSION = 1024,
	H_FROM = 2048,
	H_UA = 4096,
	H_OTHER = 1073741824
};

int32 get_line_type( const char *typeName );
int32 get_header_type( const char *headerName );
bool filter_line_type( int32 filterMask, const char *line, int32 *type = NULL, const char **textPtr = NULL );
bool filter_header_type( int32 filterMask, const char *header, int32 *type = NULL );

#endif
