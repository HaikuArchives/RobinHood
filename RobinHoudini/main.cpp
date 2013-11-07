// Robin Houdini
// © Jonas Sundström, www.kirilla.com

#define DEBUG 0
#include <Debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Application.h>
#include <Roster.h>
#include <Deskbar.h>

#include "RHoudiniApp.h"
#include "RHoudiniReplicant.h"

extern "C" _EXPORT BView* instantiate_deskbar_item(void);

int main()
{
	new HoudiniApp ();
	be_app	-> Run();
	delete	be_app;
	return (0);
}

BView *	instantiate_deskbar_item	(void)
{
	return new HoudiniReplicant();
}

