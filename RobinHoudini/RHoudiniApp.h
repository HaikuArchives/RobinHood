#include <Application.h>

#include "Defs.h"
#include "RHoudiniReplicant.h"

#ifndef _HOUDINI_APP_

class HoudiniApp : public BApplication
{
	public:
						HoudiniApp			(void);
						~HoudiniApp			(void);
	
		virtual void	ReadyToRun			(void);

	private:

};

#define _HOUDINI_APP_
#endif
