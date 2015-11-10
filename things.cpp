/*
 * Contains functions for dealing with things like potions, scrolls,
 * and other items.
 *
 * @(#)things.c        4.53 (Berkeley) 02/05/99
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

ItemThing::ItemThing()
{
    type = -1;                        /* What kind of object it is */
    pos.x = pos.y = 0;                /* Where it lives on the screen */
    launch = -1;                      /* What you need to launch it */
    packch = 0;                       /* What character it is in the pack */
    damage[0] = '\0';                 /* Damage if used like sword */
    hurldmg[0] = '\0';                /* Damage if thrown */
    count = 0;                        /* count for plural objects */
    which = -1;                       /* Which object of a type it is */
    hplus = 0;                        /* Plusses to hit */
    dplus = 0;                        /* Plusses to damage */
    arm = 0;                          /* Armor protection/charge count/gold amount */
    flags = 0;                        /* information about objects */
    group = 0;                        /* group number for this object */
    label = nullptr;                  /* Label for object */
}
/*
 * inv_name:
 *        Return the name of something as it would appear in an
 *        inventory.
 */
const char * inv_name(ItemThing *obj, bool drop)
{
    char *pb;
    struct obj_info *op;
    const char *sp;
    int which;

    pb = prbuf;
    which = obj->which;
    switch (obj->type)
    {
        case POTION:
            nameit(obj, "potion", p_colors[which], &pot_info[which], nullstr);
        when RING:
            nameit(obj, "ring", r_stones[which], &ring_info[which], ring_num);
        when STICK:
            nameit(obj, ws_type[which], ws_made[which], &ws_info[which], charge_str);
        when SCROLL:
            if (obj->count == 1)
            {
                strcpy(pb, "A scroll ");
                pb = &prbuf[9];
            }
            else
            {
                sprintf(pb, "%d scrolls ", obj->count);
                pb = &prbuf[strlen(prbuf)];
            }
            op = &scr_info[which];
            if (op->oi_know)
                sprintf(pb, "of %s", op->oi_name);
            else if (op->oi_guess)
                sprintf(pb, "called %s", op->oi_guess);
            else
                sprintf(pb, "titled '%s'", s_names[which]);
        when FOOD:
            if (which == 1)
                if (obj->count == 1)
                    sprintf(pb, "A%s %s", vowelstr(fruit), fruit);
                else
                    sprintf(pb, "%d %ss", obj->count, fruit);
            else
                if (obj->count == 1)
                    strcpy(pb, "Some food");
                else
                    sprintf(pb, "%d rations of food", obj->count);
        when WEAPON:
            sp = weap_info[which].oi_name;
            if (obj->count > 1)
                sprintf(pb, "%d ", obj->count);
            else
                sprintf(pb, "A%s ", vowelstr(sp));
            pb = &prbuf[strlen(prbuf)];
            if (obj->flags & ISKNOW)
                sprintf(pb, "%s %s", num(obj->hplus,obj->dplus,WEAPON), sp);
            else
                sprintf(pb, "%s", sp);
            if (obj->count > 1)
                strcat(pb, "s");
            if (obj->label != NULL)
            {
                pb = &prbuf[strlen(prbuf)];
                sprintf(pb, " called %s", obj->label);
            }
        when ARMOR:
            sp = arm_info[which].oi_name;
            if (obj->flags & ISKNOW)
            {
                sprintf(pb, "%s %s [",
                    num(a_class[which] - obj->arm, 0, ARMOR), sp);
                if (!terse)
                    strcat(pb, "protection ");
                pb = &prbuf[strlen(prbuf)];
                sprintf(pb, "%d]", 10 - obj->arm);
            }
            else
                sprintf(pb, "%s", sp);
            if (obj->label != NULL)
            {
                pb = &prbuf[strlen(prbuf)];
                sprintf(pb, " called %s", obj->label);
            }
        when AMULET:
            strcpy(pb, "The Amulet of Yendor");
        when GOLD:
            sprintf(prbuf, "%d Gold pieces", obj->arm); //gold value is stored in arm field.
    }
    if (inv_describe)
    {
        if (obj == cur_armor)
            strcat(pb, " (being worn)");
        if (obj == cur_weapon)
            strcat(pb, " (weapon in hand)");
        if (obj == cur_ring[LEFT])
            strcat(pb, " (on left hand)");
        else if (obj == cur_ring[RIGHT])
            strcat(pb, " (on right hand)");
    }
    if (drop && isupper(prbuf[0]))
        prbuf[0] = (char) tolower(prbuf[0]);
    else if (!drop && islower(*prbuf))
        *prbuf = (char) toupper(*prbuf);
    prbuf[MAXSTR-1] = '\0';
    return prbuf;
}

