#ifndef FIELD_LIST_H
#define FIELD_LIST_H

#include "LibHTTPBuild.h"

#include <List.h>

class FieldList : public BList
{
	public:
		FieldList(int32 count = 20) : BList(count) {  };
		virtual ~FieldList(void);
		
		bool AddField(const char *field, int32 size=-1);
		bool AddField(const char *fieldName, const char *fieldValue );
		bool RemoveField( char *fieldPtr );
		bool RemoveFieldByName( const char *fieldName );
		const char *FindField(const char *fieldName, char *fieldValue = NULL, size_t n = 0);
		void MakeEmpty(void);
};

#endif