#ifndef RHMESSAGES_H
#define RHMESSAGES_H
#include <Message.h>

#define RH_APP_SIG "application/x-vnd.KS-RH"

enum {
	MSG_LOG = 55000,
	MSG_ADD_LOG_SERVER
};

extern const char *kMSG_LOG_STRING;
extern const char *kMSG_LOG_MESSENGER;

#endif