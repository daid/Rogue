/*
 * File for the fun ends
 * Death or a total win
 *
 * @(#)rip.c        4.57 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include "rogue.h"
#include "score.h"
/**
The 1980 date is a wink to the first release date of rogue.

 12345678901234567890123456789012
1          ___________
2         /  REST IN  \
3        /    PEACE    \
4       /               \
5       |               |
6       |  killed by a  |
7       |               |
8       |      1980     |
9      *|    *     *    | *
0______)/\_//(\//\)/\/\/|_)______
 12345678901234567890123456789012
*/
static const char *rip[] = {
"          ___________",
"         /  REST IN  \\",
"        /    PEACE    \\",
"       /               \\",
"       |               |",
"       |  killed by a  |",
"       |               |",
"       |      1980     |",
"      *|    *     *    | *",
"______)/\\_//(\\//\\)/\\/\\/|_)_",
    0
};

/*
 * score:
 *        Figure score and post it.
 */
/* VARARGS2 */

void score(int amount, int flags, char monst)
{
    SCORE *scp;
    int i;
    SCORE *sc2;
    SCORE *top_ten, *endp;
    unsigned int uid;
    static const char *reason[] = {
        "killed",
        "quit",
        "A total winner",
        "killed with Amulet"
    };

    start_score();

    if (flags >= 0)
    {
        //mvaddstr(LINES - 1, 0 , "[Press return to continue]");
        //md_readchar();
    }

    top_ten = (SCORE *) malloc(numscores * sizeof (SCORE));
    endp = &top_ten[numscores];
    for (scp = top_ten; scp < endp; scp++)
    {
        scp->sc_score = 0;
        for (i = 0; i < MAXSTR; i++)
            scp->sc_name[i] = 0;
        scp->sc_flags = 0;
        scp->sc_level = 0;
        scp->sc_monster = 0;
        scp->sc_uid = 0;
    }

    rd_score(top_ten);
    /*
     * Insert her in list if need be
     */
    sc2 = NULL;
    if (amount > 0)
    {
        uid = 0;
        for (scp = top_ten; scp < endp; scp++)
            if (amount > scp->sc_score)
                break;
            else if (!allscore &&        /* only one score per nowin uid */
                flags != 2 && scp->sc_uid == uid && scp->sc_flags != 2)
                    scp = endp;
        if (scp < endp)
        {
            if (flags != 2 && !allscore)
            {
                for (sc2 = scp; sc2 < endp; sc2++)
                {
                    if (sc2->sc_uid == uid && sc2->sc_flags != 2)
                        break;
                }
                if (sc2 >= endp)
                    sc2 = endp - 1;
            }
            else
                sc2 = endp - 1;
            while (sc2 > scp)
            {
                *sc2 = sc2[-1];
                sc2--;
            }
            scp->sc_score = amount;
            strncpy(scp->sc_name, "Jedi", MAXSTR);
            scp->sc_flags = flags;
            if (flags == 2)
                scp->sc_level = max_level;
            else
                scp->sc_level = level;
            scp->sc_monster = monst;
            scp->sc_uid = uid;
            scp->sc_time = 0;
            sc2 = scp;
        }
    }
    /*
     * Print the list
     */
    startDisplayOfStringList();
    displayStringListItem("   Score Level");
    for (scp = top_ten; scp < endp; scp++)
    {
        if (scp->sc_score) {
            if (scp->sc_flags == 0 || scp->sc_flags == 3)
                displayStringListItem("%2d %5d %s on level %d by %s", (int) (scp - top_ten + 1), scp->sc_score, reason[scp->sc_flags], scp->sc_level, killname((char) scp->sc_monster, TRUE));
            else
                displayStringListItem("%2d %5d %s on level %d", (int) (scp - top_ten + 1), scp->sc_score, reason[scp->sc_flags], scp->sc_level);
        }
        else
            break;
    }
    finishDisplayOfStringList();
    /*
     * Update the list file
     */
    if (sc2 != NULL)
    {
        wr_score(top_ten);
    }
}

/*
 * death:
 *        Do something really fun when he dies
 */

void death(char monst)
{
    const char **dp, *killer;

    purse -= purse / 10;
    clearMapDisplay();
    setMapViewTarget(16, 5);
    killer = killname(monst, FALSE);
    if (!tombstone)
    {
        msg("Killed by %s%s%s with %d gold", (monst != 's' && monst != 'h') ? "a" : "", (monst != 's' && monst != 'h') ? vowelstr(killer) : "", killer, purse);
    }
    else
    {
        dp = rip;
        int x = 0;
        int y = 0;
        while (*dp)
        {
            const char* str = *dp++;
            x = 0;
            while(*str)
                setMapDisplay(x++, y, *str++);
            y++;
        }

        if (monst == 's' || monst == 'h')
            setMapDisplay(20, 5, ' ');
        else
            setMapDisplay(21, 5, vowelstr(killer)[0]);

        char buffer[32];
        sprintf(buffer, "%14d gold", purse);
        setStatusLine(buffer);
        x = (32 - strlen(killer)) / 2;
        while(*killer)
            setMapDisplay(x++, 6, *killer++);
    }
    refreshMap();
    md_readchar();

    startDisplayOfStringList();
    displayStringListItem("Your items:");
    for (ItemThing* obj : player.pack)
    {
        obj->flags |= ISKNOW;
        switch (obj->type)
        {
            case FOOD:
            when WEAPON:
                weap_info[obj->which].oi_know = true;
            when ARMOR:
                arm_info[obj->which].oi_know = true;
            when SCROLL:
                scr_info[obj->which].oi_know = true;
            when POTION:
                pot_info[obj->which].oi_know = true;
            when RING:
                ring_info[obj->which].oi_know = true;
            when STICK:
                ws_info[obj->which].oi_know = true;
            when AMULET:
                break;
        }
        displayStringListItem("%c) %s", obj->packch, inv_name(obj, FALSE));
    }
    finishDisplayOfStringList();

    score(purse, amulet ? 3 : 0, monst);
    my_exit(0);
}

