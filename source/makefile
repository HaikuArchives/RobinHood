SHELL=/bin/sh

dirs := \
	RHConsole \
	RHLog \
	RHDaemon \
	modules

default install clean:
	@for dir in $(dirs); do \
		if [ -d "$$dir" ]; then \
			INSTALL_DIR=../server $(MAKE) -C $$dir $@; \
		fi \
	done

