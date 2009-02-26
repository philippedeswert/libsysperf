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

#================================================================
# sp_gen_manfile.py  --  runtime '--help' to man page converter
#
# Author: Simo Piiroinen
#
# History
#
# 25-Feb-2009 Simo Piiroinen
# - fixed groff escaping
# - source for title line (.TH) can be set at command line
#
# 29-Jun-2006 Simo Piiroinen
# - escape also '\'' chars
#
# 30-May-2006 Simo Piiroinen
# - transforms Capitalized sections to UPPERCASE
#
# 26-Jan-2006 Simo Piiroinen
# - supressed double warnings due to missing/empty sections
#
# 07-Jul-2005 Simo Piiroinen
# - incorrect .TH line fixed
#
# 06-Jul-2005 Simo Piiroinen
# - shuffled sections warning shows removed sections too
# - overrides for: section, name, version, brief via -Dkey=val
# - major cleanup
# - copied from track2
#
# 15-Jun-2005 Simo Piiroinen
# - rearranges sections
# - added more sanity checks
#
# 15-Dec-2004 Simo Piiroinen
# - exclicit application & manfile paths must be given
# - directory component for manfile is automatically created
#
# 09-Dec-2004 Simo Piiroinen
# - output to 'app.1' instead of 'app.py.1'
# - some comments added
# - removed some debug stuff
#
# 05-Nov-2004 Simo Piiroinen
# - python scripts (*.py) executed with python, not default
#   shabang app (to avoid running arm python in sbox)
#
# 25-Oct-2004 Simo Piiroinen
# - first version
#================================================================

import sys,os,string,time
from stat import *

# ============================================================================
# Usage And Version Query Support
# ============================================================================

