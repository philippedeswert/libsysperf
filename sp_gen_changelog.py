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

# ============================================================================
# File: sp_gen_changelog.py
# 
# Author: Simo Piiroinen
# ============================================================================

#----------------------------------------------------------------
# ChangeLog generator
#
# Parses history entries in format used below from given text
# (ie source) files.
#
# History
# 06-Jul-2005 Simo Piiroinen
# - command line parser added
#
# 05-Jul-2005 Simo Piiroinen
# - renamed: gen_changelog.py -> sp_gen_changelog.py
#
# 16-Jun-2005 Simo Piiroinen
# - week combining now sorts with week, not date
#
# 15-Jun-2005 Simo Piiroinen
# - almost complete rewrite
# - combines identical change notes from multiple files
#
# 20-Aug-2004 Simo Piiroinen
# - first version
#
# 23-Sep-2004 Simo Piiroinen
# - now sorted latest first
#
# 05-Nov-2004 Simo Piiroinen
# - entries now grouped by week & file -> more compact listing 
#----------------------------------------------------------------

import sys,os,string,time

# ============================================================================
# Usage And Version Query Support
# ============================================================================

TOOL_NAME = os.path.splitext(os.path.basename(sys.argv[0]))[0]
TOOL_VERS = "0.0.0"
TOOL_HELP = """\
NAME
  <NAME> <VERS>  --  scan changelog entries from source files

SYNOPSIS
  <NAME> [options] [source] ...

DESCRIPTION
  This tool is used to generate changelog from entries in source
  files.
  
  The entries should look like:
      
  05-Jul-2005 <name>
  - <description>
         :
  04-Jul-2005 <name>
  - <description>
         :

OPTIONS
  -h | --help
        This help text
  -V | --version
        Tool version
  -v | --verbose
        Enable diagnostic messages
  -q | --quiet
        Disable warning messages
  -s | --silent
        Disable all messages
  -o <path> | --output=<path>
        Output file to use instead of stdout.
	
EXAMPLES
  % <NAME> -o changelog *.c *.h Makefile
  
    Generates "changelog" from entries in all c sources and Makefile.
  
AUTHOR
    Simo Piiroinen

COPYRIGHT
  Copyright (C) 2001, 2004-2007 Nokia Corporation.

  This is free software.  You may redistribute copies of it under the
  terms of the GNU General Public License v2 included with the software.
  There is NO WARRANTY, to the extent permitted by law.

SEE ALSO
"""
def tool_version():
    print TOOL_VERS
    sys.exit(0)
    
def tool_usage():
    s = TOOL_HELP
    s = s.replace("<NAME>", TOOL_NAME)
    s = s.replace("<VERS>", TOOL_VERS)
    sys.stdout.write(s)
    sys.exit(0)

# ============================================================================
# Message API
# ============================================================================

msg_progname = TOOL_NAME
msg_verbose  = 3

def msg_emit(lev,tag,msg):
    if msg_verbose >= lev:
    	msg = string.split(msg,"\n")
    	msg = map(string.rstrip, msg)
    	while msg and msg[-1] == "":
	    msg.pop()
	    
	pad = "|" + " " * (len(tag)-1)
	
    	for s in msg:
	    s = string.expandtabs(s)
	    print>>sys.stderr, "%s%s" % (tag, s)
	    tag = pad

def msg_fatal(msg):
    msg_emit(1, msg_progname + ": FATAL: ", msg)
    sys.exit(1)
    
def msg_error(msg):
    msg_emit(2, msg_progname + ": ERROR: ", msg)
    
def msg_warning(msg):
    msg_emit(2, msg_progname + ": Warning: ", msg)
    
def msg_silent():
    global msg_verbose
    msg_verbose = 0
    
def msg_more_verbose():
    global msg_verbose
    msg_verbose += 1
    
def msg_less_verbose():
    global msg_verbose
    msg_verbose -= 1

    
COMBINE_WEEK = 01

#----------------------------------------------------------------
# make sure locale will not affect our dates...
m_num = {}
m_str = ('-',
'Jan','Feb','Mar','Apr','May','Jun',
'Jul','Aug','Sep','Oct','Nov','Dec')
for m in m_str: m_num[string.upper(m)] = len(m_num)

#----------------------------------------------------------------
def index_to_month(m):
    "Convert: 2 -> 'Feb'"
    
    assert 1 <= m <= 12
    return m_str[m]

