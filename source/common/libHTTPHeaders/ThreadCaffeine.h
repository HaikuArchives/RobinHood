#ifndef THREAD_CAFFEINE_H
#define THREAD_CAFFEINE_H

#include "LibHTTPBuild.h"

#include <OS.h>
#include <List.h>
#include <Locker.h>

class ThreadCaffeine
{
	public:
		ThreadCaffeine( bigtime_t sleep = 10000000 ); // Default is 10 seconds
		virtual ~ThreadCaffeine( void );
		
		status_t SetTeam( team_id team );
		
		thread_id Run( void );
		status_t Quit( void );
		status_t AddThread( const char *thread_name );
		status_t AddThread( thread_id thread, bigtime_t timeout );
		status_t RemoveThread( thread_id thread );
		void WaitForThreads( bool wake );
	
	protected:
		static int32 thread_entry( void *arg );
		int32 MainLoop( void );
		
		bool		running;
		thread_id	main_thread;
		team_id		team;
		BList		*threadList;
		BLocker		*lLock;
		bigtime_t 	sleep;
};

#endif