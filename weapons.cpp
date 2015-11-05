/*
 * Functions for dealing with problems brought about by weapons
 *
 * @(#)weapons.c        4.34 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <string.h>
#include <ctype.h>
#include "rogue.h"

#define NO_WEAPON -1

int group = 2;

static struct init_weaps {
    const char *iw_dam;        /* Damage when wielded (or thrown damage when bow required but not using a bow) */
    const char *iw_hrl;        /* Damage when thrown (or fired from a bow) */
    int iw_launch;        /* Launching weapon */
    int iw_flags;        /* Miscellaneous flags */
} init_dam[MAXWEAPONS] = {
    { "2x4",        "1x3",        NO_WEAPON,        0,                },        /* Mace */
    { "3x4",        "1x2",        NO_WEAPON,        0,                },        /* Long sword */
    { "1x1",        "1x1",        NO_WEAPON,        0,                },        /* Bow */
    { "1x1",        "2x3",        BOW,              ISMANY|ISMISL,    },        /* Arrow */
    { "1x6",        "1x4",        NO_WEAPON,        ISMANY|ISMISL,    },        /* Dagger */
    { "4x4",        "1x2",        NO_WEAPON,        0,                },        /* 2h sword */
    { "1x1",        "1x3",        NO_WEAPON,        ISMANY|ISMISL,    },        /* Dart */
    { "1x2",        "2x4",        NO_WEAPON,        ISMANY|ISMISL,    },        /* Shuriken */
    { "2x3",        "1x6",        NO_WEAPON,        ISMISL,           },        /* Spear */
};

/*
 * missile:
 *        Fire a missile in a given direction
 */

void
missile(int ydelta, int xdelta)
{
    ItemThing *obj;

    /*
     * Get which thing we are hurling
     */
    if ((obj = get_item("throw", WEAPON)) == NULL)
        return;
    if (!dropcheck(obj) || is_current(obj))
        return;
    obj = leave_pack(obj, TRUE, FALSE);
    do_motion(obj, ydelta, xdelta);
    /*
     * AHA! Here it has hit something.  If it is a wall or a door,
     * or if it misses (combat) the monster, put it on the floor
     */
    if (monster_at(obj->pos.x, obj->pos.y) == NULL || !hit_monster(unc(obj->pos), obj))
        fall(obj, TRUE);
}

/*
 * do_motion:
 *        Do the actual motion on the screen done by an object traveling
 *        across the room
 */

void do_motion(ItemThing *obj, int ydelta, int xdelta)
{
    /*
     * Come fly with us ...
     */
    obj->pos = hero;
    for (;;)
    {
        /*
         * Erase the old one
         */
        if (!ce(obj->pos, hero) && cansee(unc(obj->pos)) && !terse)
        {
            setMapDisplay(obj->pos.x, obj->pos.y, char_at_place(obj->pos.x, obj->pos.y));
        }
        /*
         * Get the new position
         */
        obj->pos.y += ydelta;
        obj->pos.x += xdelta;
        if (step_ok(char_at(obj->pos.x, obj->pos.y)) && char_at(obj->pos.x, obj->pos.y) != DOOR && !monster_at(obj->pos.x, obj->pos.y))
        {
            /*
             * It hasn't hit anything yet, so display it
             * If it alright.
             */
            if (cansee(unc(obj->pos)) && !terse)
            {
                setMapDisplay(obj->pos.x, obj->pos.y, obj->type);
                refreshMap();
                animationDelay();
            }
            continue;
        }
        break;
    }
}

/*
 * fall:
 *        Drop an item someplace around here.
 */

void
fall(ItemThing *obj, bool pr)
{
    static coord fpos;

    if (fallpos(&obj->pos, &fpos))
    {
        item_at(fpos.x, fpos.y) = obj;
        obj->pos = fpos;
        if (cansee(fpos.y, fpos.x))
        {
            setMapDisplay(fpos.x, fpos.y, char_at_place(fpos.x, fpos.y));
        }
        lvl_obj.push_front(obj);
        return;
    }
    if (pr)
    {
        if (has_hit)
        {
            endmsg();
            has_hit = FALSE;
        }
        msg("the %s vanishes as it hits the ground",
            weap_info[obj->which].oi_name);
    }
    delete obj;
}

/*
 * init_weapon:
 *        Set up the initial goodies for a weapon
 */

void
init_weapon(ItemThing *weap, int which)
{
    struct init_weaps *iwp;

    weap->type = WEAPON;
    weap->which = which;
    iwp = &init_dam[which];
    strncpy(weap->damage, iwp->iw_dam, sizeof(weap->damage));
    strncpy(weap->hurldmg,iwp->iw_hrl, sizeof(weap->hurldmg));
    weap->launch = iwp->iw_launch;
    weap->flags = iwp->iw_flags;
    weap->hplus = 0;
    weap->dplus = 0;
    if (which == DAGGER)
    {
        weap->count = rnd(4) + 2;
        weap->group = group++;
    }
    else if (weap->flags & ISMANY)
    {
        weap->count = rnd(8) + 8;
        weap->group = group++;
    }
    else
    {
        weap->count = 1;
        weap->group = 0;
    }
}

/*
 * hit_monster:
 *        Does the missile hit the monster?
 */
int
hit_monster(int y, int x, ItemThing *obj)
{
    static coord mp;

    mp.y = y;
    mp.x = x;
    return fight(&mp, obj, TRUE);
}

/*
 * num:
 *        Figure out the plus number for armor/weapons
 */
const char* num(int n1, int n2, char type)
{
    static char numbuf[10];

    sprintf(numbuf, n1 < 0 ? "%d" : "+%d", n1);
    if (type == WEAPON)
        sprintf(&numbuf[strlen(numbuf)], n2 < 0 ? ",%d" : ",+%d", n2);
    return numbuf;
}

/*
 * wield:
 *        Pull out a certain weapon
 */

void
wield()
{
    ItemThing *obj, *oweapon;
    const char *sp;

    oweapon = cur_weapon;
    if (!dropcheck(cur_weapon))
    {
        cur_weapon = oweapon;
        return;
    }
    cur_weapon = oweapon;
    if ((obj = get_item("wield", WEAPON)) == NULL)
    {
bad:
        after = FALSE;
        return;
    }

    if (obj->type == ARMOR)
    {
        msg("you can't wield armor");
        goto bad;
    }
    if (is_current(obj))
        goto bad;

    sp = inv_name(obj, TRUE);
    cur_weapon = obj;
    if (!terse)
        addmsg("you are now ");
    msg("wielding %s (%c)", sp, obj->packch);
}

/*
 * fallpos:
 *        Pick a random position around the give (y, x) coordinates
 */
bool fallpos(coord *pos, coord *newpos)
{
    int y, x, cnt;

    cnt = 0;
    for (y = pos->y - 1; y <= pos->y + 1; y++)
        for (x = pos->x - 1; x <= pos->x + 1; x++)
        {
            /*
             * check to make certain the spot is empty, if it is,
             * put the object there, set it in the level list
             * and re-draw the room if he can see it
             */
            if (y == hero.y && x == hero.x)
                continue;
            if (!item_at(y, x) && rnd(++cnt) == 0 && step_ok(char_at(x, y)))
            {
                newpos->y = y;
                newpos->x = x;
            }
        }
    return (bool)(cnt != 0);
}
