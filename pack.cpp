/*
 * Routines to deal with the pack
 *
 * @(#)pack.c        4.40 (Berkeley) 02/05/99
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

/*
 * add_pack:
 *        Pick up an object and add it to the pack.  If the argument is
 *        non-null use it as the linked_list pointer instead of gettting
 *        it off the ground.
 */

void add_pack(ItemThing *obj, bool silent)
{
    bool from_floor;

    from_floor = false;
    if (obj == nullptr)
    {
        if ((obj = item_at(hero.x, hero.y)) == nullptr)
            return;
        from_floor = true;
    }

    /*
     * Check for and deal with scare monster scrolls
     */
    if (obj->type == SCROLL && obj->which == S_SCARE)
    {
        if (obj->flags & ISFOUND)
        {
            lvl_obj.remove(obj);
            item_at(hero.x, hero.y) = nullptr;
            delete obj;
            msg("the scroll turns to dust as you pick it up");
            return;
        }
    }

    // Deal with the complexity of adding this item to the players inventory.
    bool placed = false;
    // First, check if this is a grouped item, and place it in the proper group. This does not take inventory space.
    for (ItemThing* op : player.pack)
    {
        if (op->type == obj->type && op->which == obj->which && op->group && op->group == obj->group)
        {
            //Call tke pack_room function, which removes the object from the floor (odd side behaviour)
            pack_room(from_floor, obj);
            
            op->count += obj->count;
            delete obj;
            obj = op;
            placed = true;
            break;
        }
    }
    
    if (!placed)
    {
        // First, check if there is room in your inventory. This adds the item to the "inpack" counter as side effect.
        if (!pack_room(from_floor, obj))
            return;
        
        // Next, check if we need to stack this item on an stack you already have
        for (ItemThing* op : player.pack)
        {
            if (op->type == obj->type && op->which == obj->which && ISMULT(op->type))
            {
                op->count += obj->count;
                delete obj;
                obj = op;
                placed = true;
                break;
            }
        }
        if (!placed)
        {
            // If it's not a stacked item, find a proper position to place this item.
            // We want to place it with the same "type" of objects. So the list is sorted on types.
            auto place_before = player.pack.end();
            for(auto it = player.pack.begin(); it != player.pack.end(); it++)
            {
                ItemThing* op = *it;
                if (op->type == obj->type)
                {
                    place_before = it;
                    place_before++;
                    if (op->which == obj->which && place_before != player.pack.end())
                    {
                        ItemThing* before = *place_before;
                        if (before->type != obj->type && before->which == obj->which)
                            break;
                    }
                }
            }
            player.pack.insert(place_before, obj);
            obj->packch = pack_char();
        }
    }
    
    obj->flags |= ISFOUND;

    /*
     * If this was the object of something's desire, that monster will
     * get mad and run at the hero.
     */
    for(MonsterThing* mp : mlist)
        if (mp->dest == &obj->pos)
            mp->dest = &hero;

    if (obj->type == AMULET)
        amulet = true;
    /*
     * Notify the user
     */
    if (!silent)
    {
        if (!terse)
            addmsg("you now have ");
        msg("%s (%c)", inv_name(obj, !terse), obj->packch);
    }
}

/*
 * pack_room:
 *        See if there's room in the pack.  If not, print out an
 *        appropriate message
 */
bool pack_room(bool from_floor, ItemThing *obj)
{
    if (pack_count() >= MAXPACK)
    {
        if (!terse)
            addmsg("there's ");
        addmsg("no room");
        if (!terse)
            addmsg(" in your pack");
        endmsg();
        if (from_floor)
            move_msg(obj);
        return false;
    }

    if (from_floor)
    {
        lvl_obj.remove(obj);
        item_at(hero.x, hero.y) = nullptr;
    }

    return true;
}

/*
 * leave_pack:
 *        take an item out of the pack
 */
ItemThing * leave_pack(ItemThing *obj, bool newobj, bool all)
{
    ItemThing *nobj;

    nobj = obj;
    if (obj->count > 1 && !all)
    {
        last_pick = obj;
        obj->count--;
        if (newobj)
        {
            nobj = new ItemThing();
            *nobj = *obj;
            nobj->count = 1;
        }
    }
    else
    {
        last_pick = nullptr;
        pack_used[obj->packch - 'a'] = false;
        player.pack.remove(obj);
    }
    return nobj;
}

/*
 * pack_char:
 *        Return the next unused pack character.
 */
char pack_char()
{
    bool *bp;

    for (bp = pack_used; *bp; bp++)
        continue;
    *bp = true;
    return (char)((int)(bp - pack_used) + 'a');
}

/*
 * pack_count:
 *      Return the amount of items in the pack.
 */
int pack_count()
{
    int count = 0;
    for(ItemThing* obj : player.pack)
    {
        if (obj->group)
            count++;    //Grouped items only count as a single item.
        else
            count += obj->count;
    }
    return count;
}

/*
 * inventory:
 *        List what is in the pack.  Return true if there is something of
 *        the given type.
 */
