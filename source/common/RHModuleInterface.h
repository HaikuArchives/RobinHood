#ifndef RHMODULE_INTERFACE_H
#define RHMODULE_INTERFACE_H

#include "RequestPB.h"
#include "HTTPUtils.h"

extern "C" __declspec(dllexport) status_t handle_request( RequestPB *pb );
extern "C" __declspec(dllexport) bool can_handle_resource( const char *mimeType, http_method method, int32 *priority );

#endif
