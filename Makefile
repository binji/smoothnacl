# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
#   http://www.gnu.org/software/make/manual/make.html
#

#
# Get pepper directory for toolchain and includes.
#
# If NACL_SDK_ROOT is not set, then assume it can be found a two directories up,
# from the default example directory location.
#
THIS_MAKEFILE:=$(abspath $(lastword $(MAKEFILE_LIST)))
NACL_SDK_ROOT?=$(abspath $(dir $(THIS_MAKEFILE))nacl_sdk/pepper_23)
CHROME_PATH?=/home/binji/dev/chromium/src/out/Debug/chrome
NEXE_ARGS?=--enable-nacl --ignore-gpu-blacklist --incognito

#
# Defaults
#
NACL_WARNINGS:=-Wno-long-long -Wall -Wswitch-enum -Werror -pedantic

#
# Project Settings
#
VALID_TOOLCHAINS:=newlib glibc
TOOLCHAIN?=newlib

SMOOTHLIFE_CXXFLAGS:=$(NACL_CXXFLAGS) -I$(NACL_SDK_ROOT)/src
SMOOTHLIFE_CXXFLAGS+=-I$(NACL_SDK_ROOT)/src/ppapi/lib/gl


#
# Default target
#
all:

#
# Alias for standard commands
#
CP:=python $(NACL_SDK_ROOT)/tools/oshelpers.py cp
MKDIR:=python $(NACL_SDK_ROOT)/tools/oshelpers.py mkdir
MV:=python $(NACL_SDK_ROOT)/tools/oshelpers.py mv
RM:=python $(NACL_SDK_ROOT)/tools/oshelpers.py rm


#
# Verify we selected a valid toolchain for this example
#
ifeq (,$(findstring $(TOOLCHAIN),$(VALID_TOOLCHAINS)))
$(warning Availbile choices are: $(VALID_TOOLCHAINS))
$(error Can not use TOOLCHAIN=$(TOOLCHAIN) on this example.)
endif


#
# Compute path to requested NaCl Toolchain
#
OSNAME:=$(shell python $(NACL_SDK_ROOT)/tools/getos.py)
TC_PATH:=$(abspath $(NACL_SDK_ROOT)/toolchain)


#
# Verify we have a valid NACL_SDK_ROOT by looking for the toolchain directory
#
ifeq (,$(wildcard $(TC_PATH)))
$(warning No valid NACL_SDK_ROOT at $(NACL_SDK_ROOT))
ifeq ($(origin NACL_SDK_ROOT), 'file')
$(error Override the default value via enviornment variable, or command-line.)
else
$(error Fix the NACL_SDK_ROOT specified in the environment or command-line.)
endif
endif


#
# Disable DOS PATH warning when using Cygwin based NaCl tools on Windows
#
CYGWIN ?= nodosfilewarning
export CYGWIN


#
# Defaults for TOOLS
#
Debug_CCFLAGS?=-g -O0
Release_CCFLAGS?=-O2
Debug_LDFLAGS?=-g
Release_LDFLAGS?=
x86_32_CCFLAGS?=-m32
x86_64_CCFLAGS?=-m64
x86_32_LDFLAGS?=-m32
x86_64_LDFLAGS?=-m64

newlib_CC?=$(TC_PATH)/$(OSNAME)_x86_newlib/bin/i686-nacl-gcc -c
newlib_CXX?=$(TC_PATH)/$(OSNAME)_x86_newlib/bin/i686-nacl-g++ -c -std=gnu++98
newlib_LINK?=$(TC_PATH)/$(OSNAME)_x86_newlib/bin/i686-nacl-g++ -Wl,-as-needed
newlib_LIB?=$(TC_PATH)/$(OSNAME)_x86_newlib/bin/i686-nacl-ar r
newlib_DUMP?=$(TC_PATH)/$(OSNAME)_x86_newlib/x86_64-nacl/bin/objdump
newlib_CCFLAGS?=-MMD -pthread $(NACL_WARNINGS) -idirafter $(NACL_SDK_ROOT)/include
newlib_LDFLAGS?=-pthread -L 

glibc_CC?=$(TC_PATH)/$(OSNAME)_x86_glibc/bin/i686-nacl-gcc -c
glibc_CXX?=$(TC_PATH)/$(OSNAME)_x86_glibc/bin/i686-nacl-g++ -c -std=gnu++98
glibc_LINK?=$(TC_PATH)/$(OSNAME)_x86_glibc/bin/i686-nacl-g++ -Wl,-as-needed
glibc_LIB?=$(TC_PATH)/$(OSNAME)_x86_glibc/bin/i686-nacl-ar r
glibc_DUMP?=$(TC_PATH)/$(OSNAME)_x86_glibc/x86_64-nacl/bin/objdump
glibc_PATHS:=-L $(TC_PATH)/$(OSNAME)_x86_glibc/x86_64-nacl/lib32
glibc_PATHS+=-L $(TC_PATH)/$(OSNAME)_x86_glibc/x86_64-nacl/lib
glibc_CCFLAGS?=-MMD -pthread $(NACL_WARNINGS) -idirafter $(NACL_SDK_ROOT)/include
glibc_LDFLAGS?=-pthread



