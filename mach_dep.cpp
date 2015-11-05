/*
 * Various installation dependent routines
 *
 * @(#)mach_dep.c        4.37 (Berkeley) 05/23/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

/*
 * The various tuneable defines are:
 *
 *        SCOREFILE        Where/if the score file should live.
 *        ALLSCORES        Score file is top ten scores, not top ten
 *                        players.  This is only useful when only a few
 *                        people will be playing; otherwise the score file
 *                        gets hogged by just a few people.
 *        NUMSCORES        Number of scores in the score file (default 10).
 *        NUMNAME                String version of NUMSCORES (first character
 *                        should be capitalized) (default "Ten").
 *        MAXLOAD                What (if any) the maximum load average should be
 *                        when people are playing.  Since it is divided
 *                        by 10, to specify a load limit of 4.0, MAXLOAD
 *                        should be "40".         If defined, then
 *      LOADAV                Should it use it's own routine to get
 *                        the load average?
 *      NAMELIST        If so, where does the system namelist
 *                        hide?
 *        MAXUSERS        What (if any) the maximum user count should be
 *                        when people are playing.  If defined, then
 *      UCOUNT                Should it use it's own routine to count
 *                        users?
 *      UTMP                If so, where does the user list hide?
 *        CHECKTIME        How often/if it should check during the game
 *                        for high load average.
 */

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include "extern.h"

#ifdef __linux__
# define SCOREFILE "/var/lib/griffin/rogue.score"
#else
# define SCOREFILE "rogue.score"
#endif

# ifndef NUMSCORES
#        define        NUMSCORES        10
#        define        NUMNAME                "Ten"
# endif

unsigned int numscores = NUMSCORES;
const char *Numname = NUMNAME;

bool allscore = TRUE;

/*
 * init_check:
 *        Check out too see if it is proper to play the game now
 */

void
init_check()
{
}

/*
 * open_score:
 *        Open up the score file for future use
 */

void
open_score()
{
#ifdef SCOREFILE
    const char *scorefile = SCOREFILE;
     /* 
      * We drop setgid privileges after opening the score file, so subsequent 
      * open()'s will fail.  Just reuse the earlier filehandle. 
      */

    if (scoreboard != NULL) { 
        rewind(scoreboard); 
        return; 
    } 

    scoreboard = fopen(scorefile, "rb+");

    if ((scoreboard == NULL) && (errno == ENOENT))
    {
        scoreboard = fopen(scorefile, "wb+");
    }

    if (scoreboard == NULL) { 
         fprintf(stderr, "Could not open %s for writing: %s\n", scorefile, strerror(errno)); 
         fflush(stderr); 
    } 
#else
    scoreboard = NULL;
#endif
}

/*
 * setup:
 *        Get starting setup for all games
 */

void setup()
{
}

/*
 * start_score:
 *        Start the scoring sequence
 */

void
start_score()
{
}

/*
 * flush_type:
 *        Flush typeahead for traps, etc.
 */

void
flush_type()
{
    md_flush_input();
}
