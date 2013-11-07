#ifndef TCP_IO_H
#define TCP_IO_H

#include "LibHTTPBuild.h"

#include <DataIO.h>
#include <socket.h>
#include <netdb.h>

class Socket_IO : public BDataIO
{
	public:
		Socket_IO( int socket );
		virtual ~Socket_IO( void ); // Delete object to disconnect
		
		virtual ssize_t Read( void *buffer, size_t numBytes );
		virtual ssize_t Write( const void *buffer, size_t numBytes );
		
		status_t Connect( const char *IPname, unsigned short port ); // port is in native byte order
		int Connect( const struct sockaddr_in *remote_interface );
		int Bind( unsigned short port, int32 address = INADDR_ANY ); //address is in NBO
		int Listen( int acceptance_count );
		int Accept( struct sockaddr *client_interface, int *client_size ); // int Accept( struct sockaddr *client_interface, int *client_size );
		int Close( void );
		void SetCOD( bool flag ); // Set Close-On-Delete
		
		void SetBlocking( bool shouldBlock );
		int GetSocket( void );// Returns the socket token for this object's socket
		const char *GetPeerName( void );
	
	protected:
		int sock;
		bool closeOnDelete;
		char peername[16];
};

class TCP_IO : public Socket_IO
{
	public:
		TCP_IO( void ); // Create socket
		TCP_IO( int socket );  // Init with pre-existing socket
		
		int Accept( void ); // Don't care about remote interface!
		int Accept( struct sockaddr_in *client_interface ); 
};

class UDP_IO : public Socket_IO
{
	public:
		UDP_IO( int socket ); // Init with pre-existing socket
		UDP_IO( void ); // Create socket
		UDP_IO( unsigned short port ); // Create socket and bind to port
};

class TCP_Listener : public TCP_IO
{
	public:
		TCP_Listener( unsigned short port, int acceptance_count = 5 ); // port is in native byte order
		virtual ~TCP_Listener( void );
		
		unsigned short GetPort( void );
		int GetAcceptanceCount( void );
	
	protected:
		unsigned short port;
		int acceptance_count;
};
#endif