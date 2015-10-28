/*
 * new_level:
 *        Dig and draw a new level
 *
 * @(#)new_level.c        4.38 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <string.h>
#include "rogue.h"

#define TREAS_ROOM 20        /* one chance in TREAS_ROOM for a treasure room */
#define MAXTREAS 10        /* maximum number of treasures in a treasure room */
#define MINTREAS 2        /* minimum number of treasures in a treasure room */

void
new_level()
{
    MONSTER_THING *tp;
    PLACE *pp;
    char *sp;
    int i;

    player.flags &= ~ISHELD;        /* unhold when you go down just in case */
    if (level > max_level)
        max_level = level;
    /*
     * Clean things off from last level
     */
    for (pp = places; pp < &places[MAXCOLS*MAXLINES]; pp++)
    {
        pp->p_ch = ' ';
        pp->p_flags = F_REAL;
        pp->p_monst = NULL;
    }
    clearMapDisplay();
    /*
     * Free up the monsters on the last level
     */
    for (tp = mlist; tp != NULL; tp = tp->next)
        free_list(tp->pack);
    free_list(mlist);
    /*
     * Throw away stuff left on the previous level (if anything)
     */
    free_list(lvl_obj);
    do_rooms();                                /* Draw rooms */
    do_passages();                        /* Draw passages */
    no_food++;
    put_things();                        /* Place objects (if any) */
    /*
     * Place the traps
     */
    if (rnd(10) < level)
    {
        ntraps = rnd(level / 4) + 1;
        if (ntraps > MAXTRAPS)
            ntraps = MAXTRAPS;
        i = ntraps;
        while (i--)
        {
            /*
             * not only wouldn't it be NICE to have traps in mazes
             * (not that we care about being nice), since the trap
             * number is stored where the passage number is, we
             * can't actually do it.
             */
            do
            {
                find_floor((struct room *) NULL, &stairs, FALSE, FALSE);
            } while (chat(stairs.y, stairs.x) != FLOOR);
            sp = &flat(stairs.y, stairs.x);
            *sp &= ~F_REAL;
            *sp |= rnd(NTRAPS);
        }
    }
    /*
     * Place the staircase down.
     */
    find_floor((struct room *) NULL, &stairs, FALSE, FALSE);
    chat(stairs.y, stairs.x) = STAIRS;
    seenstairs = FALSE;

    for (tp = mlist; tp != NULL; tp = tp->next)
        tp->room = roomin(&tp->pos);

    find_floor((struct room *) NULL, &hero, FALSE, TRUE);
    enter_room(&hero);
    setMapDisplay(hero.x, hero.y, PLAYER);
    if (on(player, SEEMONST))
        turn_see(FALSE);
    if (on(player, ISHALU))
        visuals(0);
}

/*
 * rnd_room:
 *        Pick a room that is really there
 */
int
rnd_room()
{
    int rm;

    do
    {
        rm = rnd(MAXROOMS);
    } while (rooms[rm].r_flags & ISGONE);
    return rm;
}

/*
 * put_things:
 *        Put potions and scrolls on this level
 */

void
put_things()
{
    int i;
    ITEM_THING *obj;

    /*
     * Once you have found the amulet, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (amulet && level < max_level)
        return;
    /*
     * check for treasure rooms, and if so, put it in.
     */
    if (rnd(TREAS_ROOM) == 0)
        treas_room();
    /*
     * Do MAXOBJ attempts to put things on a level
     */
    for (i = 0; i < MAXOBJ; i++)
        if (rnd(100) < 36)
        {
            /*
             * Pick a new object and link it in the list
             */
            obj = new_thing();
            attach(lvl_obj, obj);
            /*
             * Put it somewhere
             */
            find_floor((struct room *) NULL, &obj->pos, FALSE, FALSE);
            chat(obj->pos.y, obj->pos.x) = (char) obj->type;
        }
    /*
     * If he is really deep in the dungeon and he hasn't found the
     * amulet yet, put it somewhere on the ground
     */
    if (level >= AMULETLEVEL && !amulet)
    {
        obj = new_item();
        attach(lvl_obj, obj);
        obj->hplus = 0;
        obj->dplus = 0;
        strncpy(obj->damage,"0x0",sizeof(obj->damage));
        strncpy(obj->hurldmg,"0x0",sizeof(obj->hurldmg));
        obj->arm = 11;
        obj->type = AMULET;
        /*
         * Put it somewhere
         */
        find_floor((struct room *) NULL, &obj->pos, FALSE, FALSE);
        chat(obj->pos.y, obj->pos.x) = AMULET;
    }
}

/*
 * treas_room:
 *        Add a treasure room
 */
#define MAXTRIES 10        /* max number of tries to put down a monster */


void
treas_room()
{
    int nm;
    ITEM_THING *tp;
    MONSTER_THING *monster_p;
    struct room *rp;
    int spots, num_monst;
    static coord mp;

    rp = &rooms[rnd_room()];
    spots = (rp->r_max.y - 2) * (rp->r_max.x - 2) - MINTREAS;
    if (spots > (MAXTREAS - MINTREAS))
        spots = (MAXTREAS - MINTREAS);
    num_monst = nm = rnd(spots) + MINTREAS;
    while (nm--)
    {
        find_floor(rp, &mp, 2 * MAXTRIES, FALSE);
        tp = new_thing();
        tp->pos = mp;
        attach(lvl_obj, tp);
        chat(mp.y, mp.x) = (char) tp->type;
    }

    /*
     * fill up room with monsters from the next level down
     */

    if ((nm = rnd(spots) + MINTREAS) < num_monst + 2)
        nm = num_monst + 2;
    spots = (rp->r_max.y - 2) * (rp->r_max.x - 2);
    if (nm > spots)
        nm = spots;
    level++;
    while (nm--)
    {
        spots = 0;
        if (find_floor(rp, &mp, MAXTRIES, TRUE))
        {
            monster_p = new_monster_thing();
            new_monster(monster_p, randmonster(FALSE), &mp);
            monster_p->flags |= ISMEAN;        /* no sloughers in THIS room */
            give_pack(monster_p);
        }
    }
    level--;
}
