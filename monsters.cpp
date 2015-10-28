/*
 * File with various monster functions in it
 *
 * @(#)monsters.c        4.46 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <assert.h>
#include <string.h>
#include "rogue.h"
#include <ctype.h>

/*
 * List of monsters in rough order of vorpalness
 */
static char lvl_mons[] =  {
    'K', 'E', 'B', 'S', 'H', 'I', 'R', 'O', 'Z', 'L', 'C', 'Q', 'A',
    'N', 'Y', 'F', 'T', 'W', 'P', 'X', 'U', 'M', 'V', 'G', 'J', 'D'
};

static char wand_mons[] = {
    'K', 'E', 'B', 'S', 'H',   0, 'R', 'O', 'Z',   0, 'C', 'Q', 'A',
      0, 'Y',   0, 'T', 'W', 'P',   0, 'U', 'M', 'V', 'G', 'J',   0
};

/*
 * randmonster:
 *        Pick a monster to show up.  The lower the level,
 *        the meaner the monster.
 */
char
randmonster(bool wander)
{
    int d;
    char *mons;

    mons = (wander ? wand_mons : lvl_mons);
    do
    {
        d = level + (rnd(10) - 6);
        if (d < 0)
            d = rnd(5);
        if (d > 25)
            d = rnd(5) + 21;
    } while (mons[d] == 0);
    return mons[d];
}

/*
 * new_monster:
 *        Pick a new monster and add it to the list
 */

void new_monster(MONSTER_THING *tp, char type, coord *cp)
{
    struct monster *mp;
    int lev_add;

    if ((lev_add = level - AMULETLEVEL) < 0)
        lev_add = 0;
    attach(mlist, tp);
    tp->type = type;
    tp->disguise = type;
    tp->pos = *cp;
    tp->oldch = getMapDisplay(cp->x, cp->y);
    tp->room = roomin(cp);
    moat(cp->y, cp->x) = tp;
    mp = &monsters[tp->type-'A'];
    tp->stats.s_lvl = mp->m_stats.s_lvl + lev_add;
    tp->stats.s_maxhp = tp->stats.s_hpt = roll(tp->stats.s_lvl, 8);
    tp->stats.s_arm = mp->m_stats.s_arm - lev_add;
    strcpy(tp->stats.s_dmg,mp->m_stats.s_dmg);
    tp->stats.s_str = mp->m_stats.s_str;
    tp->stats.s_exp = mp->m_stats.s_exp + lev_add * 10 + exp_add(tp);
    tp->flags = mp->m_flags;
    if (level > 29)
        tp->flags |= ISHASTE;
    tp->turn = TRUE;
    tp->pack = NULL;
    if (ISWEARING(R_AGGR))
        runto(cp);
    if (type == 'X')
        tp->disguise = rnd_thing();
}

/*
 * expadd:
 *        Experience to add for this monster's level/hit points
 */
int
exp_add(MONSTER_THING *tp)
{
    int mod;

    if (tp->stats.s_lvl == 1)
        mod = tp->stats.s_maxhp / 8;
    else
        mod = tp->stats.s_maxhp / 6;
    if (tp->stats.s_lvl > 9)
        mod *= 20;
    else if (tp->stats.s_lvl > 6)
        mod *= 4;
    return mod;
}

/*
 * wanderer:
 *        Create a new wandering monster and aim it at the player
 */

void
wanderer()
{
    MONSTER_THING *tp;
    static coord cp;

    tp = new_monster_thing();
    do
    {
        find_floor((struct room *) NULL, &cp, FALSE, TRUE);
    } while (roomin(&cp) == player.room);
    new_monster(tp, randmonster(TRUE), &cp);
    if (on(player, SEEMONST))
    {
        if (!on(player, ISHALU))
            setMapDisplay(cp.x, cp.y, tp->type | DISPLAY_INVERT);
        else
            setMapDisplay(cp.x, cp.y, (rnd(26) + 'A') | DISPLAY_INVERT);
    }
    runto(&tp->pos);
#ifdef MASTER
    if (wizard)
        msg("started a wandering %s", monsters[tp->type-'A'].m_name);
#endif
}

/*
 * wake_monster:
 *        What to do when the hero steps next to a monster
 */
MONSTER_THING* wake_monster(int y, int x)
{
    MONSTER_THING *tp;
    struct room *rp;
    int ch;
    const char *mname;

#ifdef MASTER
    if ((tp = moat(y, x)) == NULL)
        msg("can't find monster in wake_monster");
#else
    tp = moat(y, x);
    assert(tp != NULL);
#endif
    ch = tp->type;
    /*
     * Every time he sees mean monster, it might start chasing him
     */
    if (!on(*tp, ISRUN) && rnd(3) != 0 && on(*tp, ISMEAN) && !on(*tp, ISHELD)
        && !ISWEARING(R_STEALTH) && !on(player, ISLEVIT))
    {
        tp->dest = &hero;
        tp->flags |= ISRUN;
    }
    if (ch == 'M' && !on(player, ISBLIND) && !on(player, ISHALU)
        && !on(*tp, ISFOUND) && !on(*tp, ISCANC) && on(*tp, ISRUN))
    {
        rp = player.room;
        if ((rp != NULL && !(rp->r_flags & ISDARK))
            || dist(y, x, hero.y, hero.x) < LAMPDIST)
        {
            tp->flags |= ISFOUND;
            if (!save(VS_MAGIC))
            {
                if (on(player, ISHUH))
                    lengthen(unconfuse, spread(HUHDURATION));
                else
                    fuse(unconfuse, 0, spread(HUHDURATION), AFTER);
                player.flags |= ISHUH;
                mname = set_mname(tp);
                addmsg("%s", mname);
                if (strcmp(mname, "it") != 0)
                    addmsg("'");
                msg("s gaze has confused you");
            }
        }
    }
    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && !on(*tp, ISRUN))
    {
        tp->flags |= ISRUN;
        if (player.room->r_goldval)
            tp->dest = &player.room->r_gold;
        else
            tp->dest = &hero;
    }
    return tp;
}

/*
 * give_pack:
 *        Give a pack to a monster if it deserves one
 */

void
give_pack(MONSTER_THING *tp)
{
    if (level >= max_level && rnd(100) < monsters[tp->type-'A'].m_carry)
        attach(tp->pack, new_thing());
}

/*
 * save_throw:
 *        See if a creature save against something
 */
int
save_throw(int which, MONSTER_THING *tp)
{
    int need;

    need = 14 + which - tp->stats.s_lvl / 2;
    return (roll(1, 20) >= need);
}

/*
 * save:
 *        See if he saves against various nasty things
 */
int save(int which)
{
    if (which == VS_MAGIC)
    {
        if (ISRING(LEFT, R_PROTECT))
            which -= cur_ring[LEFT]->arm;
        if (ISRING(RIGHT, R_PROTECT))
            which -= cur_ring[RIGHT]->arm;
    }
    return save_throw(which, &player);
}
