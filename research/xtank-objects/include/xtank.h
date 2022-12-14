/*-
 * Copyright (c) 1988 Terry Donahue
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// TODO: Remove #if defineds / simplify

#ifndef _XTANK_H_
#define _XTANK_H_

#include <stdlib.h>
#include <stdio.h>

#if defined(SYSV) || defined(SVR4) || defined(__FreeBSD__) || \
	(defined(__APPLE__) && defined(__MACH__))
#define NEED_STRING_H
#endif

#if defined(__FreeBSD__) || (defined(__APPLE__) && defined(__MACH__)) || \
	defined(__linux__)
#define NEED_DIRENT_H
#endif

#if defined(__alpha) || defined(__osf__) || defined(__FreeBSD__) || \
	(defined(__APPLE__) && defined(__MACH__)) || \
	defined(__linux__)
#define USE_DLOPEN
#endif

#if defined(NEED_STRING_H)
#include <string.h>
#endif
#if defined(NEED_STRINGS_H)
#include <strings.h>
#endif

#include <math.h>
#include "screen.h"
#include "vdesc.h"			/* needed for Weapon */
#include "xtanklib.h"			/* many important things here */

#ifdef AMIGA
#include "amiga.h"
#endif

#ifdef TEST_TURRETS
#define TURRET_LENGTH 25
#endif /* TEST_TURRETS */

/* Number of frames animation lasts after end of game */
#define QUIT_DELAY 17
/* Number of frames you get to keep watching after your vehicle is killed */
#define DEATH_DELAY 30

/* Maze geometry information */
#define MAZE_HEIGHT	26
#define MAZE_WIDTH	26
#define MAZE_TOP	2
#define MAZE_BOTTOM	27
#define MAZE_LEFT	2
#define MAZE_RIGHT	27

/* Additional flags used in maze */
#define BOX_CHANGED	(1<<5)
#define VEHICLE_0	(1<<8)
#define ANY_VEHICLE	0x0fffff00

/* General max values */
#define MAX_WEAPON_STATS 20
#define MAX_GAME_SPEED   30

/* Description max values */
#define MAX_VDESCS	100
#define MAX_MDESCS	100
#define MAX_PDESCS	30
#define MAX_SDESCS	30

/* Flags for display routines */
#define OFF	        0
#define ON	        1
#define REDISPLAY       2

/* Return values for animation routine */
#define GAME_FAILED	       (-1)
#define GAME_RUNNING		0
#define GAME_OVER		1
#define GAME_QUIT		2
#define GAME_RESET		3
#define SWAPPED         4

/* Return values for description loading */
#define DESC_LOADED     0
#define DESC_SAVED	1
#define DESC_NOT_FOUND  2
#define DESC_BAD_FORMAT 3
#define DESC_NO_ROOM    4

/*
 * eithier this or use different symbols for do_special,
 * ie not SP_xxx
 * this seemed simple enough
 */

#define SP_update	(0+MAX_SPEC_STATS)
#define SP_activate	(1+MAX_SPEC_STATS)
#define SP_deactivate	(2+MAX_SPEC_STATS)
#define SP_toggle	(3+MAX_SPEC_STATS)
#define SP_draw		(4+MAX_SPEC_STATS)
#define SP_erase	(5+MAX_SPEC_STATS)
#define SP_redisplay	(6+MAX_SPEC_STATS)

#define SPDF_clear	1
#define SPDF_break	2
#define SPDF_fix	3

/* Vehicle, weapon, and program status masks */
#define VS_functioning		(1<<0)
#define VS_is_alive		(1<<1)
#define VS_was_alive		(1<<2)
#define VS_disc_spin		(1<<3)
#define VS_rel_turret		(1<<4)
#define VS_sliding		(1<<5)
#define VS_permanently_dead	(1<<6)

#define WS_on			(1<<0)
#define WS_func			(1<<1)
#define WS_no_ammo		(1<<2)

#define TS_3d			(1<<0)
#define TS_wide 		(1<<1)
#define TS_long 		(1<<2)
#define TS_extend 		(1<<3)
#define TS_clip 		(1<<4)

#define PROG_on                 (1<<0)

/* Types of explosions (move to xtanklib.h?) */
#define EXP_TANK      0
#define EXP_GLEAM     1
#define EXP_DAM0      2
#define EXP_DAM1      3
#define EXP_DAM2      4
#define EXP_DAM3      5
#define EXP_DAM4      6
#define EXP_EXHAUST   7
#define EXP_ELECTRIC  8
#define EXP_NUKE      9

/* Types of descriptions */
#define VDESC 0
#define MDESC 1
#define SDESC 2

/* 3d elevation information */
#define WALLTOP_Z        (BOX_WIDTH/4)
#define WALLBOTTOM_Z     (-BOX_WIDTH/4)
#define TURRET_MOUNT_Z   (BOX_WIDTH/8)
#define SIDE_MOUNT_Z     (-BOX_WIDTH/8)

#define INT_FORCE_DONT   0
#define INT_FORCE_ON     1
#define INT_FORCE_OFF    2

#endif /* _XTANK_H_ */
