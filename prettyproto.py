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
# File: prettyproto.py
#
# Author: Simo Piiroinen
#
# -----------------------------------------------------------------------------
#
# Usage: cproto source.c | ./prettyproto.py
#
# History:
#
# 23-May-2006 Simo Piiroinen
# - documented
# =============================================================================

import sys

def sym_p(c):
    if '0' <= c <= '9': return 1
    if 'a' <= c <= 'z': return 1
    if 'A' <= c <= 'Z': return 1
    if c in "_": return 1
    return None

if __name__ == "__main__":

    o = []
    m = {}

    for s in sys.stdin:
        s = s.expandtabs()
        s = s.strip()
        a,b = None,None
        i,n = 0,len(s)
        while i < n:
            if s[i] == '(':
                b = i
                break
            if not sym_p(s[i]):
                a = i + 1
            i += 1
        else:
            continue

        t = s[:a]
        n = s[a:b]
        a = s[b:]
        p = ''
        while t[-1] == '*':
            p += '*'
            t = t[:-1]

        k = n.split('_')[0]

        if not m.has_key(k):
            m[k] = []
            o.append(k)

        m[k].append((t,p,n,a))

## QUARANTINE     m = m.items()
## QUARANTINE     m.sort()
## QUARANTINE     for k,v in m:

    for k in o:
        v = m[k]

        print ""
        print "/* -- %s -- */" % k
        print ""
        i = max(map(lambda x:len(x[0]), v))
        k = max(map(lambda x:len(x[1]), v))
        l = max(map(lambda x:len(x[2]), v))

        for t,p,n,a in v:
            print "%-*s%-*s%-*s%s" % (i,t,k,p,l,n,a)
