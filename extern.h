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
#define K_UP_LEFT                0x101
#define K_UP                     0x102
#define K_UP_RIGHT               0x103
#define K_LEFT                   0x104
#define K_RIGHT                  0x105
#define K_DOWN_LEFT              0x106
#define K_DOWN                   0x107
#define K_DOWN_RIGHT             0x108
/*
 * Now all the global variables
 */

extern int        wizard;
extern char        fruit[], prbuf[];
extern int orig_dsusp;
extern FILE        *scoreboard;

/*
 * Function types
 */

void    auto_save(int);
void        come_down();
void        doctor();
void        land();
void    leave(int);
void        my_exit();
void        nohaste();
void        playit();
void        print_disc(char);
void    quit(int);
void        rollwand();
void        runners();
void        set_order();
void        sight();
void        stomach();
void        swander();
void        tstp(int ignored);
void        unconfuse();
void        unsee();
void        visuals();

char        *killname(char monst, bool doart);
char        *nothing(char type);
char        *type_name(int type);

int        md_chmod(char *filename, int mode);
char        *md_gethomedir();
char        *md_getusername();
int        md_getuid();
int        md_getpid();
void        md_init();
int        md_readchar();
void       md_flush_input();
int        md_unlink(char *file);
int        md_unlink_open_file(char *file, FILE *inf);
int md_issymlink(char *sp);

