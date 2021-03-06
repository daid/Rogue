/*
 * Functions to implement the various sticks one might find
 * while wandering around the dungeon.
 *
 * @(#)sticks.c        4.39 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <vector>
#include <string.h>
#include <ctype.h>
#include "rogue.h"
#include "areas.h"

/*
 * fix_stick:
 *        Set up a new stick
 */

void
fix_stick(ItemThing *cur)
{
    if (strcmp(ws_type[cur->which], "staff") == 0)
        strncpy(cur->damage,"2x3",sizeof(cur->damage));
    else
        strncpy(cur->damage,"1x1",sizeof(cur->damage));
    strncpy(cur->hurldmg,"1x1",sizeof(cur->hurldmg));

    switch (cur->which)
    {
        case WS_LIGHT:
            cur->arm = rnd(10) + 10; //amount of charges is stored in arm field
        otherwise:
            cur->arm = rnd(5) + 3; //amount of charges is stored in arm field
    }
}

/*
 * do_zap:
 *        Perform a zap with a wand
 */

void do_zap()
{
    ItemThing *obj;
    MonsterThing *tp;
    int y, x;
    const char *name;
    char monster;
    static ItemThing bolt;

    if ((obj = get_item("zap with", STICK)) == nullptr)
        return;
    if (obj->type != STICK)
    {
        after = false;
        msg("you can't zap with that!");
        return;
    }
    if (!get_dir())
    {
        after = false;
        return;
    }
    if (obj->arm <= 0) //amount of charges is stored in the arm field.
    {
        //When trying to zap an empty wand, usually nothing happens.
        if (rnd(3))
        {
            msg("nothing happens");
            return;
        }
        //But sometimes you can still use it.
        //However...
        if (rnd(3) < -obj->arm)
        {
            //The wand explodes, take 2d6 damage!
            msg("The %s explodes!", inv_name(obj, false));

            leave_pack(obj, false, false);
            delete obj;

            take_damage(roll(2, 6), 'z');
            return;
        }
    }
    switch (obj->which)
    {
        case WS_LIGHT:
            /*
             * Reddy Kilowat wand.  Light up the room
             */
            ws_info[WS_LIGHT].oi_know = true;
            visit_field_of_view(hero.x, hero.y, 10, [](int x, int y)
            {
                flags_at(x, y) |= F_ISLIT;
            });
            addmsg("your surroundings are lit");
            if (!terse)
                addmsg(" by a shimmering %s light", pick_color("blue"));
            endmsg();
        when WS_DRAIN:
            /*
             * take away 1/2 of hero's hit points, then take it away
             * evenly from the monsters in the room (or next to hero
             * if he is in a passage)
             */
            if (player.stats.s_hpt < 2)
            {
                msg("you are too weak to use it");
                return;
            }
            else
                drain();
        when WS_INVIS:
        case WS_POLYMORPH:
        case WS_TELAWAY:
        case WS_TELTO:
        case WS_CANCEL:
            y = hero.y;
            x = hero.x;
            while(step_ok(char_at(x, y)) && !monster_at(x, y))
            {
                y += delta.y;
                x += delta.x;
            }
            if ((tp = monster_at(x, y)) != nullptr)
            {
                monster = tp->type;
                if (monster == 'F')
                    player.flags &= ~ISHELD;
                switch (obj->which) {
                    case WS_INVIS:
                        tp->flags |= ISINVIS;
                        if (cansee(y, x))
                            setMapDisplay(x, y, char_at_place(x, y));
                        break;
                    case WS_POLYMORPH:
                    {
                        tp->polymorph(rnd(26) + 'A');
                        if (see_monst(tp))
                            setMapDisplay(x, y, tp->type);
                        ws_info[WS_POLYMORPH].oi_know |= see_monst(tp);
                        break;
                    }
                    case WS_CANCEL:
                        tp->flags |= ISCANC;
                        tp->flags &= ~(ISINVIS|CANHUH);
                        tp->disguise = tp->type;
                        if (see_monst(tp))
                            setMapDisplay(x, y, tp->disguise);
                        break;
                    case WS_TELAWAY:
                    case WS_TELTO:
                    {
                        coord new_pos;

                        if (obj->which == WS_TELAWAY)
                        {
                            new_pos = Area::random_position(Area::ForMonster);
                        }
                        else
                        {
                            new_pos.y = hero.y + delta.y;
                            new_pos.x = hero.x + delta.x;
                        }
                        tp->dest = &hero;
                        tp->flags |= ISRUN;
                        relocate(tp, &new_pos);
                    }
                }
            }
        when WS_MISSILE:
            ws_info[WS_MISSILE].oi_know = true;
            bolt.type = '*';
            strncpy(bolt.hurldmg,"1x4",sizeof(bolt.hurldmg));
            bolt.hplus = 100;
            bolt.dplus = 1;
            bolt.flags = ISMISL;
            if (cur_weapon != nullptr)
                bolt.launch = cur_weapon->which;
            do_motion(&bolt, delta.y, delta.x);
            if ((tp = monster_at(bolt.pos.x, bolt.pos.y)) != nullptr && !save_throw(VS_MAGIC, tp))
                hit_monster(unc(bolt.pos), &bolt);
            else if (terse)
                msg("missle vanishes");
            else
                msg("the missle vanishes with a puff of smoke");
        when WS_HASTE_M:
        case WS_SLOW_M:
            y = hero.y;
            x = hero.x;
            while (step_ok(char_at(x, y)) && !monster_at(x, y))
            {
                y += delta.y;
                x += delta.x;
            }
            if ((tp = monster_at(x, y)) != nullptr)
            {
                if (obj->which == WS_HASTE_M)
                {
                    if (on(*tp, ISSLOW))
                        tp->flags &= ~ISSLOW;
                    else
                        tp->flags |= ISHASTE;
                }
                else
                {
                    if (on(*tp, ISHASTE))
                        tp->flags &= ~ISHASTE;
                    else
                        tp->flags |= ISSLOW;
                    tp->turn_delay = 2;
                }
                delta.y = y;
                delta.x = x;
                runto(tp);
            }
        when WS_ELECT:
        case WS_FIRE:
        case WS_COLD:
            if (obj->which == WS_ELECT)
                name = "bolt";
            else if (obj->which == WS_FIRE)
                name = "flame";
            else
                name = "ice";
            fire_bolt(&hero, &delta, name);
            ws_info[obj->which].oi_know = true;
        when WS_NOP:
            break;
    }
    obj->arm--;//amount of charges left is stored in the arm field
}