TOOL_NAME = os.path.splitext(os.path.basename(sys.argv[0]))[0]
TOOL_VERS = "0.0.0"
TOOL_HELP = """\
NAME
  <NAME> <VERS>  --  generate man files from text source

SYNOPSIS
  <NAME> [options]

DESCRIPTION
  Tool for auto-generating man pages. 
  
  The input data must be of the same format as <NAME> itself
  produces with --help option.
  
  The input can be obtained from text file, or directly from
  command by using --help option.

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
  -c <appl> | --command=<appl>
        Get input from 'appl --help'.
  -o <path> | --output=<path>
        Output file to use instead of stdout.
  -Dname=value
        Overides for parsed values: name, version, 
	section, source and brief.
	
EXAMPLES
  % <NAME> -c track -o track.1
  
    Generates man file from 'track --help' output.
  
  % <NAME> -f libfoo.txt -Dversion=1.2.3 -Dsection=3 -o libfoo.so.3
  
    Generates man file from textfile contents, section "Library calls",
    and version explicitly set to 1.2.3.

AUTHOR
    Simo Piiroinen
    
COPYRIGHT
  Copyright (C) 2001, 2004-2007 Nokia Corporation.

  This is free software.  You may redistribute copies of it under the
  terms of the GNU General Public License v2 included with the software.
  There is NO WARRANTY, to the extent permitted by law.

SEE ALSO
  man(1)
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

# ----------------------------------------------------------------------------
# these are the first sections in a man page
sec_head     = ( "NAME","SYNOPSIS","DESCRIPTION","OPTIONS","EXAMPLES" )

# these are the last sections in a man page
sec_tail     = ( "AUTHOR","REPORTING BUGS","COPYRIGHT","SEE ALSO" )

# fix some section names
sec_remap    = { "EXAMPLE" : "EXAMPLES", }

# these sections are optional
sec_optional = ( "AUTHOR","REPORTING BUGS","SEE ALSO")

# ----------------------------------------------------------------------------
# Just to be sure that locale will not be used for month names
man_month = "JanFebMarAprMayJunJulAugSepOctNovDec"

def man_date():
    "Date as inserted in the man file"
    
    y,m = time.localtime(time.time())[:2]
    m = (m-1)*3
    return "%s %d" % (man_month[m:m+3], y)


# ----------------------------------------------------------------------------

man_escape_list = (
("\\","\(rs"), # backslash needs to be the first entry
(".", "\\."),
("'","\\(cq"),
('"',"\\(dq"),
)

def man_escape(s):
    "Escape text so that it can be inserted to MAN file"
 
    for f,t in man_escape_list:
        s = s.replace(f,t)
    return s


# ----------------------------------------------------------------------------
def normalize_version(vers):
    "makes sure that version string is of type 1.2.3"
    
    v = vers
    if v[:1] == "v":
	v = v[1:]
    v = v.split(".")
    if len(v) != 3:
	msg_fatal("version string '%s' not format 1.2.3" % vers)
    try:
	v = map(int, v)
    except ValueError:
	msg_fatal("version string '%s' not format 1.2.3" % vers)
    return string.join(map(str, v), ".")
    

# ----------------------------------------------------------------------------
def readlines(file):
    "read & normalize text file contents"
    
    text = file.readlines()
    text = map(string.expandtabs, text)
    text = map(string.rstrip, text)
    return text

# ----------------------------------------------------------------------------
def white_cnt(s):
    "number of leading white space chars in a string"
    
    n = 0
    for c in s:
	if c != " ": break
	n += 1
    #print "%3d:%s" % (n,s)
    return n

# ----------------------------------------------------------------------------
def normalize_sections(secs):
    "remove leading white space from section content"
    
    n = 99
    for s in secs:
	for r in s[1]:
	    c = white_cnt(r)
	    if c and c < n:
		n = c
    if n:
	for s in secs:
	    s[1] = map(lambda x:x[n:], s[1])
    
# ----------------------------------------------------------------------------
def collect_section(secs, name):
    "remove named section from section list"
    
    hits = 0
    rows = []
    
    i = 0
    while i < len(secs):
	s = secs[i]
	if s[0] == name:
	    rows.extend(s[1])
	    del secs[i]
	    hits += 1
	else:
	    i += 1
	    
    if not hits:
	return None
    
    return [name, rows]

# ----------------------------------------------------------------------------
def collect_sections(secs, names, missing, empty):
    "remove named sections from section list"
    
    res = []
    for n in names:
	s = collect_section(secs, n)
	if s == None:
	    missing.append(n)
	    if n in sec_optional:
		msg_warning("missing section: %s" % n)
	    else:
		msg_error("missing section: %s" % n)
	elif not s[1]:
	    empty.append(n)
	    msg_warning("empty section: %s" % n)
	else:
	    res.append(s)
    return res
    
# ----------------------------------------------------------------------------
def parse_name(text):
    "parse NAME line from input text"
    
    while text:
	s = text.pop()
	# allow empty lines at start of output
	if s == "":
	    continue
	# allow full man style output
	if s == "NAME":
	    continue
	return s.strip()
    msg_fatal("no valid name line found!")


# ----------------------------------------------------------------------------

def sec_p(row):
    "determine if a input line is section header"
    
    if row == "" or row[0] <= " ": 
	return None
    while row[-1] == ":":
	row = row[:-1]
    return row.upper()
    
# ----------------------------------------------------------------------------
def parse_help_output(text, stab):
    "transform text input to section list"
    
    # - - - - - - - - - - - - - - - - - - - - - - - -
    # try to get name and version string from output
    #
    # "<name> <vers> -- <brief>"
    # - - - - - - - - - - - - - - - - - - - - - - - -
    
    vers  = None
    brief = None
    name  = parse_name(text)
    
    for s in (" -- ", " - "):
	if s in name:
	    name, brief = map(string.strip, name.split(s,1))
	    break

    s = name.split()
    if len(s) >= 3:
	msg_fatal("incorrect NAME line %s" % repr(name))
    if len(s) >= 2:
	vers = s[1]
    name = s[0]

    # - - - - - - - - - - - - - - - - - - - - - - - -
    # overrides from command line?
    # - - - - - - - - - - - - - - - - - - - - - - - -

    if stab["name"]:    name  = stab["name"]
    if stab["version"]: vers  = stab["version"]
    if stab["brief"]:   brief = stab["version"]

    # - - - - - - - - - - - - - - - - - - - - - - - -
    # must have valid NAME
    # - - - - - - - - - - - - - - - - - - - - - - - -
    
    if not name:
	msg_fatal("empty NAME parsed")
	
    # "ALL CAPS" -> "all caps"
    if name == name.upper():
	name = name.lower()

    #print>>sys.stderr, name, vers, brief
	
    # - - - - - - - - - - - - - - - - - - - - - - - -
    # must have valid version
    # - - - - - - - - - - - - - - - - - - - - - - - -

    # try to get version string from command ?
    if not vers and stab["command"]:
	vers = readlines(os.popen("%s --version" % stab["command"]))
	if len(vers) < 1:
	    msg_fatal("version output empty\n")
	    
	if len(vers) > 1:
	    msg_fatal("version output >1 line\n")
	
	vers = vers[0]

    if not vers:
	msg_fatal("unable to determine version, use: -Dversion=1.2.3\n")
	
    # make sure it is valid
    vers = normalize_version(vers)

    # - - - - - - - - - - - - - - - - - - - - - - - -
    # split output to section headers & content lines
    # - - - - - - - - - - - - - - - - - - - - - - - -
    
    secs = []
    while text:
	sec = sec_p(text.pop())
	if not sec:
	    continue
	
	txt = []
	while text:
	    row = text.pop()
	    if sec_p(row):
		text.append(row)
		break
	    txt.append(row)
	while txt and txt[-1] == "":
	    txt.pop()
	secs.append([sec, txt])

    # - - - - - - - - - - - - - - - - - - - - - - - -
    # strip leading white space from content lines
    # - - - - - - - - - - - - - - - - - - - - - - - -

    normalize_sections(secs)

    # - - - - - - - - - - - - - - - - - - - - - - - -
    # add NAME section
    # - - - - - - - - - - - - - - - - - - - - - - - -

    if brief:
	brief = "%s - %s" % (name, brief)
    else:
	brief = name

    secs = [["NAME",[brief]]] + secs


    # - - - - - - - - - - - - - - - - - - - - - - - -
    # shuffle sections to standard order
    # - - - - - - - - - - - - - - - - - - - - - - - -

    # original order
    orig = map(lambda x:x[0], secs)
    
    # reorganize
    
    missing,empty = [],[]
    head = collect_sections(secs, sec_head, missing,empty)
    tail = collect_sections(secs, sec_tail, missing,empty)
    secs = head + secs + tail
    
    # compare with original
    used = map(lambda x:x[0], secs)

    # ... but ignore stuff we have already warned about
    tags = missing+empty
    orig = filter(lambda x:not x in tags, orig)
    used = filter(lambda x:not x in tags, used)
    
    if orig != used:
	while len(used) < len(orig):
	    used.append("(none)")
	while len(orig) < len(used):
	    orig.append("(none)")
	n = max(map(len, orig))
	r = ["section order shuffled:"]
	for a,b in zip(orig, used):
	    if a == b:
		a,b = a.lower(),b.lower()
		sep = "    "
	    else:
		sep = "<-->"

	    r.append("%-*s %s %s" % (n,a,sep,b))
	r = string.join(r,"\n")
	msg_warning(r)

    return name,vers,secs

# ----------------------------------------------------------------------------
def man_generate(text, stab):
    "generate man page from text input"
    
    name,vers,secs = parse_help_output(text, stab)
    
    tag = {
    "1" : "User Commands",
    "2" : "System Calls",
    "3" : "Library Calls",
    "4" : "Special Files",
    "5" : "File Formats",
    "6" : "Games",
    "7" : "Miscellaneous",
    "8" : "System Administration Commands",
    "9" : "Kernel Routines",
    }
    
    man = ['.\\" AUTOGENERATED FILE - DO NOT EDIT']
    _ = lambda x:man.append(x)

    # The first non-empty line should contain tool name and version

    
    title   = name.upper()
    section = stab["section"]
    date    = man_date()
    source  = stab["source"]
    manual  = tag.get(section, "Unknown")
    
    t = [title, section, date, source, manual]
    t = map(man_escape, t)
    _('.TH "%s"' % string.join(t, '" "'))
    
    for sec,rows in secs:
	_('.SH %s' % man_escape(sec))
	for r in rows: 
	    _("\\&%s" % man_escape(r))
	    _(".br")
	    
    # Remove excess empty lines
    
    out = []
    while man:
	s = man.pop()
	out.append(s)
	if s[:3] == ".SH":
	    while man and man[-1] in ("",".br"):
		man.pop()
    out.reverse()
    
    # Concatenate rows & write output
    
    out.append('')
    out = string.join(out, "\n")
    
    return out
    

# ----------------------------------------------------------------------------
def txt2man(stab):
    "man file converter"
    
    input   = stab["input"]
    output  = stab["output"]
    command = stab["command"]
    	
    # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    # parse usage rows
    # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    
    if command:
	base = os.path.splitext(command)[0]
	for e in (".txt", ".man", ".src"):
	    text = base + e
	    #print>>sys.stderr, "trying %s ..." % repr(text)
	    if os.path.exists(text):
		msg_warning("using %s as command output" % repr(text))
		text = open(text)
		stab["command"] = None
		break
	else:
	    text = os.popen("%s --help" % command)
    elif input:
	text = open(input)
    else:
	text = sys.stdin
	
    text = readlines(text)
    text = filter(lambda x:x[:1]!='#', text)
    text.reverse()
    
    man = man_generate(text, stab)
    
    if output:
	open(output,"w").write(man)
    else:
	sys.stdout.write(man)
    

# ============================================================================
# Main Entry Point
# ============================================================================

def main():
    # - - - - - - - - - - - - - - - - - - - - - - - -
    # defaults
    # - - - - - - - - - - - - - - - - - - - - - - - -
    
    stab = {
    "input"   : None, # stdin
    "output"  : None, # stdout
    "command" : None, # read from input
    "name"    : None, # read from input
    "version" : None, # read from input
    "brief"   : None, # read from input
    "section" : "1",  # user commands
    "source"  : "SysPerf Tools",
    }
    
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
	
	elif key in ("-f", "--input"):   stab["input"]   = val or args.pop()
	elif key in ("-o", "--output"):  stab["output"]  = val or args.pop()
	elif key in ("-c", "--command"): stab["command"] = val or args.pop()
	elif key in ("-D", "--define"):
	    val = val or args.pop()
	    if '=' in val:
		key,val = val.split('=',1)
	    else:
		key,val = val, ""
	    if not stab.has_key(key):
		msg_warning("unknown define: %s" % repr(key))
	    stab[key] = val
	else:
	    msg_fatal("unknown option: %s\n(use --help for usage)\n" % repr(arg))
	    
    txt2man(stab)

if __name__ == "__main__":
    try:
	main()
    except KeyboardInterrupt:
	print>>sys.stderr, "User Break"
	sys.exit(1)