/*
 * drop:
 *        Put something down
 */

void drop()
{
    ItemThing *obj;

    if (item_at(hero.x, hero.y))
    {
        after = false;
        msg("there is something there already");
        return;
    }
    if ((obj = get_item("drop", 0)) == NULL)
        return;
    if (!dropcheck(obj))
        return;
    obj = leave_pack(obj, TRUE, (bool)!ISMULT(obj->type));
    /*
     * Link it into the level object list
     */
    lvl_obj.push_front(obj);
    item_at(hero.x, hero.y) = obj;
    obj->pos = hero;
    if (obj->type == AMULET)
        amulet = FALSE;
    msg("dropped %s", inv_name(obj, TRUE));
}

/*
 * dropcheck:
 *        Do special checks for dropping or unweilding|unwearing|unringing
 */
bool dropcheck(ItemThing *obj)
{
    if (obj == NULL)
        return TRUE;
    if (obj != cur_armor && obj != cur_weapon && obj != cur_ring[LEFT] && obj != cur_ring[RIGHT])
        return TRUE;
    if (obj->flags & ISCURSED)
    {
        msg("you can't.\nIt appears to be cursed");
        return FALSE;
    }
    if (obj == cur_weapon)
    {
        cur_weapon = NULL;
    }
    else if (obj == cur_armor)
    {
        waste_time();
        cur_armor = NULL;
    }
    else
    {
        cur_ring[obj == cur_ring[LEFT] ? LEFT : RIGHT] = NULL;
        switch(obj->which)
        {
            case R_ADDSTR:
                chg_str(-obj->arm);
                break;
            case R_SEEINVIS:
                unsee(0);
                extinguish(unsee);
                break;
        }
    }
    return TRUE;
}

/*
 * new_thing:
 *        Return a new thing
 */
ItemThing* new_thing()
{
    ItemThing *cur;
    int r;

    cur = new ItemThing();
    cur->hplus = 0;
    cur->dplus = 0;
    strncpy(cur->damage, "0x0", sizeof(cur->damage));
    strncpy(cur->hurldmg, "0x0", sizeof(cur->hurldmg));
    cur->arm = 11;
    cur->count = 1;
    cur->group = 0;
    cur->flags = 0;
    /*
     * Decide what kind of object it will be
     * If we haven't had food for a while, let it be food.
     */
    switch (no_food > 3 ? 2 : pick_one(things, NUMITEMTYPES))
    {
        case 0:
            cur->type = POTION;
            cur->which = pick_one(pot_info, MAXPOTIONS);
        when 1:
            cur->type = SCROLL;
            if (rnd(100) < std::max(0, 5 - level) * 10)
            {
                cur->which = S_HINT;
            }else{
                cur->which = pick_one(scr_info, MAXSCROLLS);
            }
        when 2:
            cur->type = FOOD;
            no_food = 0;
            if (rnd(10) != 0)
                cur->which = 0;
            else
                cur->which = 1;
        when 3:
            init_weapon(cur, pick_one(weap_info, MAXWEAPONS));
            if ((r = rnd(100)) < 10)
            {
                cur->flags |= ISCURSED;
                cur->hplus -= rnd(3) + 1;
            }
            else if (r < 15)
                cur->hplus += rnd(3) + 1;
        when 4:
            cur->type = ARMOR;
            cur->which = pick_one(arm_info, MAXARMORS);
            cur->arm = a_class[cur->which];
            if ((r = rnd(100)) < 20)
            {
                cur->flags |= ISCURSED;
                cur->arm += rnd(3) + 1;
            }
            else if (r < 28)
                cur->arm -= rnd(3) + 1;
        when 5:
            cur->type = RING;
            cur->which = pick_one(ring_info, MAXRINGS);
            switch (cur->which)
            {
                case R_ADDSTR:
                case R_PROTECT:
                case R_ADDHIT:
                case R_ADDDAM:
                    if ((cur->arm = rnd(3)) == 0)
                    {
                        cur->arm = -1;
                        cur->flags |= ISCURSED;
                    }
                when R_AGGR:
                case R_TELEPORT:
                    cur->flags |= ISCURSED;
            }
        when 6:
            cur->type = STICK;
            cur->which = pick_one(ws_info, MAXSTICKS);
            fix_stick(cur);
    }
    return cur;
}

