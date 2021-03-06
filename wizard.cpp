/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 *
 * @(#)wizard.c        4.30 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rogue.h"
#include "areas.h"

/*
 * whatis:
 *        What a certin object is
 */

void whatis(bool insist, int type)
{
    ItemThing *obj;

    if (player.pack.size() == 0)
    {
        msg("you don't have anything in your pack to identify");
        return;
    }

    for (;;)
    {
        obj = get_item("identify", type);
        if (insist)
        {
            if (n_objs == 0)
                return;
            else if (obj == nullptr)
                msg("you must identify something");
            else if (type && obj->type != type &&
               !(type == R_OR_S && (obj->type == RING || obj->type == STICK)) )
                    msg("you must identify a %s", type_name(type));
            else
                break;
        }
        else
            break;
    }

    if (obj == nullptr)
        return;

    switch (obj->type)
    {
        case SCROLL:
            set_know(obj, scr_info);
        when POTION:
            set_know(obj, pot_info);
        when STICK:
            set_know(obj, ws_info);
        when WEAPON:
        case ARMOR:
            obj->flags |= ISKNOW;
        when RING:
            set_know(obj, ring_info);
    }
    msg(inv_name(obj, false));
}

/*
 * set_know:
 *        Set things up when we really know what a thing is
 */

void
set_know(ItemThing *obj, struct obj_info *info)
{
    char **guess;

    info[obj->which].oi_know = true;
    obj->flags |= ISKNOW;
    guess = &info[obj->which].oi_guess;
    if (*guess)
    {
        free(*guess);
        *guess = nullptr;
    }
}

/*
 * type_name:
 *        Return a pointer to the name of the type
 */
const char* type_name(int type)
{
    struct h_list *hp;
    static struct h_list tlist[] = {
        {POTION, "potion",                false},
        {SCROLL, "scroll",                false},
        {FOOD,         "food",                false},
        {R_OR_S, "ring, wand or staff",        false},
        {RING,         "ring",                false},
        {STICK,         "wand or staff",        false},
        {WEAPON, "weapon",                false},
        {ARMOR,         "suit of armor",        false},
    };

    for (hp = tlist; hp->h_ch; hp++)
        if (type == hp->h_ch)
            return hp->h_desc;
    /* NOTREACHED */
    return(0);
}


/*
 * telport:
 *        Bamf the hero someplace else
 */

void teleport()
{
    setMapDisplay(hero.x, hero.y, char_at_place(hero.x, hero.y));
    hero = Area::random_position(Area::ForMonster);
    look(true);
    setMapDisplay(hero.x, hero.y, PLAYER);
    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Flytrap
     */
    if (on(player, ISHELD)) {
        player.flags &= ~ISHELD;
        vf_hit = 0;
        strcpy(monsters['F'-'A'].m_stats.s_dmg, "000x0");
    }
    no_move = 0;
    count = 0;
    running = false;
    flush_type();
}
