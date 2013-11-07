SHELL=/bin/sh

dirs := \
	RHConsole \
	RHLog \
	RHDaemon \
	modules

default install clean:
	@for dir in $(dirs); do \
		if [ -d "$$dir" ]; then \
			$(MAKE) -C $$dir $@; \
		fi \
	done