int inventory(int type)
{
    int ch;
    
    startDisplayOfStringList();
    n_objs = 0;
    for (ItemThing* list : player.pack)
    {
        if (type && type != list->type && !(type == CALLABLE &&
            list->type != FOOD && list->type != AMULET) &&
            !(type == R_OR_S && (list->type == RING || list->type == STICK)))
                continue;
        n_objs++;
        ch = displayStringListItem("%c) %s", list->packch, inv_name(list, false));
        if (ch == ESCAPE)
            return ch;
        for (ItemThing* list2 : player.pack)
        {
            if (type && type != list2->type && !(type == CALLABLE &&
                list2->type != FOOD && list2->type != AMULET) &&
                !(type == R_OR_S && (list2->type == RING || list2->type == STICK)))
                    continue;
            if (list2->packch == ch)
                return ch;
            if (list2 == list)
                break;
        }
    }
    ch = finishDisplayOfStringList();
    if (ch == ESCAPE)
        return ch;
    for (ItemThing* list : player.pack)
    {
        if (type && type != list->type && !(type == CALLABLE &&
            list->type != FOOD && list->type != AMULET) &&
            !(type == R_OR_S && (list->type == RING || list->type == STICK)))
                continue;
        if (list->packch == ch)
            return ch;
    }
    
    if (n_objs == 0)
    {
        if (terse)
            msg(type == 0 ? "empty handed" :
                            "nothing appropriate");
        else
            msg(type == 0 ? "you are empty handed" :
                            "you don't have anything appropriate");
        return ESCAPE;
    }

    return 0;
}

/*
 * pick_up:
 *        Add something to characters pack.
 */

void pick_up(int ch)
{
    ItemThing *obj;

    if (on(player, ISLEVIT))
        return;

    obj = item_at(hero.x, hero.y);
    if (move_on)
    {
        move_msg(obj);
    }
    else
    {
        //If a monster was going for this item, replace it's target with the hero.
        for(MonsterThing* mp : mlist)
        {
            if (mp->dest == &obj->pos)
                mp->dest = &hero;
        }
        
        switch (ch)
        {
            case GOLD:
                if (obj == nullptr)
                    return;
                money(obj->arm);//gold value is stored in "arm" field (used to be handled with an ugly define)
                lvl_obj.remove(obj);
                item_at(obj->pos.x, obj->pos.y) = nullptr;
                delete obj;
                break;
            default:
            case ARMOR:
            case POTION:
            case FOOD:
            case WEAPON:
            case SCROLL:        
            case AMULET:
            case RING:
            case STICK:
                add_pack(nullptr, false);
                break;
        }
    }
}

/*
 * move_msg:
 *        Print out the message if you are just moving onto an object
 */

void move_msg(ItemThing *obj)
{
    if (!terse)
        addmsg("you ");
    msg("moved onto %s", inv_name(obj, true));
}

/*
 * picky_inven:
 *        Allow player to inventory a single item
 */

void picky_inven()
{
    char mch;

    if (player.pack.size() == 0)
        msg("you aren't carrying anything");
    else if (player.pack.size() == 1)
        msg("a) %s", inv_name(player.pack.front(), false));
    else
    {
        mch = displayMessage(terse ? "item:" : "which item do you wish to inventory:");
        if (mch == ESCAPE)
        {
            return;
        }
        for (ItemThing* obj : player.pack)
            if (mch == obj->packch)
            {
                msg("%c) %s", mch, inv_name(obj, false));
                return;
            }
        msg("'%s' not in pack", getKeyName(mch));
    }
}

/*
 * get_item:
 *        Pick something out of a pack for a purpose
 */
ItemThing * get_item(const char *purpose, int type)
{
    int ch;

    if (player.pack.size() == 0)
        msg("you aren't carrying anything");
    else if (again)
        if (last_pick)
            return last_pick;
        else
            msg("you ran out");
    else
    {
        for (;;)
        {
            if (!terse)
                addmsg("which object do you want to ");
            addmsg(purpose);
            if (terse)
                addmsg(" what");
            ch = msg("? (* for list): ");
            /*
             * Give the poor player a chance to abort the command
             */
            if (ch == ESCAPE || ch == K_EXIT)
            {
                reset_last();
                after = false;
                return nullptr;
            }
            if (ch == '*')
            {
                ch = inventory(type);
                if (ch == ESCAPE)
                {
                    after = false;
                    return nullptr;
                }
                if (ch == 0)
                {
                    //No selection been made during the inventory call.
                    continue;
                }
            }
            ItemThing* obj = nullptr;
            for (ItemThing* o : player.pack)
            {
                if (o->packch == ch)
                {
                    obj = o;
                    break;
                }
            }
            if (obj == nullptr)
            {
                msg("'%s' is not a valid item", getKeyName(ch));
                continue;
            }
            else 
                return obj;
        }
    }
    return nullptr;
}

/*
 * money:
 *        Add or subtract gold from the pack
 */
void money(int value)
{
    purse += value;
    if (value > 0)
    {
        if (!terse)
            addmsg("you found ");
        msg("%d gold pieces", value);
    }
}

/*
 * reset_last:
 *        Reset the last command when the current one is aborted
 */
void reset_last()
{
    last_comm = l_last_comm;
    last_dir = l_last_dir;
    last_pick = l_last_pick;
}