/*
 * drain:
 *        Do drain hit points from player shtick
 */

void
drain()
{
    std::vector<MonsterThing*> drainee;

    /*
     * First cnt how many things we need to spread the hit points among
     */
    visit_field_of_view(hero.x, hero.y, 10, [&drainee](int x, int y)
    {
        if (monster_at(x, y))
            drainee.push_back(monster_at(x, y));
    });
    if (drainee.size() == 0)
    {
        msg("you have a tingling feeling");
        return;
    }
    player.stats.s_hpt /= 2;
    int amount = player.stats.s_hpt / drainee.size();
    /*
     * Now zot all of the monsters
     */
    for(MonsterThing* mp : drainee)
    {
        if ((mp->stats.s_hpt -= amount) <= 0)
            killed(mp, see_monst(mp));
        else
            runto(mp);
    }
}

/*
 * fire_bolt:
 *        Fire a bolt in a given direction from a specific starting place
 */

void fire_bolt(coord *start, coord *dir, const char *name)
{
    coord *c1, *c2;
    MonsterThing *tp;
    char dirch = 0, ch;
    bool hit_hero, used, changed;
    static coord pos;
    static coord spotpos[BOLT_LENGTH];
    ItemThing bolt;

    bolt.type = WEAPON;
    bolt.which = FLAME;
    strncpy(bolt.hurldmg,"6x6",sizeof(bolt.hurldmg));
    bolt.hplus = 100;
    bolt.dplus = 0;
    weap_info[FLAME].oi_name = name;
    switch (dir->y + dir->x)
    {
        case 0: dirch = '/';
        when 1: case -1: dirch = (dir->y == 0 ? '-' : '|');
        when 2: case -2: dirch = '\\';
    }
    pos = *start;
    hit_hero = (bool)(start != &hero);
    used = false;
    changed = false;
    for (c1 = spotpos; c1 <= &spotpos[BOLT_LENGTH-1] && !used; c1++)
    {
        pos.y += dir->y;
        pos.x += dir->x;
        *c1 = pos;
        ch = char_at(pos.x, pos.y);
        switch (ch)
        {
            case DOOR:
                /*
                 * this code is necessary if the hero is on a door
                 * and he fires at the wall the door is in, it would
                 * otherwise loop infinitely
                 */
                if (ce(hero, pos))
                    goto def;
                /* FALLTHROUGH */
            case '|':
            case '-':
            case WALL_H:
            case WALL_V:
            case WALL_TL:
            case WALL_TR:
            case WALL_BL:
            case WALL_BR:
            case SOLID_WALL:
            case ' ':
                if (!changed)
                    hit_hero = !hit_hero;
                changed = false;
                dir->y = -dir->y;
                dir->x = -dir->x;
                c1--;
                msg("the %s bounces", name);
                break;
            default:
def:
                if (!hit_hero && (tp = monster_at(pos.x, pos.y)) != nullptr)
                {
                    hit_hero = true;
                    changed = !changed;
                    if (!save_throw(VS_MAGIC, tp))
                    {
                        bolt.pos = pos;
                        used = true;
                        if (tp->type == 'D' && strcmp(name, "flame") == 0)
                        {
                            addmsg("the flame bounces");
                            if (!terse)
                                addmsg(" off the dragon");
                            endmsg();
                        }
                        else
                            hit_monster(unc(pos), &bolt);
                    }
                    else if (ch != 'M' || tp->disguise == 'M')
                    {
                        if (start == &hero)
                            runto(tp);
                        if (terse)
                            msg("%s misses", name);
                        else
                            msg("the %s whizzes past %s", name, set_mname(tp));
                    }
                }
                else if (hit_hero && ce(pos, hero))
                {
                    hit_hero = false;
                    changed = !changed;
                    if (!save(VS_MAGIC))
                    {
                        take_damage(roll(6, 6), (start == &hero) ? ('b') : (monster_at(start->x, start->y)->type));
                        used = true;
                        if (terse)
                            msg("the %s hits", name);
                        else
                            msg("you are hit by the %s", name);
                    }
                    else
                        msg("the %s whizzes by you", name);
                }
                setMapDisplay(pos.x, pos.y, dirch);
        }
    }
    refreshMapWithMore();
    for (c2 = spotpos; c2 < c1; c2++)
        setMapDisplay(c2->x, c2->y, char_at_place(c2->x, c2->y));
    refreshMap();
}

/*
 * charge_str:
 *        Return an appropriate string for a wand charge
 */
const char * charge_str(ItemThing *obj)
{
    static char buf[20];

    if (!(obj->flags & ISKNOW))
        buf[0] = '\0';
    else if (terse)
        sprintf(buf, " [%d]", std::max(0, obj->arm));//amount of charges is stored in the arm field
    else
        sprintf(buf, " [%d charges]", std::max(0, obj->arm));
    return buf;
}
