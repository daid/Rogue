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

#define RN                (((seed = seed*11109+13849) >> 16) & 0xffff)
#ifdef CTRL
#undef CTRL
#endif
#define CTRL(c)                (c & 037)

/*
 * Now all the global variables
 */

extern bool        got_ltc, in_shell;
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
void        getltchars();
void        land();
void    leave(int);
void        my_exit();
void        nohaste();
void        playit();
void    playltchars(void);
void        print_disc(char);
void    quit(int);
void    resetltchars(void);
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
char        *md_crypt(char *key, char *salt);
int        md_dsuspchar();
int        md_erasechar();
char        *md_gethomedir();
char        *md_getusername();
int        md_getuid();
char        *md_getpass(char *prompt);
int        md_getpid();
char        *md_getrealname(int uid);
void        md_init();
int        md_killchar();
void        md_normaluser();
int        md_readchar();
void       md_flush_input();
int        md_setdsuspchar(int c);
int        md_suspchar();
int        md_unlink(char *file);
int        md_unlink_open_file(char *file, FILE *inf);
int md_issymlink(char *sp);

