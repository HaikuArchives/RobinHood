#ifndef ICON_CACHE_H
#define ICON_CACHE_H

#include "List.h"
#include "Bitmap.h"
#include "Locker.h"
#include "String.h"

struct iconItem
{
	BString		name;
	char		*buffer;
	int32		size;
};

class IconCache : public BList, public BLocker
{
	public:
		IconCache( int32 maxItems );
		virtual ~IconCache( void );
		
		void AddIcon( const char *name, const char *buffer, int32 size );
		iconItem *FindIcon( const char *name );
	
	protected:
		int32		maxItems;
		
};

#endif
