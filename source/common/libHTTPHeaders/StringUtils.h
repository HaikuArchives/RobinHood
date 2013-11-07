#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "LibHTTPBuild.h"

#include <OS.h>

_IMPEXP_LIBHTTP bool match_pattern( const char *pattern, const char *test );
_IMPEXP_LIBHTTP const char *get_next_field( const char *fieldStr, char *fieldName, char *fieldValue );
_IMPEXP_LIBHTTP size_t uri_esc_str( char *dst, const char *src, size_t bufSize, bool usePlus = false, const char *protectedChars = "" );
_IMPEXP_LIBHTTP size_t uri_unescape_str( char *dst, const char *src, size_t bufSize );
_IMPEXP_LIBHTTP const char *get_next_token( char *dest, const char *source, size_t size, const char *delim = " ", const char *quote = "" );
_IMPEXP_LIBHTTP char *strxcpy (char *to, const char *from, size_t size);
_IMPEXP_LIBHTTP size_t decode_base64( char *dst, const char *src, size_t length );

#endif