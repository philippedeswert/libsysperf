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
# File: sp-textconv.py
#
# Author: Simo Piiroinen
#
# -----------------------------------------------------------------------------
#
# History
#
# 14-Aug-2006 Simo Piiroinen
# - added html title control
#
# 26-Jan-2006 Simo Piiroinen
# - added preprocessor and '::INC path' directive
#
# 22-Jul-2005 Simo Piiroinen
# - added image & clickmap support
#
# 06-Jul-2005 Simo Piiroinen
# - added command line parser
#
# 29-Jun-2005 Simo Piiroinen
# - imported to libsysperf
#
# 21-Apr-2005 Simo Piiroinen
# - rewritten from scratch
# - compatible with old markup.py
# =============================================================================

import sys,os,string

# ============================================================================
# Usage And Version Query Support
# ============================================================================

TOOL_NAME = os.path.splitext(os.path.basename(sys.argv[0]))[0]
TOOL_VERS = "0.0.3"
TOOL_HELP = """\
NAME
  <NAME> <VERS>  --  simple text to html converter with TOC generator

SYNOPSIS
  <NAME> [options]

DESCRIPTION
  This tool can be used to create simple text or html documentation
  with tables of content from "tagged" ascii source files.

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
  -m <mode> | --mode=<mode>
        Default is html, or determined from output path.

EXAMPLES

  % cat  document.src

  :: <comment row>
  ::HR
  ::TOC

  Heading Level 1
  ===============

  Heading Level 2
  ---------------

  Heading Level 3
  ...............

  Text content

  ::LINK [<link-desc>::]<link-url>
  ::INC <file-to-include>
  ::IMG <image-url>

  ::END

  % <NAME> -f document.src -o document.txt

    Writes formatted text version of document.

  % <NAME> -f document.src -o document.html

    Writes html version of document.

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

TXT = 0
TAG = -1

latin1_to_html = {
  chr(0x22): "&quot;",
  chr(0x26): "&amp;",
  chr(0x3c): "&lt;",
  chr(0x3e): "&gt;",
  chr(0xa0): "&nbsp;",
  chr(0xa1): "&iexcl;",
  chr(0xa2): "&cent;",
  chr(0xa3): "&pound;",
  chr(0xa4): "&curren;",
  chr(0xa5): "&yen;",
  chr(0xa6): "&brvbar;",
  chr(0xa7): "&sect;",
  chr(0xa8): "&uml;",
  chr(0xa9): "&copy;",
  chr(0xaa): "&ordf;",
  chr(0xab): "&laquo;",
  chr(0xac): "&not;",
  chr(0xad): "&shy;",
  chr(0xae): "&reg;",
  chr(0xaf): "&macr;",
  chr(0xb0): "&deg;",
  chr(0xb1): "&plusmn;",
  chr(0xb2): "&sup2;",
  chr(0xb3): "&sup3;",
  chr(0xb4): "&acute;",
  chr(0xb5): "&micro;",
  chr(0xb6): "&para;",
  chr(0xb7): "&middot;",
  chr(0xb8): "&cedil;",
  chr(0xb9): "&sup1;",
  chr(0xba): "&ordm;",
  chr(0xbb): "&raquo;",
  chr(0xbc): "&frac14;",
  chr(0xbd): "&frac12;",
  chr(0xbe): "&frac34;",
  chr(0xbf): "&iquest;",
  chr(0xc0): "&Agrave;",
  chr(0xc1): "&Aacute;",
  chr(0xc2): "&Acirc;",
  chr(0xc3): "&Atilde;",
  chr(0xc4): "&Auml;",
  chr(0xc5): "&Aring;",
  chr(0xc6): "&AElig;",
  chr(0xc7): "&Ccedil;",
  chr(0xc8): "&Egrave;",
  chr(0xc9): "&Eacute;",
  chr(0xca): "&Eirc;",
  chr(0xcb): "&Euml;",
  chr(0xcc): "&Igrave;",
  chr(0xcd): "&Iacute;",
  chr(0xce): "&Icirc;",
  chr(0xcf): "&Iuml;",
  chr(0xd0): "&ETH;",
  chr(0xd1): "&Ntilde;",
  chr(0xd2): "&Ograve;",
  chr(0xd3): "&Oacute;",
  chr(0xd4): "&Ocirc;",
  chr(0xd5): "&Otilde;",
  chr(0xd6): "&Ouml;",
  chr(0xd7): "&times;",
  chr(0xd8): "&Oslash;",
  chr(0xd9): "&Ugrave;",
  chr(0xda): "&Uacute;",
  chr(0xdb): "&Ucirc;",
  chr(0xdc): "&Uuml;",
  chr(0xdd): "&Yacute;",
  chr(0xde): "&THORN;",
  chr(0xdf): "&szlig;",
  chr(0xe0): "&agrave;",
  chr(0xe1): "&aacute;",
  chr(0xe2): "&acirc;",
  chr(0xe3): "&atilde;",
  chr(0xe4): "&auml;",
  chr(0xe5): "&aring;",
  chr(0xe6): "&aelig;",
  chr(0xe7): "&ccedil;",
  chr(0xe8): "&egrave;",
  chr(0xe9): "&eacute;",
  chr(0xea): "&ecirc;",
  chr(0xeb): "&euml;",
  chr(0xec): "&igrave;",
  chr(0xed): "&iacute;",
  chr(0xee): "&icirc;",
  chr(0xef): "&iuml;",
  chr(0xf0): "&eth;",
  chr(0xf1): "&ntilde;",
  chr(0xf2): "&ograve;",
  chr(0xf3): "&oacute;",
  chr(0xf4): "&ocirc;",
  chr(0xf5): "&otilde;",
  chr(0xf6): "&ouml;",
  chr(0xf7): "&divide;",
  chr(0xf8): "&oslash;",
  chr(0xf9): "&ugrave;",
  chr(0xfa): "&uacute;",
  chr(0xfb): "&ucirc;",
  chr(0xfc): "&uuml;",
  chr(0xfd): "&yacute;",
  chr(0xfe): "&thorn;",
  chr(0xff): "&yuml;",
}

# ----------------------------------------------------------------
def slice(txt,sep,cnt):
    if txt == None:
        return [None] * cnt
    v = string.split(txt,sep,cnt-1)
    v = map(string.strip, v)
    while len(v) < cnt:
        v.append(None)
    return v

def rslice(txt,sep,cnt):
    v = slice(txt,sep,cnt)
    v.reverse()
    return v

# ----------------------------------------------------------------
heading_level_lut = { '=':1, '-': 2, '.':3 }

def heading_level(text):

    # if row of text consist only of "underline" chars
    # return corresponding heading level

    c = text[:1]
    if heading_level_lut.has_key(c):
        if text.count(c) == len(text):
            return heading_level_lut[c]
    return TXT

def preprocess(stk,txt,path):
    assert not path in stk
    stk.append(path)

    if path == None:
        srce = sys.stdin
    else:
        srce = open(path)

    for i in srce:
        i = i.rstrip()
        i = i.expandtabs()

        if i[:2] != "::":
            txt.append(i)
            continue
        v = i.split()
        k,v = v[0],v[1:]
        if k == "::END":
            break
        elif k == "::INC":
            p = v[0]
            if p[:1] != "/":
                p = os.path.join(os.path.dirname(path), p)
            preprocess(stk,txt,p)
        else:
            txt.append(i)

    assert stk.pop() == path

# ----------------------------------------------------------------
def read_file(path):

    # read input lines
    # - trim whitespace at eol
    # - expand tabulators -> spaces

    srce, stk = [],[]
    preprocess(stk,srce,path)

## QUARANTINE     if path == None:
## QUARANTINE   srce = sys.stdin.readlines()
## QUARANTINE     else:
## QUARANTINE   srce = open(path).readlines()
## QUARANTINE     srce = map(string.rstrip, srce)
## QUARANTINE     srce = map(string.expandtabs, srce)
## QUARANTINE
## QUARANTINE     i,n = 0,len(srce)
## QUARANTINE     while i < n:
## QUARANTINE   if srce[i] == "::END":
## QUARANTINE       del srce[i:]
## QUARANTINE       break
## QUARANTINE   i += 1

    # go through the text rows in reverse order
    # so that we encounter the heading underlines
    # before the heading texts

    dest = []
    lev  = TXT
    pad  = ["",""]

    while srce:
        row = srce.pop()

        # ::TAG ARGS
        if row[:2] == "::":
            tag,arg = slice(row,None,2)
            dest.append([TAG,tag,arg])
            lev = TXT
            continue

        # previous row was heading underline
        # -> this line is heading text
        if row and lev:
            dest.append([lev,row,None])
            continue

        # if not underline, normal text
        lev = heading_level(row)
        if lev == TXT:
            dest.append([lev,row,None])

    # return rows in original order
    dest.reverse()

    return dest

# ----------------------------------------------------------------
def get_heading_rows(rows):
    return filter(lambda x:x[0]>TXT, rows)

# ----------------------------------------------------------------
def enum_heading_rows(rows):
    # generate heading numbers: 1, 1.1, 1.2, etc

    clr = [0] * 10
    cnt = clr[:]

    for r in get_heading_rows(rows):
        lev = r[0]
        cnt[lev-1] += 1
        cnt = cnt[:lev] + clr[lev:]
        r[2] = string.join(map(str, cnt[:lev]),".")

# ----------------------------------------------------------------
# Row type predicates
def heading_p(r): return r[0] > TXT
def empty_p(r):   return r[0] == TXT and not r[1]

# ----------------------------------------------------------------
def filter_empty_rows_sub(srce,temp):
    while srce:
        r = srce.pop()
        if not empty_p(r):
            temp.append(r)
            continue
        while srce:
            n = srce.pop()
            if empty_p(n):
                continue
            if not heading_p(n):
                temp.append(r)
            temp.append(n)
            break

# ----------------------------------------------------------------
def filter_empty_rows(srce):
    temp = []
    filter_empty_rows_sub(srce, temp)
    filter_empty_rows_sub(temp, srce)

# ----------------------------------------------------------------
def html_escape(text):
    text = map(lambda c: latin1_to_html.get(c,c), text)
    return string.join(text,"")

    text = string.replace(text, "&", "&amp;")
    text = string.replace(text, "<", "&lt;")
    text = string.replace(text, ">", "&gt;")
    return text

# ----------------------------------------------------------------
def pre_on(pre, txt):
    if not pre: txt = "<pre>" + txt
    return 1,txt

# ----------------------------------------------------------------
def pre_off(pre, txt):
    if pre: txt = "</pre>" + txt
    return 0,txt

def img_html(_,arg):
    if "::" in arg:
        alt,img = arg.split("::",1)
    else:
        alt,img = "",arg
    alt,img = map(string.strip, (alt,img))
    base = os.path.splitext(img)[0]
    cmap = base + ".cmap"
    if os.path.exists(cmap):
        _('<map name="%s">' % base)
        for s in open(cmap):
            _(s.rstrip())
        _('</map>')
        _('<img src="%s" usemap="#%s">'%(img,base))
    else:
        _('<img src="%s">'%(img))

# ----------------------------------------------------------------
def to_html(srce, title=None):

    if title == None:
        title = "<no title>"

    # generate TOC as nested HTML lists

    stk = [0]
    toc = []
    _ = lambda x:toc.append(x)

    for lev,txt,arg in get_heading_rows(srce):
        if lev == 1 and stk[-1] > 1:
            _("<br><br>")

        while lev < stk[-1]:
            _("</ul>")
            stk.pop()

        if lev > stk[-1]:
            _("<ul>")
            stk.append(lev)

        tag  = "%s %s"   % (arg,txt)
        tag  = html_escape(tag)
        name = "toc_%s"  % arg
        href = "#doc_%s" % arg

        _('<li><a name="%s"></a><a href="%s">%s</a>' % (name,href,tag))

        if lev == 1: _("<br><br>")

    while stk.pop() > 0:
        _("</ul>")

    toc = string.join(toc, "\n")

    # generate HTML body from source text

    dest = []
    _ = lambda x:dest.append(x)

    _("<html>")
    _("<head>")
    _("<title>")
    _(html_escape(title))
    _("</title>")
    _("</head>")
    _("<body>")
    pre = 0
    for lev,txt,arg in srce:

        if lev == TXT:
            pre,txt = pre_on(pre, html_escape(txt))
            _(txt)

        elif lev == TAG:
            txt,tag = None,txt

            if tag == "::":
                pass
            elif tag == "::HR":
                txt = "<hr>"
            elif tag == "::TOC":
                pre,txt = pre_off(pre,toc)
            elif tag == "::IMG":
                img_html(_,arg)
            elif tag == "::LINK":
                arg = map(string.strip, arg.split("::",1))
                link = arg.pop()
                if arg:
                    name = arg.pop()
                else:
                    name = link
                name = html_escape(name)
                txt = '<a href="%s">%s</a>' % (link, name)
            else:
                print>>sys.stderr, "Unknown TAG: %s" % repr(tag)
                sys.exit(1)

            if txt != None:
                _(txt)
        else:
            txt  = "%s %s"   % (arg,txt)
            name = "doc_%s"  % arg
            href = "#toc_%s" % arg

            txt  = html_escape(txt)

            txt  = '<a href="%s">%s</a>' % (href,txt)
            txt  = '<h%d>%s</h%d>'       % (lev, txt, lev)
            txt  = '<a name="%s"></a>%s' % (name, txt)

            if lev == 1:
                txt = "<hr>\n" + txt

            pre,txt = pre_off(pre, txt)
            _(txt)

    if pre:
        _("</pre>")
    _("</body>")
    _("</html>")
    _("")

    return string.join(dest, "\n")

# ----------------------------------------------------------------
def to_text(srce, title="<no title>"):

    # generate text TOC

    stk = [0]
    toc = []
    _ = lambda x:toc.append(x)

    for lev,txt,arg in get_heading_rows(srce):
        if lev == 1 or stk[-1] == 1:
            _("")
        while lev < stk[-1]:
            stk.pop()
        if lev > stk[-1]:
            stk.append(lev)
        tag  = "%s %s" % (arg,txt)
        pad  = "    " * (len(stk)-1)
        _(pad + tag)

    toc = string.join(toc, "\n")

    # re-format source text

    hr   = "- " * (78/2)
    hr   = "\n%s\n" % hr
    dest = []
    _ = lambda x:dest.append(x)

    for lev,txt,arg in srce:

        if lev == TXT:
            _(txt)

        elif lev == TAG:
            txt,tag = None,txt
            if tag == "::":
                pass
            elif tag == "::HR":
                txt = hr
            elif tag == "::IMG":
                txt = "[IMAGE: %s]" % arg
            elif tag == "::TOC":
                txt = toc
            elif tag == "::LINK":
                arg = map(string.strip, arg.split("::",1))
                txt = "<%s>" % arg.pop()
                if arg:
                    txt = "%s\n%s" % (arg.pop(), txt)
            else:
                print>>sys.stderr, "Unknown TAG: %s" % repr(tag)
                sys.exit(1)

            if txt != None:
                _(txt)
        else:
            if lev == 1: _(hr)
            else:        _("")

            txt  = "%s %s" % (arg, txt)
            pad  = " =-."[lev] * len(txt)
            _(txt)
            _(pad)
            _("")

    _("")

    dest = string.join(dest, "\n")
    i,n = 0, len(dest)
    while i < n and dest[i] == "\n":
        i += 1
    dest = dest[i:]
    return dest

# ============================================================================
# Main Entry Point
# ============================================================================

def main():
    # - - - - - - - - - - - - - - - - - - - - - - - -
    # defaults
    # - - - - - - - - - - - - - - - - - - - - - - - -

    input  = None       # read from stdin
    output = None       # write to stdout
    mode   = None
    title  = None

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

        elif key in ("-f", "--input"):   input  = val or args.pop()
        elif key in ("-o", "--output"):  output = val or args.pop()
        elif key in ("-t", "--title"):   title  = val or args.pop()
        elif key in ("-m", "--mode"):    mode   = val or args.pop()

        else:
            msg_fatal("unknown option: %s\n(use --help for usage)\n" % repr(arg))

    if not mode:

        if not output or os.path.splitext(output)[1] == ".html":
            mode = "html"
        else:
            mode = "text"

    rows = read_file(input)
    filter_empty_rows(rows)
    enum_heading_rows(rows)

    if not title:
        if input:
            title = input
            title = os.path.basename(title)
            title = os.path.splitext(title)[0]
        else:
            title = "<no title>"

    if mode == "html":
        text = to_html(rows, title)
    else:
        text = to_text(rows, title)

    if output == None:
        sys.stdout.write(text)
    else:
        open(output,"w").write(text)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print>>sys.stderr, "User Break"
        sys.exit(1)

###
