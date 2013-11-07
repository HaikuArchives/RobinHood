#ifndef LIB_HTTP_BUILD
#define LIB_HTTP_BUILD

#ifdef _BUILDING_LIB_HTTP
#define	_IMPEXP_LIBHTTP		__declspec(dllexport)
#else
#define	_IMPEXP_LIBHTTP		__declspec(dllimport)
#endif

class _IMPEXP_LIBHTTP BufferedIO;
class _IMPEXP_LIBHTTP DataIOPump;
class _IMPEXP_LIBHTTP DNSResolver;
class _IMPEXP_LIBHTTP FieldList;
class _IMPEXP_LIBHTTP FileIO;
class _IMPEXP_LIBHTTP DesIO;
class _IMPEXP_LIBHTTP HTMLError;
class _IMPEXP_LIBHTTP HTTPFileServer;
class _IMPEXP_LIBHTTP HTTPHandler;
class _IMPEXP_LIBHTTP HTTPListener;
class _IMPEXP_LIBHTTP HTTPConnection;
class _IMPEXP_LIBHTTP HTTPMessage;
class _IMPEXP_LIBHTTP HTTPResponse;
class _IMPEXP_LIBHTTP HTTPRequest;
struct _IMPEXP_LIBHTTP brokenURI;
class _IMPEXP_LIBHTTP Socket_IO;
class _IMPEXP_LIBHTTP TCP_IO;
class _IMPEXP_LIBHTTP UDP_IO;
class _IMPEXP_LIBHTTP TCP_Listener;
class _IMPEXP_LIBHTTP HTTP_Io;
class _IMPEXP_LIBHTTP HTTPRealm;
class _IMPEXP_LIBHTTP HTTPRealmList;
class _IMPEXP_LIBHTTP Environment;
class _IMPEXP_LIBHTTP ByteRangeSet;

#endif