/* -*- mode: c -*- */

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
 * File: proc_meminfo.inc
 * 
 * Author: Simo Piiroinen
 * 
 * -------------------------------------------------------------------------
 * 
 * History:
 * 
 * 30-May-2006 Simo Piiroinen
 * - moved from track2 source tree
 * ========================================================================= */

#ifdef VAR 
VAR(MemTotal)
VAR(MemFree)
VAR(MemShared)
VAR(Buffers)
VAR(Cached)
VAR(SwapCached)
VAR(Active)
VAR(Inact_dirty)
VAR(Inact_clean)
VAR(Inact_target)
VAR(HighTotal)
VAR(HighFree)
VAR(LowTotal)
VAR(LowFree)
VAR(SwapTotal)
VAR(SwapFree)
VAR(Committed_AS)
#undef VAR
#endif