#----------------------------------------------------------------
def month_to_index(m):
    "Convert: 'Feb' or '02' or '2' -> 2"
    
    i = m_num.get(string.upper(m), None)
    return i or int(m,10)


#----------------------------------------------------------------
def date_to_week(ymd):
    assert 1900 <= ymd[0] <= 2100
    assert    1 <= ymd[1] <= 12
    assert    1 <= ymd[2] <= 31

    y = ymd[0]
    m = index_to_month(ymd[1])
    # use ISO week number
    t = ymd + (12,) + (0,)*5
    t = time.mktime(t)
    
    l = time.localtime(t)
    
    
    w = time.strftime("%W", l)
    
    t -= l[6] * 24 * 60 * 60
    l = time.localtime(t)
    #s = "%02d.%02d." % (l[1],l[2])
    s = "%02d.%02d." % (l[2],l[1])
    t += 6 * 24 * 60 * 60
    l = time.localtime(t)
    #e = "%02d.%02d." % (l[1],l[2])
    e = "%02d.%02d." % (l[2],l[1])
    

    return "%04d week %s (%s - %s)" % (y,w,s,e)
    
    return "%04d week %s (%s)" % (y,w,m)

    #return "%04d %s (week %s)" % (y,m,w)


#----------------------------------------------------------------
def get_date(s):
    "Convert: '12-Sep-2004' -> (2004,9,12)"
    
    s = string.split(s,'-')
    if len(s) == 3:
	try:
	    d = int(s[0])
	    y = int(s[2])
	    m = month_to_index(s[1])
	    return (y,m,d)
	except ValueError:
	    pass
    return None

#----------------------------------------------------------------
def get_dateline(s):
    "Parse: <pfix> <date> <author> from line"
    
    v = s.split(None,2)
    if len(v) == 3:
	i = s.find(v[1])
	d = get_date(v[1])
	a = v[2]
	if d != None and a != "":
	    return d,a,s[:i]
    return None,None,None


#----------------------------------------------------------------
def content(path):
    "Reversed list of textlines from file"
    
    v = open(path).readlines()
    v = map(string.rstrip, v)
    v = map(string.expandtabs, v)
    v.reverse()
    return v

#----------------------------------------------------------------
def spaces(s):
    "Count spaces in beginning of string"
    
    n = 0
    for c in s: 
	if c != " ": break
	n += 1
    return n

#----------------------------------------------------------------
def pfix(p, s):
    "Check if string starts with prefix"
    
    if p == None: 
	return 1
    n = min(len(p),len(s))
    return s[:n] == p[:n]


#----------------------------------------------------------------
def text_has_alnum_chars(s):
    "check if string has any alnum chars"
    
    for c in s:
        if 'a' <= c <= 'z': return 1
        if 'A' <= c <= 'Z': return 1
        if '0' <= c <= '9': return 1
    return 0

#----------------------------------------------------------------
class LogEntry:
    def __init__(self, file, date, auth, text):
	self.file = [file]
	self.date = date
	self.auth = auth
	self.left = spaces(text)
	self.text = text[self.left:]
	self.enum = 0
	self.week = date_to_week(date)

    def multi(self):
	return len(self.file) > 1
	
    def __str__(self):
	v = []
	v.append("F:%s"%str(self.file))
	v.append("A:%s"%self.auth)
	v.append("D:%s"%str(self.date))
	v.append("E:%s" % string.replace(self.text,"\n","\\n"))
	return string.join(v)
	
    def add(self, text):
	n = min(self.left, spaces(text))
	self.text += "\n"
	self.text += text[n:]

	
#----------------------------------------------------------------
def parse_file(path, entries):

    text = content(path)
    
    left = None
    date = None
    auth = None
    
    prev = None
    enum = 0
    
    while text:
	s = text.pop()
	if not pfix(left, s):
	    break
	
	d,a,t = get_dateline(s)
	if d != None:
	    date = d
	    auth = a
	    left = t
	    prev = None
	    #print d,a
	    continue
	    
	
	if not date:
	    continue
	
	s = s[len(left):]
	if not s:
	    continue
	
	if not text_has_alnum_chars(s):
	    break

	c = spaces(s)
	
	if prev and prev.left < c:
	    prev.add(s)
	else:
	    prev = LogEntry(path,date,auth,s)
	    entries.append(prev)
	    prev.enum = enum
	    enum += 1