#
# NMF Manifiest generation
#
# Use the python script create_nmf to scan the binaries for dependencies using
# objdump.  Pass in the (-L) paths to the default library toolchains so that we
# can find those libraries and have it automatically copy the files (-s) to
# the target directory for us.
NMF:=python $(NACL_SDK_ROOT)/tools/create_nmf.py


#
# Verify we can find the Chrome executable if we need to launch it.
#
.PHONY: CHECK_FOR_CHROME
CHECK_FOR_CHROME:
ifeq (,$(wildcard $(CHROME_PATH)))
	$(warning No valid Chrome found at CHROME_PATH=$(CHROME_PATH))
	$(error Set CHROME_PATH via an environment variable, or command-line.)
else
	$(warning Using chrome at: $(CHROME_PATH))
endif

#
# Per target object lists
#
SMOOTHLIFE_OBJS:=smoothlife_instance smoothlife_view smoothlife_module kernel functions smoother simulation smoothlife_thread

# $1 toolchain, $2 config, $3 arch, $4 object
define CC_RULE
$(1)/$(2)/$(4)_$(3).o : $(4).cc $(THIS_MAKE) | $(1)/$(2)
	$$($(1)_CC) -o $$@ $$< $$($(2)_CCFLAGS) $$($(3)_CCFLAGS) $$($(1)_CCFLAGS) $$(SMOOTHLIFE_CXXFLAGS) -Ilib/$(1)_$(3)/include
endef

# $1 toolchain, $2 config, $3 arch
define ARCH_RULE
SMOOTHLIFE_$(1)_$(2)_$(3)_O:=$$(patsubst %,$(1)/$(2)/%_$(3).o,$(SMOOTHLIFE_OBJS))
$(1)/$(2)/smoothlife_$(3).nexe : $$(SMOOTHLIFE_$(1)_$(2)_$(3)_O)
	$$($(1)_LINK) -o $$@ $$^ $$($(2)_LDFLAGS) $$($(3)_LDFLAGS) $$($(1)_LDFLAGS) $$(SMOOTHLIFE_LDFLAGS) -L$$(NACL_SDK_ROOT)/lib/$$(OSNAME)_$(1)_$(3)/$(2) -Llib/$(1)_$(3)/lib -lppapi_gles2 -lppapi -lpthread -lppapi_cpp -lfftw3

$$(foreach obj,$$(SMOOTHLIFE_OBJS),$$(eval $$(call CC_RULE,$(1),$(2),$(3),$$(obj))))
endef

# $1 toolchain, $2 config, $3 arch list
define CONFIG_RULE
$(1)/$(2): | $(1)
	$$(MKDIR) $(1)/$(2)
-include $(1)/$(2)/*.d

ALL_TARGETS+=$(1)/$(2)/smoothlife.nmf
SMOOTHLIFE_$(1)_$(2)_NEXES:=$$(patsubst %,$(1)/$(2)/smoothlife_%.nexe,$(3))
$(1)/$(2)/smoothlife.nmf : $$(SMOOTHLIFE_$(1)_$(2)_NEXES)
	$$(NMF) -D $$($(1)_DUMP) $$($(1)_PATHS) -o $$@ $$^ -t $(1) -s $(1)/$(2)
$$(foreach arch,$(3),$$(eval $$(call ARCH_RULE,$(1),$(2),$$(arch))))
endef

# $1 toolchain, $2 arch list
define TOOLCHAIN_RULE
$(1):
	$$(MKDIR) $(1)
$$(eval $$(call CONFIG_RULE,$(1),Debug,$(2)))
$$(eval $$(call CONFIG_RULE,$(1),Release,$(2)))
endef

$(eval $(call TOOLCHAIN_RULE,newlib,x86_32 x86_64))
#$(eval $(call TOOLCHAIN_RULE,glibc,x86_32 x86_64))

#
# Target to remove temporary files
#
.PHONY: clean
clean:
	$(RM) -fr newlib/Debug
	$(RM) -fr newlib/Release
	$(RM) -fr glibc/Debug
	$(RM) -fr glibc/Release


all: $(ALL_TARGETS)

RUN: all
	python ../httpd.py

CONFIG?=Debug
PAGE?=index_$(TOOLCHAIN)_$(CONFIG).html

LAUNCH: CHECK_FOR_CHROME all
ifeq (,$(wildcard $(PAGE)))
	$(warning No valid HTML page found at $(PAGE))
	$(error Make sure TOOLCHAIN and CONFIG are properly set)
endif
	$(CHROME_PATH) $(NEXE_ARGS) localhost:5103/$(PAGE)
