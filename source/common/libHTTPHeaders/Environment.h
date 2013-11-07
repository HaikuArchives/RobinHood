#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "LibHTTPBuild.h"
#include <SupportDefs.h>

class Environment
{
	public:
		Environment( void );
		Environment( char **envir );
		virtual ~Environment( void );
		
		const char *GetEnv( const char *name, int *index = NULL );
		const char *GetEnv( int index );
		void PutEnv( const char *string );
		void PutEnv( const char *name, const char *value );
		
		char **GetEnvironment( void );
		void MakeEmpty();
		int CountVariables();
		void CopyEnv( char **envir );
	
	protected:
		void InitEnvironment( void );
		void AllocateBlock( void );
		
	protected:
		char		**env;
		int			next, totalVars;
		int			pointerSize, blocks;
};

#endif