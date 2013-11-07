#define DEBUG 0
#include <Debug.h>

#include <stdio.h>
#include <unistd.h>

#include <Roster.h>
#include <Alert.h>
#include <PopUpMenu.h>
#include <Menu.h>
#include <MenuItem.h>
#include <String.h>
#include <Path.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <Directory.h>

#include <View.h>
#include <Deskbar.h>
#include <Bitmap.h>
#include <AppFileInfo.h>

#include "Defs.h"
#include "RHoudiniReplicant.h"

const rgb_color COLOR_FOREGROUND = { 0, 0, 0 };
const rgb_color COLOR_BACKGROUND = { 255, 203, 102 };

HoudiniReplicant::HoudiniReplicant	(void)
	: BView(BRect(0, 0, 15, 15), 0,0,0)
{
	PRINT(("HoudiniReplicant::HoudiniReplicant()\n"));

}

HoudiniReplicant::~HoudiniReplicant	(void)	
{
	PRINT(("HoudiniReplicant::~HoudiniReplicant()\n"));
}

HoudiniReplicant::HoudiniReplicant(BMessage * a_message)
	: BView(BRect(0,0,15,15), HOUDINI_REPLICANT, B_FOLLOW_NONE, B_WILL_DRAW),
	m_icon			(NULL),
	m_popup_menu	(NULL),
	m_start_item	(NULL),
	m_stop_item		(NULL),
	m_kill_item		(NULL),
	m_on_icon		(NULL),
	m_off_icon		(NULL),
	m_is_running	(false)
{
	PRINT(("HoudiniReplicant::HoudiniReplicant(BMessage * a_message)\n"));

	SetViewColor(B_TRANSPARENT_COLOR);

	BRect	small_rect	(0,0,15,15);
	m_on_icon	=	new	BBitmap(small_rect, B_COLOR_8_BIT);	
	m_off_icon	=	new	BBitmap(small_rect, B_COLOR_8_BIT);
	
	m_on_icon->		SetBits(& m_on_icon_bits, 256, 0, B_COLOR_8_BIT);
	m_off_icon->	SetBits(& m_off_icon_bits, 256, 0, B_COLOR_8_BIT);

	CreatePopup();
}

__declspec(dllexport) HoudiniReplicant * 
HoudiniReplicant::Instantiate	(BMessage * archive)
{
	PRINT(("__declspec(dllexport) HoudiniReplicant * "
			"HoudiniReplicant::Instantiate	(BMessage * archive)\n"));

	if (! validate_instantiation(archive, "HoudiniReplicant")) 
	{
		PRINT(("! validate_instantiation()\n"));
		return 0;
	}
	return new HoudiniReplicant(archive);
}

status_t 
HoudiniReplicant::Archive(BMessage * archive, bool deep) const
{
	PRINT(("status_t HoudiniReplicant::Archive(BMessage * archive, bool deep) const\n"));

	// Tell the overridden BView to archive itself.
	BView::Archive(archive, deep);
  
	// Store our signature and the name of our class in the archive.
	archive->AddString("add_on", HOUDINI_APP_SIG);
	archive->AddString("class", "HoudiniReplicant");

	return B_OK;
}

void 
HoudiniReplicant::AttachedToWindow ()
{
	be_roster->StartWatching (BMessenger(this), B_REQUEST_LAUNCHED | B_REQUEST_QUIT);
}

void 
HoudiniReplicant::Draw(BRect updateRect)
{
	BView::Draw(Bounds());

	MovePenTo(0,0);
	SetDrawingMode(B_OP_OVER);
	
	if (m_is_running)
		DrawBitmap(m_on_icon);
	else
		DrawBitmap(m_off_icon);
}

void
HoudiniReplicant::MouseDown(BPoint point) 
{
	ConvertToScreen(&point); 
	m_popup_menu->SetTargetForItems(this);
	m_popup_menu->Go(point, true, false, true); 
}

void
HoudiniReplicant::MessageReceived	(BMessage * a_message)
{
	switch(a_message->what) 
	{
		case 'strt':
		{
			be_roster->Launch(RH_DAEMON_APP_SIG);
			break;
		}
		
		case 'stop':
		{
			status_t	status	=	B_OK;
			BMessenger	messenger	(RH_DAEMON_APP_SIG, -1, & status);
			messenger.SendMessage (B_QUIT_REQUESTED);
			break;
		}
	
		case 'kill':
		{
			team_id		team	=	be_roster->	TeamFor (RH_DAEMON_APP_SIG);
			if (team != B_ERROR)
				kill_team (team);
			break;
		}
		
		case B_SOME_APP_LAUNCHED:
		{
			BString sig;
			sig = a_message->	FindString ("be:signature");
			
			if (sig == RH_DAEMON_APP_SIG)
			{
				m_start_item->	SetEnabled (false);
				m_stop_item->	SetEnabled (true);
				m_kill_item->	SetEnabled (true);
				m_is_running	=	true;
				Invalidate();
			}
			break;
		}

		case B_SOME_APP_QUIT:
		{
			BString sig;
			sig = a_message->	FindString ("be:signature");
			
			if (sig == RH_DAEMON_APP_SIG)
			{
				m_start_item->	SetEnabled (true);
				m_stop_item->	SetEnabled (false);
				m_kill_item->	SetEnabled (false);
				m_is_running	=	false;
				Invalidate();
			}
			break;
		}
			
		case B_QUIT_REQUESTED:
		{
			BDeskbar deskbar;
			deskbar.RemoveItem(HOUDINI_REPLICANT);   // bad, leaves the addon loaded
			break;
		}
		
		default: 
		{
			BView::MessageReceived(a_message);
			break;
		}
	}
}

void
HoudiniReplicant::CreatePopup	(void) 
{
	m_popup_menu = new BPopUpMenu("TuneTrackerPopup", false, false);
	BMenuItem * item; 

	m_start_item =	new BMenuItem("Start RobinHood", new BMessage('strt'));
	m_stop_item =	new BMenuItem("Stop RobinHood", new BMessage('stop'));
	m_kill_item =	new BMenuItem("Kill RobinHood", new BMessage('kill'));
	
	m_popup_menu->	AddItem(m_start_item);
	m_popup_menu->	AddItem(m_stop_item);
	m_popup_menu->	AddItem(m_kill_item);
	
	if (be_roster->IsRunning(RH_DAEMON_APP_SIG))
	{
		m_start_item->	SetEnabled(false);
		m_stop_item->	SetEnabled(true);
		m_kill_item->	SetEnabled(true);
		m_is_running	=	true;
	}
	else
	{
		m_start_item->	SetEnabled(true);
		m_stop_item->	SetEnabled(false);
		m_kill_item->	SetEnabled(false);
		m_is_running	=	false;
	}
		
	m_popup_menu->AddSeparatorItem();

	item = new BMenuItem("Remove icon", new BMessage(B_QUIT_REQUESTED));
	m_popup_menu->AddItem(item);

	m_popup_menu->SetTargetForItems(this);
}

