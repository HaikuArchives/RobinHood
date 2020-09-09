SHELL=/bin/sh

dirs := \
	RHConsole \
	RHLog \
	RHDaemon \
	modules

ifneq ($(shell getarch), $(shell getarch -p))
LIBHTTP_H ?= \
	$(shell findpaths -e -a `getarch` -c " " B_FIND_PATH_HEADERS_DIRECTORY libHTTP)
else
LIBHTTP_H ?= \
	$(shell findpaths -e -c " " B_FIND_PATH_HEADERS_DIRECTORY libHTTP)
endif

default install clean:
	@export LIBHTTP_H="$(LIBHTTP_H)"; \
	for dir in $(dirs); do \
		if [ -d "$$dir" ]; then \
			INSTALL_DIR=../server $(MAKE) -C $$dir $@ || exit 1; \
		fi \
	done

