// Robin Hood Web Server for BeOS
// Copyright (C) 1999-2001
// The Robin Hood Development Team

// This program is free software; you can redistribute it and/or 
// modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 
// of the License, or (at your option) any later version. 

// This program is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
// GNU General Public License for more details. 

// You should have received a copy of the GNU General Public License 
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <stdlib.h>
#include <stdio.h>
#include "VHosts.h"
#include "string.h"

VHost::VHost( void )
{
	realms.SetDefaultRealmName( "Sherwood" );
}

VHost::~VHost( void )
{
	
}

void VHost::SetHost( const char *host )
{
	this->host.SetTo( host );
}

void VHost::SetWebroot( const char *webroot )
{	
	this->webroot.SetTo( webroot );
}

void VHost::SetIndex( const char *index )
{	
	this->index.SetTo( index );
}

const char *VHost::GetHost( void )
{
	return host.String();
}

const char *VHost::GetWebroot( void )
{
	return webroot.String();
}

const char *VHost::GetIndex( void )
{
	return index.String();
}

VHostsList::VHostsList( void )
{
	
}

VHostsList::~VHostsList( void )
{
	void	*item;
	for( int32 i=0; (item = ItemAt(i)); i++ )
		delete item;
}

void VHostsList::AddVHost( VHost *host )
{
	AddItem( host );
}

VHost *VHostsList::FindVHost( const char *host )
{
	VHost	*vhost;
	
	for( int32 i=0; (vhost = (VHost *)ItemAt(i)); i++ )
	{
		if( strcasecmp( host, vhost->GetHost() ) == 0 )
			return vhost;
	}
	return NULL;
}

const char *VHostsList::FindWebroot( const char *host )
{
	VHost	*vhost;
	
	if( (vhost = FindVHost( host )) )
		return vhost->GetWebroot();
	return NULL;
}

const char *VHostsList::FindIndex( const char *host )
{
	VHost	*vhost;
	
	if( (vhost = FindVHost( host )) )
		return vhost->GetIndex();
	return NULL;
}
