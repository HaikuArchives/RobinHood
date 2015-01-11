SHELL=/bin/sh

dirs := \
	404Handler \
	CGIHandler \
	DirectoryHandler \
	FileHandler	\
	PHPHandler	\
	RedirectHandler	\
	SSIHandler

default install clean:
	@for dir in $(dirs); do \
		if [ -d "$$dir" ]; then \
			INSTALL_DIR=../../server/robin_hood_modules $(MAKE) -C $$dir $@; \
		fi \
	done
