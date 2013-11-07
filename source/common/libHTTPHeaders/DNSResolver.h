/******************************************************************************
//
//	File:		DNSResolver.h
//
//	Description: Socket related utilities
//	Includes:
//		- A thread-safe DNS Resolver class
//		- can_read_datagram() function to check for data before committing to a recv() call
//	
//
//	Copyright 1998, Joe Kloss
//
*******************************************************************************/

#ifndef DNS_RESOLVER
#define DNS_RESOLVER

#include "LibHTTPBuild.h"

#include <Locker.h>

class DNSResolver
{
	public:
		DNSResolver( void );
		virtual ~DNSResolver( void );
	
	public:
		status_t ResolveName( const char *hostAddrStr, int32 *ip_addr );
	
	protected:
		BLocker			*resolveLock;
		char			lastHost[256];
		int32			last_ip;
};

extern _IMPEXP_LIBHTTP DNSResolver dns_resolver; // Global DNSResolver

#endif