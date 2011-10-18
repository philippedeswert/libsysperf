# This file is part of libsysperf.
#
# Copyright (C) 2001, 2004-2007 by Nokia Corporation
#
# Contact: Eero Tamminen <eero.tamminen@nokia.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
# 02110-1301 USA

# =============================================================================
# File: Makefile
#
# Author: Simo Piiroinen
#
# -----------------------------------------------------------------------------
#
# History:
#
# 07-Jul-2005 Simo Piiroinen
# - scans python files too for changelog
# - cleanup
#
# 06-Jul-2005 Simo Piiroinen
# - generates man files for binaries
# - added: sp_fix_tool_vers
# - added: sp_gen_manfile
# - added: sp_textconv
#
# 05-Jul-2005 Simo Piiroinen
# - removed: sp_csv_filter
# - exports: sp_gen_changelog
# - debian/rules compatible install
# - cleanup
#
# 28-Jun-2005 Simo Piiroinen
# - added sp_csv_filter.c, calculator.c, array.c, str_array.c
#
# 22-Jun-2005 Simo Piiroinen
# - added writer.c
#
# 21-Jun-2005 Simo Piiroinen
# - initial version
# =============================================================================

# -----------------------------------------------------------------------------
# Directory Config
# -----------------------------------------------------------------------------
NAME   ?= libsysperf
ROOT   ?= /tmp/$(NAME)-testing
PREFIX ?= /usr
BIN    ?= $(PREFIX)/bin
LIB    ?= $(PREFIX)/lib
HDR    ?= $(PREFIX)/include/$(NAME)
MAN1   ?= $(PREFIX)/share/man/man1

# -----------------------------------------------------------------------------
# Common Compilation Options
# -----------------------------------------------------------------------------

BUILD  ?= final

CFLAGS  += -Wall
CFLAGS  += -Werror
CFLAGS  += -std=c99
CFLAGS  += -D_GNU_SOURCE

ifeq ($(BUILD),debug)
CFLAGS  += -g# -finstrument-functions
LDFLAGS += -g
endif

ifeq ($(BUILD),gprof)
CFLAGS  += -g -pg
LDFLAGS += -g -pg
CFLAGS  += -O0
endif

ifeq ($(BUILD),final)
CFLAGS  += -O3
LDFLAGS += -s
endif

# -----------------------------------------------------------------------------
# Development Package Files
# -----------------------------------------------------------------------------

BIN_TARGETS += sp_gen_changelog
BIN_TARGETS += sp_fix_tool_vers
BIN_TARGETS += sp_gen_manfile
BIN_TARGETS += sp_textconv

LIB_TARGETS += libsysperf.a

MAN_TARGETS += $(patsubst %,%.1.gz,$(BIN_TARGETS))

HEADERS +=\
	argvec.h\
	array.h\
	calculator.h\
	cstring.h\
	csv_float.h\
	csv_table.h\
	csv_calc.h\
	hash.h\
	mem_pool.h\
	msg.h\
	reader.h\
	release.h\
	str_array.h\
	str_pool.h\
	writer.h\
	xmalloc.h\
	proc_maps.h\
	proc_meminfo.h\
	proc_stat.h\
	proc_statm.h\
	proc_status.h

# -----------------------------------------------------------------------------
# Top Level Targets
# -----------------------------------------------------------------------------

TARGETS = $(LIB_TARGETS) $(BIN_TARGETS) $(MAN_TARGETS)

.PHONY: build clean mostlyclean distclean tags depend install changelog

build:: $(TARGETS) $(HEADERS)

install:: install-dev

clean:: mostlyclean
	$(RM) $(TARGETS)

mostlyclean::
	$(RM) *.o *.q *~

distclean:: clean
	$(RM) tags

tags::
	ctags *.c *.h

changelog::
	./sp_gen_changelog.py >$@ *.c *.h *.inc *.py Makefile

.PHONY: tree
tree :: install
	tree $(ROOT)

# -----------------------------------------------------------------------------
# Compilation Macros & Rules
# -----------------------------------------------------------------------------

# C -> asm
%.q	: %.c
	$(CC) -S -o $@ $(CFLAGS) $<

# PY -> installable script
%	: %.py
	./sp_fix_tool_vers.py -f$< -o$@
	@chmod a-w $@

# generating MAN pages
%.1.gz : %.1
	gzip -9 -c <$< >$@
%.1    : %
	./sp_gen_manfile.py -c ./$< -o $@

# updating static libraries
lib%.a :
	$(AR) ru $@ $^

# -----------------------------------------------------------------------------
# Installation Macros & Rules
# -----------------------------------------------------------------------------

INSTALL_DIR = $(if $1, install -m755 -d $2/)
INSTALL_MAN = $(if $1, install -m644 $1 $2/)
INSTALL_BIN = $(if $1, install -m755 $1 $2/)
INSTALL_LIB = $(if $1, install -m644 $1 $2/)
INSTALL_HDR = $(if $1, install -m644 $1 $2/)

install-%-man::
	$(call INSTALL_DIR,$^,$(ROOT)$(MAN1))
	$(call INSTALL_MAN,$^,$(ROOT)$(MAN1))

install-%-bin::
	$(call INSTALL_DIR,$^,$(ROOT)$(BIN))
	$(call INSTALL_BIN,$^,$(ROOT)$(BIN))

install-%-lib::
	$(call INSTALL_DIR,$^,$(ROOT)$(LIB))
	$(call INSTALL_LIB,$^,$(ROOT)$(LIB))

install-%-hdr::
	$(call INSTALL_DIR,$^,$(ROOT)$(HDR))
	$(call INSTALL_HDR,$^,$(ROOT)$(HDR))

# -----------------------------------------------------------------------------
# Development Package Installation
# -----------------------------------------------------------------------------

install-dev:: install-dev-lib install-dev-hdr install-dev-bin install-dev-man
install-dev-bin:: $(BIN_TARGETS)
install-dev-man:: $(MAN_TARGETS)
install-dev-lib:: $(LIB_TARGETS)
install-dev-hdr:: $(HEADERS)

# -----------------------------------------------------------------------------
# Dependency Scanning
# -----------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),depend)
include .depend
endif

depend::
	gcc -MM *.c > .depend

# -----------------------------------------------------------------------------
# Target Specific Dependencies
# -----------------------------------------------------------------------------

libsysperf.a : \
	msg.o\
	argvec.o\
	xmalloc.o\
	cstring.o\
	csv_table.o\
	csv_float.o\
	csv_calc.o\
	hash.o\
	mem_pool.o\
	reader.o\
	writer.o\
	str_pool.o\
	calculator.o\
	array.o\
	str_array.o\
	proc_maps.o\
	proc_meminfo.o\
	proc_stat.o\
	proc_statm.o\
	proc_status.o

sp_csv_filter : LDLIBS += -lm
sp_csv_filter : sp_csv_filter.o libsysperf.a

unused:\
fake_csv_pass.c\
fake_track.c\
str_split.c\
str_split.h

testmain: LDLIBS += -lm
testmain: testmain.o libsysperf.a

sp_csv_filter: CFLAGS += -I.
sp_csv_filter: LDLIBS += -lm
sp_csv_filter: sp_csv_filter.o libsysperf.a

# -----------------------------------------------------------------------------
# EOF
# -----------------------------------------------------------------------------
