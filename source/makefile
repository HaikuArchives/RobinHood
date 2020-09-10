SHELL=/bin/sh

dirs := \
	libHTTP \
	RHConsole \
	RHLog \
	RHDaemon \
	modules

default install clean:
	@export LIBHTTP_L="$(shell realpath ./libHTTP)/objects.*/libHTTP.a"; \
	for dir in $(dirs); do \
		if [ -d "$$dir" ]; then \
			INSTALL_DIR=../server $(MAKE) -C $$dir $@ || exit 1; \
		fi \
	done

