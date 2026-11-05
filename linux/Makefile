#
# The top level Makefile from which all other Makefiles are called.
# It defines the source files and the targets for each platform.
#
include ../make/Common.mak

export SRCS =	\
	../common/src/convld.c \
	src/main.c


.PHONY: all yocto_armv7 yocto_armv8 clean

all: yocto_armv8

yocto_armv8:
	@$(MAKE) -f Makefile.yocto_armv8 all

clean:
	@$(MAKE) -f Makefile.yocto_armv8 clean