/*
 * pick_one:
 *        Pick an item out of a list of nitems possible objects
 */
int pick_one(struct obj_info *info, int nitems)
{
    struct obj_info *end;
    struct obj_info *start;
    int i;

    start = info;
    for (end = &info[nitems], i = rnd(100); info < end; info++)
        if (i < info->oi_prob)
            break;
    if (info == end)
    {
        info = start;
    }
    return (int)(info - start);
}

/*
 * discovered:
 *        list what the player has discovered in this game of a certain type
 */

void discovered()
{
    print_disc(POTION);
    print_disc(SCROLL);
    print_disc(RING);
    print_disc(STICK);
}

/*
 * print_disc:
 *        Print what we've discovered of type 'type'
 */

#define MAX4(a,b,c,d)        (a > b ? (a > c ? (a > d ? a : d) : (c > d ? c : d)) : (b > c ? (b > d ? b : d) : (c > d ? c : d)))


void print_disc(char type)
{
    struct obj_info *info = NULL;
    int i, maxnum = 0, num_found;
    static ItemThing obj;
    static int order[MAX4(MAXSCROLLS, MAXPOTIONS, MAXRINGS, MAXSTICKS)];

    switch (type)
    {
        case SCROLL:
            maxnum = MAXSCROLLS;
            info = scr_info;
            break;
        case POTION:
            maxnum = MAXPOTIONS;
            info = pot_info;
            break;
        case RING:
            maxnum = MAXRINGS;
            info = ring_info;
            break;
        case STICK:
            maxnum = MAXSTICKS;
            info = ws_info;
            break;
    }
    set_order(order, maxnum);
    obj.count = 1;
    obj.flags = 0;
    num_found = 0;
    
    startDisplayOfStringList();
    for (i = 0; i < maxnum; i++)
        if (info[order[i]].oi_know || info[order[i]].oi_guess)
        {
            obj.type = type;
            obj.which = order[i];
            displayStringListItem("%s", inv_name(&obj, FALSE));
            num_found++;
        }
    if (num_found == 0)
        displayStringListItem("%s", nothing(type));
    finishDisplayOfStringList();
}

/*
 * set_order:
 *        Set up order for list
 */

void set_order(int *order, int numthings)
{
    int i, r, t;

    for (i = 0; i< numthings; i++)
        order[i] = i;

    for (i = numthings; i > 0; i--)
    {
        r = rnd(i);
        t = order[i - 1];
        order[i - 1] = order[r];
        order[r] = t;
    }
}

/*
 * nothing:
 *        Set up prbuf so that message for "nothing found" is there
 */
const char * nothing(char type)
{
    char *sp;
    const char *tystr = NULL;

    if (terse)
        sprintf(prbuf, "Nothing");
    else
        sprintf(prbuf, "Haven't discovered anything");
    if (type != '*')
    {
        sp = &prbuf[strlen(prbuf)];
        switch (type)
        {
            case POTION: tystr = "potion";
            when SCROLL: tystr = "scroll";
            when RING: tystr = "ring";
            when STICK: tystr = "wand";
        }
        sprintf(sp, " about any %ss", tystr);
    }
    return prbuf;
}

/*
 * nameit:
 *        Give the proper name to a potion, stick, or ring
 */

void nameit(ItemThing *obj, const char *type, const char *which, struct obj_info *op, const char *(*prfunc)(ItemThing *))
{
    char *pb;

    if (op->oi_know || op->oi_guess)
    {
        if (obj->count == 1)
            sprintf(prbuf, "A %s ", type);
        else
            sprintf(prbuf, "%d %ss ", obj->count, type);
        pb = &prbuf[strlen(prbuf)];
        if (op->oi_know)
            sprintf(pb, "of %s%s(%s)", op->oi_name, (*prfunc)(obj), which);
        else if (op->oi_guess)
            sprintf(pb, "called %s%s(%s)", op->oi_guess, (*prfunc)(obj), which);
    }
    else if (obj->count == 1)
        sprintf(prbuf, "A%s %s %s", vowelstr(which), which, type);
    else
        sprintf(prbuf, "%d %s %ss", obj->count, which, type);
}

/*
 * nullstr:
 *        Return a pointer to a null-length string
 */
const char * nullstr(ItemThing *ignored)
{
    return "";
}