#----------------------------------------------------------------
def combine(entries):
    # same: date, auth & text
    
    m = {}

    for e in entries:
	if COMBINE_WEEK:
	    k = e.week,e.auth,e.text
	else:
	    k = e.date,e.auth,e.text
	if not m.has_key(k):
	    m[k] = e
	else:
	    m[k].file.extend(e.file)
	    
    del entries[:]
    entries.extend(m.values())
    
    for e in entries:
	m = {}
	for f in e.file: m[f] = None
	e.file = m.keys()
	e.file.sort()

#----------------------------------------------------------------
# compare operators for sorting

def cfile(a,b): return cmp(a.file, b.file)
#def cdate(a,b): return cmp(a.date, b.date)
def cdate(a,b): return cmp(b.date, a.date)
def cweek(a,b): return cmp(b.week, a.week)
def cauth(a,b): return cmp(a.auth, b.auth)
def ctext(a,b): return cmp(a.text, b.text)
def cenum(a,b): return cmp(a.enum, b.enum)
def cmult(a,b): return cmp(a.multi(), b.multi())

	
def cmp_entry(a,b):
    if COMBINE_WEEK:
	return cweek(a,b) or cmult(a,b) or cfile(a,b) or cauth(a,b) or cenum(a,b)
    return cdate(a,b) or cmult(a,b) or cfile(a,b) or cauth(a,b) or cenum(a,b)

#----------------------------------------------------------------
def filelist(file, text):
    "prettyprint large number of files"
    
    todo = file[:]
    todo.reverse()
    
    blk = None
    wdt = 9999
    
    rows = []
    
    while todo:
	p = todo.pop()
	n = len(p)

	if wdt + 1 + n > 78:
	    blk = [p]
	    wdt = n
	    rows.append(blk)
	else:
	    blk.append(p)
	    wdt += 1 + n

    rows[-1][-1] += ":"
	    
    for blk in rows:
	text.append(string.join(blk))
	
#----------------------------------------------------------------
    
def generate(entries):
    "output changelog"
    
    week = None
    date = None
    file = None
    auth = None
    
    bar1 = "="*78

    res = []
    _ = lambda x:res.append(x)
    
    for e in entries:
	
	if COMBINE_WEEK:
	    if week != e.week:
		week = e.week
		file = None
		auth = None
		_("")
		_(bar1)
		_("%s" % week)
		_(bar1)
	else:
	    if date != e.date:
		date = e.date
		file = None
		auth = None
		_(bar1)
		_("%s" % `date`)
		_(bar1)

	if file != e.file:
	    file = e.file
	    auth = None
	    _("")
	    filelist(file, res)
	    
	if auth != e.auth:
	    auth = e.auth
	    _(" "*4 + "%s" % auth)
	    
	text = string.split(e.text,"\n")
	for i in text:
	    _(" "*8 + "%s" % i)
    
    _("")
    return string.join(res, "\n")
	    
# ============================================================================
# Main Entry Point
# ============================================================================

def main():
    # - - - - - - - - - - - - - - - - - - - - - - - -
    # defaults
    # - - - - - - - - - - - - - - - - - - - - - - - -
    
    output = None # stdout
    
    # - - - - - - - - - - - - - - - - - - - - - - - -
    # parse command line options
    # - - - - - - - - - - - - - - - - - - - - - - - -
    
    srce = []
    args = sys.argv[1:]
    args.reverse()
    
    while args:
	arg = args.pop()
	
	if arg[:1] != "-":
	    srce.append(arg)
	    continue
	
	if arg[:2] == "--":
	    if '=' in arg:
		key,val = string.split(arg,"=",1)
	    else:
		key,val = arg, ""
	else:
	    key,val = arg[:2],arg[2:]
	
	if   key in ("-h", "--help"):    tool_usage()
	elif key in ("-V", "--version"): tool_version()
	elif key in ("-v", "--verbose"): msg_more_verbose()
	elif key in ("-q", "--quiet"):   msg_less_verbose()
	elif key in ("-s", "--silent"):  msg_silent()

	elif key in ("-o", "--output"):  output = val or args.pop()
	
	else:
	    msg_fatal("unknown option: %s\n(use --help for usage)\n" % repr(arg))

    entries = []
    for path in srce:
	parse_file(path, entries)

    combine(entries)
    entries.sort(cmp_entry)
    text = generate(entries)
    
    if output != None:
	open(output,"w").write(text)
    else:
	sys.stdout.write(text)


if __name__ == "__main__":
    try:
	main()
    except KeyboardInterrupt:
	print>>sys.stderr, "User Break"
	sys.exit(1)
