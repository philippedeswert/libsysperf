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
# File: fix_tool_vers.py
# 
# Author: Simo Piiroinen
#
# Synopis
#
#    Single file mode: explicitly named source & destination
#    
#    % fix_tool_vers.py util.py temp.py
#    % install -m755 temp.py /path/to/bin/util
#
#    Batch mode: will overwrite source files (backups created)
#
#    % fix_tool_vers.py -- file1.py file2.py ...
#
# Description
#
#    This tool is used to sync version strings in python scripts with
#    the release version stored in C header "release.h".
#
# Options
#
#    -L <libpath>  --  appends <libpath> to module search path just
#                      before import csvlib line
#
# History
#
#    12-Oct-2006 Simo Piiroinen
#    - checks release.h against debian/changelog
#
#    06-Jul-2005 Simo Piiroinen
#    - added command line parser
#
#    16-Jun-2005 Simo Piiroinen
#    - now allows appending module seach path (for csvlib) using -L
#
#    15-Jun-2005 Simo Piiroinen
#    - now allows overwriting existing files if different from source file
#
#    08-Mar-2005 Simo Piiroinen
#    - first version
# ============================================================================

import sys,os,string
from stat import *

# ============================================================================
# Usage And Version Query Support
# ============================================================================

TOOL_NAME = os.path.splitext(os.path.basename(sys.argv[0]))[0]
TOOL_VERS = "0.0.0"
TOOL_HELP = """\
NAME
  <NAME> <VERS>  --  updates TOOL_VERS in python scripts

SYNOPSIS
  <NAME> [options]

DESCRIPTION
  This tool updates TOOL_VERS in python scripts according to C
  header file "release.h".
  
  If "debian/changelog" exists, the version information on the
  first line is compared against one defined in "release.h".

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
  -f <path> | --input=<path>
        Input file to use instead of stdin.
  -o <path> | --output=<path>
        Output file to use instead of stdout.
	
EXAMPLES
  % <NAME> -f app.py -o app
  
    Updates TOOL_VERS found in "app.py" and writes fixed version
    to "app".
  
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

VERBOSE = 0

def get_version_from_debian_changelog(path="debian/changelog"):
    if os.path.exists(path):
	t = open(path).readline()
	t = t.split("(",1)[1]
	t = t.split(")",1)[0]
	return '"%s"' % t
    return ''

def get_version_from_release_header(path="release.h"):
    for s in open(path):
	if "define" in s and "TOOL_VERS" in s:
	    a = s.find('"')
	    b = s.find('"',a+1)
	    if b > a:
		return s[a:b+1]
    return ''
    

def chk_tool_vers(vers):
    # sanity check: does it look like a string
    assert vers[0] == '"' and vers[-1] == '"'
    
    # sanity check: is it "minor.major.revision"
    v = string.split(vers[1:-1], ".")
    assert len(v) == 3
    
    # sanity check: minor, major, revision are integers
    v = map(int, v)
    
    return v

def get_tool_vers(incl):
    vers = '"0.0.0"'
    rver = get_version_from_release_header(incl)
    dver = get_version_from_debian_changelog()
    
    if dver and rver != dver:
	msg_fatal("\n".join([
	"version mismatch",
	"%s - debian/changelog" % dver,
	"%s - %s" % (rver, incl)]))

    vers = dver or rver
    chk_tool_vers(vers)
    return vers


def fix_tool_vers(text, srce, vers):
    "fix TOOL_VERS string"
    
    for i in range(len(text)):
	s = text[i]
	s = string.split(s)
	
	if len(s) < 3: continue
	if s[0] != "TOOL_VERS": continue
	if s[1] != "=": continue

	if VERBOSE:
	    print>>sys.stderr, "%s: %s -> %s" % (srce, s[2], vers)
	
	# check that old version looks valid
	chk_tool_vers(s[2])
	
	# replace with new version
	s[2] = vers
	text[i] = string.join(s)
	break
    else:
	print>>sys.stderr, "%s: WARNING: TOOL_VERS not found" % srce

def fix_tool_path(text, srce, libs):
    "fix module search path"

    # fix path
    hit = 0
    tag = "# <-- fix_tool_vers.py"
    fix = 'sys.path.append("%s") %s' % (libs, tag)
    i,n = 0,len(text)
    while i < n:
	s = text[i]
	#print>>sys.stderr,"<<",s
	
	# remove old modifications
	if s.find(tag) != -1:
	    print>>sys.stderr,"!!",s
	    del text[i]
	    continue
	
	# check for 'import sys'
	k = s.find("import")
	if k != -1 and s.find("sys") > k:
	    hit = 1
	
	# modify sys.path
	if s.find("import") != -1 and s.find("csvlib") != -1:
	    text.insert(i, fix)
	    if not hit:
		print>>sys.stderr, "%s: WARNING: does not import sys" % srce
		text.insert(i, "import sys " + tag)
	    break
	i += 1
    else:
	print>>sys.stderr, "%s: WARNING: does not use csvlib.py" % srce
    

def fix_tool(srce, dest, vers, libs):
    # use default mode flags
    mode = None
    
    # read input
    if srce != None:
	orig = open(srce).read()
	mode = os.stat(srce).st_mode & 0777
    else:
	srce = "<stdin>"
	orig = sys.stdin.read()

    # split to lines
    text = orig.split('\n')
    text = map(string.rstrip, text)
    while text and text[-1] == "":
	text.pop()
	
    # do the fixes requested
    if vers: fix_tool_vers(text, srce, vers)
    if libs: fix_tool_path(text, srce, libs)

    
    # reconstruct file contents
    text.append("")
    text = string.join(text, "\n")

    if text == orig:
	print>>sys.stderr, "%s: WARNING: fixed == original" % srce
	
    if dest != None:
	# files used
	temp = dest + ".tmp"
	back = dest + ".bak"
	
    	# write to temp file
    	if os.path.exists(temp):
	    print>>sys.stderr, "%s: FATAL: existing temp file" % srce
	    sys.exit(1)
    	open(temp,"w").write(text)
    
    	# remove old backup
    	if os.path.exists(back):
	    os.remove(back)
	    
    	# create new backup
    	if os.path.exists(dest):
	    os.rename(dest, back)
	    
    	# rename new content
    	os.rename(temp, dest)
    	if mode != None:
	    os.chmod(dest, mode)
    else:
	# just write to stdout
	sys.stdout.write(text)

# ============================================================================
# Main Entry Point
# ============================================================================

def main():
    # - - - - - - - - - - - - - - - - - - - - - - - -
    # defaults
    # - - - - - - - - - - - - - - - - - - - - - - - -

    incl   = "release.h" # find version from C header
    input  = None        # read from stdin
    output = None        # write to stdout
    libs   = None	 # do not fix library path
        
    
    # - - - - - - - - - - - - - - - - - - - - - - - -
    # parse command line options
    # - - - - - - - - - - - - - - - - - - - - - - - -
    
    args = sys.argv[1:]
    args.reverse()
    
    while args:
	arg = args.pop()
	
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

	elif key in ("-f", "--input"):   input = val or args.pop()
	elif key in ("-o", "--output"):  output = val or args.pop()
	
	else:
	    msg_fatal("unknown option: %s\n(use --help for usage)\n" % repr(arg))


    # - - - - - - - - - - - - - - - - - - - - - - - -
    # release version from release.h
    # - - - - - - - - - - - - - - - - - - - - - - - -
    vers = get_tool_vers(incl)

    # - - - - - - - - - - - - - - - - - - - - - - - -
    # do the fixing
    # - - - - - - - - - - - - - - - - - - - - - - - -
    fix_tool(input, output, vers, libs)



if __name__ == "__main__":
    try:
	main()
    except KeyboardInterrupt:
	print>>sys.stderr, "User Break"
	sys.exit(1)
