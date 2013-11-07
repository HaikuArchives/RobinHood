#ifndef VHOSTS_H
#define VHOSTS_H

#include "FieldList.h"
#include "VResource.h"
#include "HTTPRealm.h"
#include <String.h>

class VHost
{
	public:
		VHost( void );
		virtual ~VHost( void );
		
		void SetHost( const char *host );
		void SetWebroot( const char *webroot );
		void SetIndex( const char *index );
		
		const char *GetHost( void );
		const char *GetWebroot( void );
		const char *GetIndex( void );
	
	public:
		VResList		vresources; // Virtual Resources
		HTTPRealmList	realms; // Realms
	
	protected:
		BString			host;
		BString			webroot;
		BString			index;
};

class VHostsList : public BList
{
	public:
		VHostsList( void );
		virtual ~VHostsList( void );
		
		void AddVHost( VHost *host );
		VHost *FindVHost( const char *host );
		const char *FindWebroot( const char *host );
		const char *FindIndex( const char *host );
};

#endif