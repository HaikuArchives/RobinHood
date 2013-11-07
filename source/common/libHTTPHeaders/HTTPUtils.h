#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include "LibHTTPBuild.h"

#include <OS.h>
#include <Locker.h>

// Used to protect the struct *tm returned by standard time functions
// from other threads.
// struct tm * localtime (const time_t *time)
// struct tm * gmtime (const time_t *time)

extern _IMPEXP_LIBHTTP BLocker brTimeLock;

_IMPEXP_LIBHTTP const char *get_status_str( int32 status_code );

enum http_method { METHOD_POST, METHOD_GET, METHOD_HEAD, METHOD_OPTIONS, 
	METHOD_PUT, METHOD_DELETE, METHOD_TRACE, METHOD_UNKNOWN };
	
_IMPEXP_LIBHTTP http_method http_find_method( const char *method );
_IMPEXP_LIBHTTP const char *http_find_method( http_method method );

enum URI_Type { absolueURI, relativeURI };
enum Path_Type { abs_path, rel_path, empty_path };

struct brokenURI
{
	URI_Type 	URIType;
	
	char		scheme[16];
	char		host[64];
	char		port[8];
	
	Path_Type	PathType;
	
	char		path[2048];
	char		params[2048];
	char		query[2048];
};

_IMPEXP_LIBHTTP void parse_URI( const char *URI, brokenURI *brURI );
_IMPEXP_LIBHTTP char *URI_to_string( brokenURI *brURI, char *s, int32 size, bool full=true );
_IMPEXP_LIBHTTP bool basic_authenticate( const char *basic_cookie, const char *uid, const char *pass, bool encrypted=true );

_IMPEXP_LIBHTTP char *http_to_cgi_header( char *header );

extern _IMPEXP_LIBHTTP const char *kHEAD_ALLOW;
extern _IMPEXP_LIBHTTP const char *kHEAD_AUTHORIZATION;
extern _IMPEXP_LIBHTTP const char *kHEAD_ENCODING;
extern _IMPEXP_LIBHTTP const char *kHEAD_ACCEPT_ENCODING;
extern _IMPEXP_LIBHTTP const char *kHEAD_CONNECTION;
extern _IMPEXP_LIBHTTP const char *kHEAD_LENGTH;
extern _IMPEXP_LIBHTTP const char *kHEAD_TYPE;
extern _IMPEXP_LIBHTTP const char *kHEAD_DATE;
extern _IMPEXP_LIBHTTP const char *kHEAD_EXPIRES;
extern _IMPEXP_LIBHTTP const char *kHEAD_FROM;
extern _IMPEXP_LIBHTTP const char *kHEAD_IF_MODIFIED;
extern _IMPEXP_LIBHTTP const char *kHEAD_IF_UNMODIFIED;
extern _IMPEXP_LIBHTTP const char *kHEAD_LAST_MODIFIED;
extern _IMPEXP_LIBHTTP const char *kHEAD_LOCATION;
extern _IMPEXP_LIBHTTP const char *kHEAD_PRAGMA;
extern _IMPEXP_LIBHTTP const char *kHEAD_REFRESHER;
extern _IMPEXP_LIBHTTP const char *kHEAD_SERVER;
extern _IMPEXP_LIBHTTP const char *kHEAD_HOST;
extern _IMPEXP_LIBHTTP const char *kHEAD_AGENT;
extern _IMPEXP_LIBHTTP const char *kHEAD_AUTHENTICATE;
extern _IMPEXP_LIBHTTP const char *kHEAD_CONTENT_RANGE;
extern _IMPEXP_LIBHTTP const char *kHEAD_RANGE;
extern _IMPEXP_LIBHTTP const char *kCRLF;
extern _IMPEXP_LIBHTTP const char *kHTTP_DATE;
#endif
