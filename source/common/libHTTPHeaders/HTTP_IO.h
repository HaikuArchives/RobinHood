#ifndef HTTP_IO
#define HTTP_IO

#include "LibHTTPBuild.h"

#include <DataIO.h>

class TCP_IO;
class HTTPRequest;
class HTTPResponse;
struct brokenURI;

class HTTP_Io : public BPositionIO
{
	public:
		HTTP_Io( void );
		HTTP_Io( const char *URL, uint32 openMode, bool head = true );
		virtual ~HTTP_Io( void );
		
		status_t InitCheck( void ) { return initStatus; };
		bool Exists( void );
		bool IsReadable( void );
		bool IsWritable( void );
		
		virtual ssize_t Read( void *buffer, size_t numBytes );
		virtual ssize_t ReadAt( off_t position, void *buffer, size_t numBytes );
		virtual ssize_t Write( const void *buffer, size_t numBytes );
		virtual ssize_t WriteAt( off_t position, const void *buffer, size_t numBytes );
		
		virtual off_t Seek( off_t position, uint32 mode );
		virtual off_t Position( void ) const;
		
		status_t SetTo( const char *URL, uint32 openMode, bool head = true );
		void Unset( void );
		
		const char *GetURL( void );
		status_t GetType( char *type );
		status_t GetStatusCode( int32 *statusCode );
		status_t GetStatusLine( char *statusLine, int32 size );
		status_t GetSize( off_t *size );
		const char *FindHeader( const char *fieldName, char *fieldValue=NULL, size_t n=0 );
		const char *HeaderAt( int32 index );
		int32 CountHeaders( void );
		
		void UseProxy( const char *proxyHost, uint16 proxyPort );
		virtual void CloseConnection( void );
		
	protected:
		virtual void Init( void );
		virtual void InitHTTP( void );
		virtual void InitConnection( void );
		
		status_t CheckConnection( void );
		status_t DoHEAD( void );
		status_t MakeRequest( const char *method, const char *range = NULL );
		
		// User hooks
		virtual void AddHeaders( void );
		
	protected:
		status_t		initStatus;
		char			*URL;
		uint16			port;
		
		// HTTP Stuff
		HTTPRequest		*request;
		HTTPResponse	*response;
		brokenURI		*brURI;
		
		status_t		statusCode;
		off_t			position;
		uint32			seekMode;			
		int32			contentLength;
		uint32 			accessMode;
		char			contentType[256];
		
		// Connection Stuff
		TCP_IO			*connection;
		char			lastHost[64];
		uint16			lastPort;
		
		bool			isGet;
		int32			requestSN;
		bool			needsContinue;
		int32			contentRemaining;
		
		// Proxy Stuff
		char			proxyHost[64];
		uint16			proxyPort;
};

#endif