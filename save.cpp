/*
 * save and restore routines
 *
 * @(#)save.c        4.33 (Berkeley) 06/01/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "rogue.h"
#include "score.h"

typedef struct stat STAT;

static STAT sbuf;

/*
 * save_game:
 *        Implement the "save game" command
 */

void
save_game()
{
    FILE *savef;

    /*
     * test to see if the file exists
     */
    if (stat(file_name, &sbuf) >= 0)
    {
        md_unlink(file_name);
    }
    if ((savef = fopen(file_name, "wb")) == NULL)
        return;

    save_file(savef);
    /* NOTREACHED */
}

/*
 * save_file:
 *        Write the saved game on the file
 */

void save_file(FILE *savef)
{
    char buf[80];
    encwrite(version, strlen(version)+1, savef);
    sprintf(buf,"%d x %d\n", NUMLINES, NUMCOLS);
    encwrite(buf,80,savef);
    rs_save_file(savef);
    fflush(savef);
    fclose(savef);
    exit(0);
}

/*
 * restore:
 *        Restore a saved game from a file with elaborate checks for file
 *        integrity from cheaters
 */
bool restore(const char *file)
{
    FILE *inf;
    char buf[MAXSTR];
    int lines, cols;

    if (strcmp(file, "-r") == 0)
        file = file_name;


    if ((inf = fopen(file,"rb")) == NULL)
    {
        return FALSE;
    }

    fflush(stdout);
    encread(buf, (unsigned) strlen(version) + 1, inf);
    if (strcmp(buf, version) != 0)
    {
        printf("Sorry, saved game is out of date.\n");
        return FALSE;
    }
    encread(buf,80,inf);
    sscanf(buf,"%d x %d\n", &lines, &cols);

    if (lines > NUMLINES)
    {
        printf("Sorry, original game was played on a screen with %d lines.\n",lines);
        printf("Current screen only has %d lines. Unable to restore game\n", NUMLINES);
        return(FALSE);
    }
    if (cols > NUMCOLS)
    {
        printf("Sorry, original game was played on a screen with %d columns.\n",cols);
        printf("Current screen only has %d columns. Unable to restore game\n", NUMCOLS);
        return(FALSE);
    }

    setup();

    rs_restore_file(inf);
    /*
     * we do not close the file so that we will have a hold of the
     * inode for as long as possible
     */

    if (md_unlink_open_file(file, inf) < 0)
    {
        printf("Cannot unlink file\n");
        return FALSE;
    }

    if (player.stats.s_hpt <= 0)
    {
        printf("\n\"He's dead, Jim\"\n");
        return FALSE;
    }

    strcpy(file_name, file);
    displayMessage("Welcome back");
    playit();
    /*NOTREACHED*/
    return(0);
}

/*
 * encwrite:
 *        Perform an encrypted write
 */

size_t encwrite(const void *start, size_t size, FILE *outf)
{
    return fwrite(start, 1, size, outf);
}

/*
 * encread:
 *        Perform an encrypted read
 */
size_t encread(void *start, size_t size, FILE *inf)
{
    return fread(start, 1, size, inf);
}

static char scoreline[100];
/*
 * read_scrore
 *        Read in the score file
 */
void
rd_score(SCORE *top_ten)
{
    unsigned int i;

        if (scoreboard == NULL)
                return;

        rewind(scoreboard); 

        for(i = 0; i < numscores; i++)
    {
        encread(top_ten[i].sc_name, MAXSTR, scoreboard);
        encread(scoreline, 100, scoreboard);
        sscanf(scoreline, " %u %d %u %hu %d %x \n",
            &top_ten[i].sc_uid, &top_ten[i].sc_score,
            &top_ten[i].sc_flags, &top_ten[i].sc_monster,
            &top_ten[i].sc_level, &top_ten[i].sc_time);
    }

        rewind(scoreboard); 
}

/*
 * write_scrore
 *        Read in the score file
 */
void
wr_score(SCORE *top_ten)
{
    unsigned int i;

    if (scoreboard == NULL)
        return;

    rewind(scoreboard);

    for(i = 0; i < numscores; i++)
    {
          memset(scoreline,0,100);
          encwrite(top_ten[i].sc_name, MAXSTR, scoreboard);
          sprintf(scoreline, " %u %d %u %hu %d %x \n",
              top_ten[i].sc_uid, top_ten[i].sc_score,
              top_ten[i].sc_flags, top_ten[i].sc_monster,
              top_ten[i].sc_level, top_ten[i].sc_time);
          encwrite(scoreline,100,scoreboard);
    }

        rewind(scoreboard); 
}
