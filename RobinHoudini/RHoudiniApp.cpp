// HoudiniApp.cpp
// (C) Jonas Sundström @ Kirilla.com

#define DEBUG 0
#include <Debug.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <Application.h>
#include <Deskbar.h>
#include <Entry.h>
#include <Roster.h>

#include "Defs.h"
#include "RHoudiniApp.h"

HoudiniApp::HoudiniApp	(void)
 : BApplication	(HOUDINI_APP_SIG)
{
	// void
}

HoudiniApp::~HoudiniApp	(void)
{
	// void
}

void
HoudiniApp::ReadyToRun	(void)
{
	BDeskbar deskbar;

	if (! deskbar.HasItem(HOUDINI_REPLICANT))
	{
		entry_ref	ref;
		be_roster->FindApp(HOUDINI_APP_SIG, & ref);
		deskbar.AddItem(& ref);
	}
	else
		deskbar.RemoveItem(HOUDINI_REPLICANT);
	
	
	be_app->PostMessage(B_QUIT_REQUESTED);
}
