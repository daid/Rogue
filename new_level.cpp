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
#include "areas.h"

#include "mapgen/classic/classic.h"
#include "mapgen/cave/cave.h"

void new_level()
{
    PLACE *pp;

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
        pp->p_monst = nullptr;
        pp->p_item = nullptr;
    }
    clearMapDisplay();
    /*
     * Free up the monsters on the last level
     */
    for(MonsterThing* tp : mlist)
    {
        for(ItemThing* obj : tp->pack)
            delete obj;
        delete tp;
    }
    mlist.clear();
    /*
     * Throw away stuff left on the previous level (if anything)
     */
    for(ItemThing* obj : lvl_obj)
        delete obj;
    lvl_obj.clear();
    
    //Clear out the area definitions
    areas.clear();

    //Increase the counter for how many levels we haven't spawned food.
    no_food++;
    
    //We've not seen the stairs yet (used during hallucinations)
    seenstairs = false;

    //Select a map generator and run it.
    if (level == 1) //First level is always a classic
        ClassicMapGenerator().generate();
    else if (rnd(3) == 0)   //1 out of 3 change of having a cave map.
        CaveMapGenerator().generate();
    else
        ClassicMapGenerator().generate();

    //The map generater placed the hero somewhere, display him, and redraw what's needed.
    setMapDisplay(hero.x, hero.y, PLAYER);
    if (on(player, SEEMONST))
        turn_see(FALSE);
    if (on(player, ISHALU))
        visuals(0);
}
