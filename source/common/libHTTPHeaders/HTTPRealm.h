#ifndef HTTPREALM_H
#define HTTPREALM_H

#include "LibHTTPBuild.h"

#include <List.h>
#include <String.h>
#include "HTTPMessage.h"

class HTTPRealm : public BList
{
	public:
		HTTPRealm( void );
		virtual ~HTTPRealm( void );
		
		void SetName( const char *name );
		void SetUser( const char *user );
		void SetPasswd( const char *passwd );
		void AddPattern( const char *pattern );
		
		bool Match( const char *path );
		
		const char *GetName( void );
		const char *GetUser( void );
		const char *GetPasswd( void );
	
	protected:
		BString		name;
		BString		user;
		BString		passwd;
};

class HTTPRealmList : public BList
{
	public:
		HTTPRealmList( void );
		virtual ~HTTPRealmList( void );
		
		void SetDefaultRealmName( const char *name );
		void AddRealm( HTTPRealm *realm );
		HTTPRealm *MatchRealm( const char *path );
		HTTPRealm *FindRealm( const char *name );
		bool Authenticate( HTTPRequest *request, HTTPResponse *response, const char *webPath, const char *absPath = NULL, mode_t bitMask = 0 );
	
	protected:
		BString		defaultRealm;
};

#endif
