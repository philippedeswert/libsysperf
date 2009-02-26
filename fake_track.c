/*
 * This file is part of libsysperf
 *
 * Copyright (C) 2001, 2004-2007 by Nokia Corporation.
 *
 * Contact: Eero Tamminen <eero.tamminen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/* ========================================================================= *
 * File: fake_track.c
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 21-Jun-2005 Simo Piiroinen
 * - initial version
 * ========================================================================= */

/* ========================================================================= *
 * Include Files
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "msg.h"
#include "argvec.h"

/* ========================================================================= *
 * Configuration
 * ========================================================================= */

/* ------------------------------------------------------------------------- *
 * Tool Version
 * ------------------------------------------------------------------------- */

#define TOOL_NAME "track"
#define TOOL_VERS "0.4.2"

/* ------------------------------------------------------------------------- *
 * Runtime Manual
 * ------------------------------------------------------------------------- */

static const manual_t app_man[]=
{
  MAN_ADD("NAME",
          TOOL_NAME"\n"
          )
  MAN_ADD("SYNOPSIS",
          ""TOOL_NAME" <options>\n"
          )
  MAN_ADD("DESCRIPTION",
          "This tool records memory and cpu usage information from\n"
          "the /proc file system.\n"
          )
  MAN_ADD("OPTIONS", 0)

  MAN_ADD("EXAMPLES",
          "% "TOOL_NAME" -otrack.csv -ltrack.log -ptrack.pid &\n"
          "% [run applications]\n"
          "% kill -HUP `cat track.pid`\n"
          "% fixtrack -itrack.csv -oclean.csv\n"
          )
  MAN_ADD("DATA COLLECTED BY 'TRACK'",
          "COLUMN   ORIGIN   DESCRIPTION\n"
          "\n"
          "secs     track    [s] Time since the first poll\n"
          "pid      track    Process identifier\n"
          "\n"
          "ppid     stat     Parent process indentifier\n"
          "comm     stat     Command name\n"
          "\n"
          "VmSize   status   [kB] TOTAL size of process VM address space.\n"
          "VmData   status   [kB] Size of portions configured as DATA\n"
          "VmStk    status   [kB] Size of portions configured as STACK\n"
          "VmExe    status   [kB] Size of portions configured as APPBIN\n"
          "VmLib    status   [kB] Size of portions configured as DYNLIBS\n"
          "VmLck    status   [kB] Pages locked to stay in RAM.\n"
          "VmRSS    status   [kB] Pages currently resident in RAM.\n"
          "\n"
          "vsize    stat     Should be equal to VmSize\n"
          "rss      stat     Should be equal to VmRSS\n"
          "utime    stat     [Jiffies] Time spent in user space.\n"
          "stime    stat     [Jiffies] Time spent in system space.\n"
          "\n"
          "ttime    track    [Jiffies] Total time spent = utime + stime\n"
          "\n"
          "size     statm    Should be equal to VmSize, but is not\n"
          "resident statm    Should be equal to VmRSS\n"
          "shared   statm    [kB] Amount of resident RAM shared with other\n"
          "                       processes\n"
          "trs      statm    [kB] amount of APPBIN in RAM.\n"
          "drs      statm    [kB] amount of DATA + STACK in RAM.\n"
          "lrs      statm    [kB] amount of DYNLIBS in RAM.\n"
          "dt       statm    [kB] Amount of ram that is dirty (non-discardable)\n"
          )

  MAN_ADD("DATA CALCULATED BY 'FIXTRACK' DURING POSTPROCESSING'",
          "private  fixtrack  = resident - shared\n"
          "clean    fixtrack  = resident - dt\n"
          "VmMaps   fixtrack  = VmSize - (VmExe+VmLib+VmStk+VmData)\n"
          )
  MAN_ADD("COPYRIGHT",
          "Copyright (C) 2001, 2004-2007 Nokia Corporation.\n\n"
          "This is free software.  You may redistribute copies of it under the\n"
          "terms of the GNU General Public License v2 included with the software.\n"
          "There is NO WARRANTY, to the extent permitted by law.\n"
          )
  MAN_ADD("SEE ALSO",
          "proc(5), top(1),\n"
          "/usr/src/linux/Documentation/filesystems/proc.txt\n"
          )
  MAN_END
};

/* ------------------------------------------------------------------------- *
 * Commandline Arguments
 * ------------------------------------------------------------------------- */

enum
{
  opt_noswitch = -1,

  opt_help,
  opt_vers,
  opt_output,
  opt_pidfile,
  opt_logfile,
  opt_no_scan,
  opt_poll_freq,
};

static const option_t app_opt[] =
{
  OPT_ADD(opt_help,
          "h", "help", 0,
          "This help text\n"),

  OPT_ADD(opt_vers,
          "V", "version", 0,
          "Tool version\n"),

  OPT_ADD(opt_output,
          "o", "output", "<destination path>",
          "Output file to use instead of stdout.\n" ),

  OPT_ADD(opt_logfile,
          "l", "log-file", "<path>",
          "Message file to use instead of stderr.\n" ),

  OPT_ADD(opt_pidfile,
          "p", "pid-file", "<path>",
          "Process ID will be written to this file.\n" ),

  OPT_ADD(opt_no_scan,
          0, "no-scan", 0,
          "Only pseudo process information like committed_as is tracked.\n" ),

  OPT_ADD(opt_poll_freq,
          "F", "poll-freq", "<time>",
          "Polling is done every <time> milliseconds [1000ms].\n" ),

  OPT_END
};

/* ========================================================================= *
 * Main Entry Point
 * ========================================================================= */

int main(int ac, char **av)
{
  printf("hello, world\n");

  argvec_t *args = argvec_create(ac, av, app_opt, app_man);

  int   tag;
  char *par;

  while( !argvec_done(args) )
  {
    if( !argvec_next(args, &tag, &par) )
    {
      msg_error("(use --help for usage)\n");
      exit(1);
    }

    switch( tag )
    {
    case opt_help:
      //argvec_options(args, 1);
      argvec_usage(args);
      exit(0);

    default:
      printf("tag=%d, par='%s'\n", tag, par);
      break;
    }
  }

  argvec_delete(args);

  printf("bye, world\n");

  return 0;
}
