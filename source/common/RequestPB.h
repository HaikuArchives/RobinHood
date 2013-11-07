#ifndef REQUESTPB_H
#define REQUESTPB_H

#include "HTTPMessage.h"
#include <Path.h>
class VResList;
class HModuleRoster;

// *** fields ***
// --> : This field should be filled out before calling the module_roster
// HModuleRoster--> : This is filled out by the module_roster
// <-- : This field is returned by the handler module

struct RequestPB
{
	HTTPRequest	*request; // --> : the request message to be processed
	BPath *webDirectory; // --> : Absolute path name of the web directory
	VResList *vresources; // --> : Virtual Resource list for use by the module_roster
	HTTPRealmList *realms; // --> : Realms defined for the virtual host; used for authentication in the handler module
	brokenURI *brURI; // --> : The broken URL of the requested resource; brURI.path should be unescaped
	char **environ; // --> : The environment to be used; used by CGI, SSI, etc.
	bool authenticate; // --> : Do not enforce security if false; This is important when one handler calls another to get a resource
	int32 sn; // --> : The connection serial number; used for log entries
	void *cookie; // --> : Module defined field...
	status_t (*HandleRequest)(RequestPB *pb); // HModuleRoster--> : Function pointer to call module_roster::HandleRequest()
	int (*Logprintf)(const char *format, ... ); // HModuleRoster--> : Function pointer to log_printf()
	BPath *resourcePath; // HModuleRoster--> : The virtual path of the "real" resource found by the module_roster; may not be the same as brURI.path
	BList *moduleList; // HModuleRoster--> : A list of all loaded HModule objects 
	char *mimeType; // HModuleRoster--> : The MIME Type of the resource; It is the real type for "real" resources and the virtual type for "pure virtual" resources
	FieldList *extras; // HModuleRoster--> : Any extra fields defined by the invoking virtual resource; NULL if not a virtual resource
	bool closeConnection; // <-- : Should be set to true if a persistant connection should not be maintained; This MUST be "true" if a "Content-Length" header was not provided in the response
	void *reserved[4]; // Reserved for future use
};

#endif
