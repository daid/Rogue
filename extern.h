/*
 * Defines for things used in mach_dep.c
 *
 * @(#)extern.h        4.35 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

/*
 * Don't change the constants, since they are used for sizes in many
 * places in the program.
 */

#include <stdbool.h>
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "jedi.h"

#define MAXSTR                1024        /* maximum length of strings */
#define MAXLINES        32        /* maximum number of screen lines used */
#define MAXCOLS                80        /* maximum number of screen columns used */

#ifdef CTRL
#undef CTRL
#endif
#define CTRL(c)                ((c) | 0x1000)

//Custom key codes returned from md_readchar()
#define K_UP_LEFT                0x101
#define K_UP                     0x102
#define K_UP_RIGHT               0x103
#define K_LEFT                   0x104
#define K_RIGHT                  0x105
#define K_DOWN_LEFT              0x106
#define K_DOWN                   0x107
#define K_DOWN_RIGHT             0x108
#define K_SHIFT_UP_LEFT          0x201
#define K_SHIFT_UP               0x202
#define K_SHIFT_UP_RIGHT         0x203
#define K_SHIFT_LEFT             0x204
#define K_SHIFT_RIGHT            0x205
#define K_SHIFT_DOWN_LEFT        0x206
#define K_SHIFT_DOWN             0x207
#define K_SHIFT_DOWN_RIGHT       0x208

#define K_EXIT                   0x300
/*
 * Now all the global variables
 */

extern char        fruit[], prbuf[];
extern int orig_dsusp;
extern FILE        *scoreboard;

/*
 * Function types
 */

int         come_down(int arg);
int         doctor(int arg);
int         land(int arg);
void    leave(int);
void        my_exit(int code);
int         nohaste(int arg);
void        playit();
void        print_disc(char);
void    quit(int);
int         rollwand(int arg);
int         runners(int arg);
void        set_order(int *order, int numthings);
int         sight(int arg);
int         stomach(int arg);
int         swander(int arg);
void        tstp(int ignored);
int         unconfuse(int arg);
int         unsee(int arg);
int         visuals(int arg);

const char  *killname(char monst, bool doart);
const char  *nothing(char type);
const char  *type_name(int type);

void       md_init();
int        md_readchar();
void       md_flush_input();
int        md_unlink(const char *file);
int        md_unlink_open_file(const char *file, FILE *inf);

