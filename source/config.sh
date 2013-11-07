#!/bin/sh

# ---------------------------------

config_bone ()
{
	cd /boot/apps/RobinHood/lib
	ln -s /boot/beos/system/lib/libbhttp.so
	mv libbhttp.so libHTTP.so
}

create_link ()
{
	cd ~/Desktop
	ln -s /boot/apps/RobinHood/RHConsole
	mv RHConsole "Robin Hood"
}

finish ()
{
	exit $1
}

# ---------------------------------

if [ -f /boot/beos/system/lib/libbhttp.so ]
then
	config_bone
	alert --info "Robin Hood Web Server
has been configured for BONE."
else
	alert --info "Robin Hood Web Server
has been configured for BeOS R5 networking."
fi

# ---------------------------------

alert --info "Do you want me to place a link to the
Robin Hood Console on your desktop?
" Yes No > /dev/null && ASKICON=1

if [ -z "$ASKICON" ]
then
	finish 0
else
	create_link
fi

# ---------------------------------

finish 0
