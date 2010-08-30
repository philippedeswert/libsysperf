#!/usr/bin/env python

# This file is part of libsysperf.
#
# Copyright (C) 2001, 2004-2007 by Nokia Corporation.
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
# File: chk-unused.py
# 
# Author: Simo Piiroinen
# 
# -----------------------------------------------------------------------------
# 
# History:
# 
# 05-Jul-2005 Simo Piiroinen
# - initial version
# =============================================================================

import sys,os,string

# ============================================================================
# Usage And Version Query Support
# ============================================================================

TOOL_NAME = os.path.splitext(os.path.basename(sys.argv[0]))[0]
TOOL_VERS = "0.0.0"

src_ext = (".c", ".cc")
hdr_ext = (".h", ".inc")

if __name__ == "__main__":
    
    srce = sys.argv[1:]
    if not srce:
	srce = os.listdir(".")

    unused = []
	
    for s in srce:
	b,e = os.path.splitext(s)
	if not e in src_ext:
	    continue
	if not os.path.exists(b + ".o"):
	    unused.append(s)
	    for e in hdr_ext:
		s = b + e
		if os.path.exists(s):
		    unused.append(s)
	    
    unused.sort()
    
    print string.join(unused, "\\\n")
