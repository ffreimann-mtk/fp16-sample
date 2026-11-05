#
# Common makefile definitions.
#
export COLOR_RESET  := "\033[0m"
export COLOR_GREEN  := "\033[0;32m"
export COLOR_RED    := "\033[0;31m"
export COLOR_YELLOW := "\033[0;33m"
export COLOR_BLUE   := "\033[0;34m"

export PROJ_NAME = fp16-test

export ROOT_BUILD_PATH = build

#
# Adjust the YOCTO tool path as required
#
export YOCTO_SDK_PATH ?= /path/to/yocto/sdk
