/*
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 *
 * @(#)main.c        4.22 (Berkeley) 02/05/99
 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include "rogue.h"

/*
 * main:
 *        The main program, of course
 */
int main(int argc, char **argv)
{
    md_init();
    initJedi();

    /*
     * get home and options from environment
     */

#ifdef __linux__
    strcpy(file_name, "/var/lib/griffin/rogue.save");
#else
    strcpy(file_name, "rogue.save");
#endif

    open_score();

    /*
     * check for print-score option
     */

    if (argc == 2)
    {
        if (strcmp(argv[1], "-s") == 0)
        {
            score(0, -1, 0);
            exit(0);
        }
    }

    init_check();                        /* check for legal startup */
    restore("-r");

    init_probs();                        /* Set up prob tables for objects */
    init_player();                        /* Set up initial player stats */
    init_names();                        /* Set up names of scrolls */
    init_colors();                        /* Set up colors of potions */
    init_stones();                        /* Set up stone settings of rings */
    init_materials();                        /* Set up materials of wands */
    setup();

    displayMessage("WELCOME TO THE DUNGEONS OF DOOM");
    new_level();                        /* Draw current level */
    /*
     * Start up daemons and fuses
     */
    start_daemon(runners, 0, AFTER);
    start_daemon(doctor, 0, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    start_daemon(stomach, 0, AFTER);
    playit();
    return(0);
}

/*
 * rnd:
 *        Pick a very random number.
 */
int
rnd(int range)
{
    return range == 0 ? 0 : getRandomNumber(range - 1);
}

/*
 * roll:
 *        Roll a number of dice
 */
int 
roll(int number, int sides)
{
    int dtotal = 0;

    while (number--)
        dtotal += rnd(sides)+1;
    return dtotal;
}

/*
 * playit:
 *        The main loop of the program.  Loop until the game is over,
 *        refreshing things and looking at the proper times.
 */

void
playit()
{
    oldpos = hero;
    while (playing)
        command();                        /* Command execution */
    my_exit(0);
}

/*
 * quit:
 *        Have player make certain, then exit.
 */

void
quit(int sig)
{
    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (displayMessage("really quit?") == 'y')
    {
        msg("You quit with %d gold pieces", purse);
        score(purse, 1, 0);
        my_exit(0);
    }
    else
    {
        status(false);
        refreshMap();
        count = 0;
        to_death = false;
    }
}

/*
 * leave:
 *        Leave quickly, but curteously
 */

void
leave(int sig)
{
    my_exit(0);
}

/*
 * my_exit:
 *        Leave the process properly
 */

void
my_exit(int st)
{
    exit(st);
}

