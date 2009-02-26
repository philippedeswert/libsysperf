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
 * File: proc_stat.h
 *
 * Author: Simo Piiroinen
 *
 * -------------------------------------------------------------------------
 *
 * History:
 *
 * 31-May-2006 Simo Piiroinen
 * - copied from track2 source tree
 * ========================================================================= */

#ifndef PROC_STAT_H_
#define PROC_STAT_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct proc_stat_t
{
  int           pid;    // %d
  /* The process id.
   */

  char comm[64];        // %s - %quoted
  /* The filename of the executable, in
   * parentheses. This is visible
   * whether or not the executable is
   * swapped out.
   */

  int          state;   // %c
  /* One character from the string
   * "RSDZTW" where R is running, S is
   * sleeping in an interruptible wait, D
   * is waiting in uninterruptible disk
   * sleep, Z is zombie, T is traced or
   * stopped (on a signal), and W is pag-
   * ing.
   */

  int           ppid;   // %d
  /* The PID of the parent.
   */

  int           pgrp;   // %d
  /* The process group ID of the process.
   */

  int           session;        // %d
  /* The session ID of the process.
   */

  int           tty_nr; // %d
  /* The tty the process uses.
   */

  int           tpgid;  // %d
  /* The process group ID of the process
   * which currently owns the tty that
   * the process is connected to.
   */

  unsigned long flags;  // %lu
  /* The flags of the process. The math
   * bit is decimal 4, and the traced bit
   * is decimal 10.
   */

  unsigned long minflt; // %lu
  /* The number of minor faults the pro-
   * cess has made which have not
   * required loading a memory page from
   * disk.
   */

  unsigned long cminflt;        // %lu
  /* The number of minor faults that the
   * process and its children have made.
   */

  unsigned long majflt; // %lu
  /* The number of major faults the pro-
   * cess has made which have required
   * loading a memory page from disk.
   */

  unsigned long cmajflt;        // %lu
  /* The number of major faults that the
   * process and its children have made.
   */

  unsigned long utime;  // %lu
  /* The number of jiffies that this pro-
   * cess has been scheduled in user
   * mode.
   */

  unsigned long stime;  // %lu
  /* The number of jiffies that this pro-
   * cess has been scheduled in kernel
   * mode.
   */

  signed long cutime;   // %ld
  /* The number of jiffies that this pro-
   * cess and its children have been
   * scheduled in user mode.
   */

  signed long cstime;   // %ld
  /* The number of jiffies that this pro-
   * cess and its children have been
   * scheduled in kernel mode.
   */

  signed long priority; // %ld
  /* The standard nice value, plus fif-
   * teen. The value is never negative
   * in the kernel.
   */

  signed long nice;     // %ld
  /* The nice value ranges from 19
   * (nicest) to -19 (not nice to oth-
   * ers).
   */

  signed long unused0;  // %ld
  /* This value is hard coded to 0 as a
   * placeholder for a removed field.
   */

  signed long itrealvalue;      // %ld
  /* The time in jiffies before the next
   * SIGALRM is sent to the process due
   * to an interval timer.
   */

  unsigned long starttime;      // %lu
  /* The time in jiffies the process
   * started after system boot.
   */

  unsigned long vsize;  // %lu
  /* Virtual memory size in bytes.
   */

  signed long rss;      // %ld
  /* Resident Set Size: number of pages
   * the process has in real memory,
   * minus 3 for administrative purposes.
   * This is just the pages which count
   * towards text, data, or stack space.
   * This does not include pages which
   * have not been demand-loaded in, or
   * which are swapped out.
   */

  unsigned long rlim;   // %lu
  /* Current limit in bytes on the rss of
   * the process (usually 4,294,967,295).
   */

  unsigned long startcode;      // %lu
  /* The address above which program text
   * can run.
   */

  unsigned long endcode;        // %lu
  /* The address below which program text
   * can run.
   */

  unsigned long startstack;     // %lu
  /* The address of the start of the
   * stack.
   */

  unsigned long kstkesp;        // %lu
  /* The current value of esp (stack
   * pointer), as found in the kernel
   * stack page for the process.
   */

  unsigned long kstkeip;        // %lu
  /* The current EIP (instruction
   * pointer).
   */

  unsigned long signal; // %lu
  /* The bitmap of pending signals (usu-
   * ally 0).
   */

  unsigned long blocked;        // %lu
  /* The bitmap of blocked signals (usu-
   * ally 0, 2 for shells).
   */

  unsigned long sigignore;      // %lu
  /* The bitmap of ignored signals.
   */

  unsigned long sigcatch;       // %lu
  /* The bitmap of catched signals.
   */

  unsigned long wchan;  // %lu
  /* This is the "channel" in which the
   * process is waiting. It is the
   * address of a system call, and can be
   * looked up in a namelist if you need
   * a textual name. (If you have an up-
   * to-date /etc/psdatabase, then try ps
   * -l to see the WCHAN field in
   * action.)
   */

  unsigned long nswap;  // %lu
  /* Number of pages swapped - not main-
   * tained.
   */

  unsigned long cnswap; // %lu
  /* Cumulative nswap for child pro-
   * cesses.
   */

  int           exit_signal;    // %d
  /* Signal to be sent to parent when we
   * die.
   */

  int           processor;      // %d
  /* Processor number last executed on.
   */

} proc_stat_t;

static inline void proc_stat_dtor(void) { }
static inline void proc_stat_ctor(void) { }

void proc_stat_update(proc_stat_t *self, char *data);
int  proc_stat_parse(proc_stat_t *self, const char *path);
void proc_stat_repr(proc_stat_t *self, FILE *file);

#ifdef __cplusplus
};
#endif

#endif // PROC_STAT_H_
