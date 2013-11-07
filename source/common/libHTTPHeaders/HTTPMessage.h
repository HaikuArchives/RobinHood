#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include "LibHTTPBuild.h"

#include "FieldList.h"
#include "HTTPUtils.h"

class HTTPMessage
{
	public:
		HTTPMessage( void );
		virtual ~HTTPMessage( void );
		
		virtual void InitMessage( void );
		void SetStartLine( const char *start_line );
		const char *GetStartLine( void );
		
		void SetVersion( const char *version );
		const char *GetVersion( void );
		
		void AddHeader( const char *header );
		void AddHeader( const char *fieldName, const char *fieldValue );
		bool RemoveHeader( char *headerPtr );
		bool RemoveHeaderByName( const char *fieldName );
		
		const char *FindHeader( const char *fieldName, char *fieldValue=NULL, size_t n=0 );
		const char *HeaderAt( int32 index );
		int32 CountHeaders( void );
		void FreeHeaders( void );
		
		void SetContentLength( int64 length );
		int64 GetContentLength( void );
		
		void SetMessageBody( BDataIO *body );
		BDataIO *GetMessageBody( void );
		status_t DeleteMessageBody( void );
		
		void SetBodyBuffering( bool buffered );
		
		virtual int32 SendMessage( BDataIO *io, bool simple = false );
		virtual int32 ReceiveMessage( BDataIO *io ) = 0;
		
		virtual int32 SendHeaders( BDataIO *io, bool simple = false );
		virtual int32 SendBody( BDataIO *io );
		virtual int32 ReceiveStartLine( BDataIO *io );
		virtual int32 ReceiveHeaders( BDataIO *io );
		virtual int32 ReceiveBody( BDataIO *io );
		
	protected:
		char			start_line[4096];
		char			version[16];
		bool			buffered;
		
		FieldList		headers;
		int64			contentLength;
		BDataIO			*body;
};

class HTTPResponse : public HTTPMessage
{
	public:
		HTTPResponse( void );
		virtual ~HTTPResponse( void );
		
		void SetStatusLine( const char *status );
		void SetStatusLine( int32 statusCode );
		const char *GetStatusLine( void ) { return GetStartLine(); };
		void SetHTMLMessage( int32 statusCode, const char *msg = NULL );
		
		int32 GetStatusCode( void );
		
		virtual int32 ReceiveMessage( BDataIO *io );
		virtual int32 SendMessage( BDataIO *io, bool simple = false );
		
	protected:
		int32			statusCode;
		BDataIO			*internalMessage;
};

class HTTPRequest : public HTTPMessage
{
	public:
		HTTPRequest( void );
		virtual ~HTTPRequest( void );
		
		void SetRequestLine( const char *requestLine );
		void SetRequestLine( const char *method, const char *uri );
		void SetRequestLine( http_method method, brokenURI *uri );
		const char *GetRequestLine( void ) { return GetStartLine(); };
		void GetRequestLine( char *method, char *uri, char *version );
		
		http_method GetMethod( void );
		void ParseURI( brokenURI *uri );
		
		void SetReplyIO( BDataIO *replyIO );
		BDataIO *GetReplyIO( void );
		
		void SetPort( unsigned short port );
		unsigned short GetPort( void );
		void SetRemoteHost( const char *remoteHost );
		const char *GetRemoteHost( void );
		
		virtual int32 ReceiveMessage( BDataIO *io );
		virtual int32 SendReply( HTTPResponse *response );
		
	protected:
		http_method		method;
		BDataIO			*replyIO;
		short			port;
		char			remoteHost[64];
};

#endif