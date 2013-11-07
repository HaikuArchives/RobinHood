#ifndef RHMODULE_INIT_H
#define RHMODULE_INIT_H

#include "RequestPB.h"
#include "HTTPUtils.h"

extern "C" __declspec(dllexport) status_t init_module( void );
extern "C" __declspec(dllexport) void shutdown_module( void );

#endif