/*
 * total_winner:
 *        Code for a winner
 */

void total_winner()
{
    struct obj_info *op;
    int worth = 0;
    int oldpurse;

    clearMapDisplay();
    setMapViewTarget(16, 5);
    displayMessage("You made it!");
    displayMessage("Congratulations, you have made it to the light of day!");

    //addstr("\nYou have joined the elite ranks of those who have escaped the\n");
    //addstr("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
    //addstr("a great profit and are admitted to the Fighters' Guild.\n");
    
    startDisplayOfStringList();
    displayStringListItem("   Worth  Item");
    oldpurse = purse;
    for (ItemThing* obj : player.pack)
    {
        switch (obj->type)
        {
            case FOOD:
                worth = 2 * obj->count;
            when WEAPON:
                worth = weap_info[obj->which].oi_worth;
                worth *= 3 * (obj->hplus + obj->dplus) + obj->count;
                obj->flags |= ISKNOW;
            when ARMOR:
                worth = arm_info[obj->which].oi_worth;
                worth += (9 - obj->arm) * 100;
                worth += (10 * (a_class[obj->which] - obj->arm));
                obj->flags |= ISKNOW;
            when SCROLL:
                worth = scr_info[obj->which].oi_worth;
                worth *= obj->count;
                op = &scr_info[obj->which];
                if (!op->oi_know)
                    worth /= 2;
                op->oi_know = TRUE;
            when POTION:
                worth = pot_info[obj->which].oi_worth;
                worth *= obj->count;
                op = &pot_info[obj->which];
                if (!op->oi_know)
                    worth /= 2;
                op->oi_know = TRUE;
            when RING:
                op = &ring_info[obj->which];
                worth = op->oi_worth;
                if (obj->which == R_ADDSTR || obj->which == R_ADDDAM ||
                    obj->which == R_PROTECT || obj->which == R_ADDHIT)
                {
                        if (obj->arm > 0)
                            worth += obj->arm * 100;
                        else
                            worth = 10;
                }
                if (!(obj->flags & ISKNOW))
                    worth /= 2;
                obj->flags |= ISKNOW;
                op->oi_know = TRUE;
            when STICK:
                op = &ws_info[obj->which];
                worth = op->oi_worth;
                worth += 20 * obj->arm; //amount of charges is stored in arm field
                if (!(obj->flags & ISKNOW))
                    worth /= 2;
                obj->flags |= ISKNOW;
                op->oi_know = TRUE;
            when AMULET:
                worth = 1000;
        }
        if (worth < 0)
            worth = 0;
        displayStringListItem("%c) %5d  %s", obj->packch, worth, inv_name(obj, FALSE));
        purse += worth;
    }
    displayStringListItem("   %5d  Gold Pieces          ", oldpurse);
    finishDisplayOfStringList();
    score(purse, 2, ' ');
    my_exit(0);
}

/*
 * killname:
 *        Convert a code to a monster name
 */
const char* killname(char monst, bool doart)
{
    struct h_list *hp;
    const char *sp;
    bool article;
    static struct h_list nlist[] = {
        {'a',        "arrow",                TRUE},
        {'b',        "bolt",                 TRUE},
        {'d',        "dart",                 TRUE},
        {'h',        "hypothermia",          FALSE},
        {'s',        "starvation",           FALSE},
        {'z',        "exploding wand",       TRUE},
        {'\0'}
    };

    if (isupper(monst))
    {
        sp = monsters[monst-'A'].m_name;
        article = TRUE;
    }
    else
    {
        sp = "Wally the Wonder Badger";
        article = FALSE;
        for (hp = nlist; hp->h_ch; hp++)
            if (hp->h_ch == monst)
            {
                sp = hp->h_desc;
                article = hp->h_print;
                break;
            }
    }
    if (doart && article)
        sprintf(prbuf, "a%s ", vowelstr(sp));
    else
        prbuf[0] = '\0';
    strcat(prbuf, sp);
    return prbuf;
}

/*
 * death_monst:
 *        Return a monster appropriate for a random death.
 */
char
death_monst()
{
    static char poss[] =
    {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'h', 'd', 's',
        ' '        /* This is provided to generate the "Wally the Wonder Badger"
                   message for killer */
    };

    return poss[rnd(sizeof poss / sizeof (char))];
}
