/*
 * All sorts of miscellaneous routines
 *
 * @(#)misc.c        4.66 (Berkeley) 08/06/83
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

/*
 * look:
 *        A quick glance all around the player
 */
void look(bool wakeup)
{
    int x, y;
    int ch;
    MonsterThing *tp;
    PLACE *pp;
    int ey, ex;
    int passcount;
    char pfl, pch;
    int* fp;
    int sy, sx, sumhero = 0, diffhero = 0;

    passcount = 0;
    if (!ce(oldpos, hero))
    {
        erase_lamp(oldpos);
        oldpos = hero;
    }
    ey = hero.y + 1;
    ex = hero.x + 1;
    sx = hero.x - 1;
    sy = hero.y - 1;
    if (door_stop && !firstmove && running)
    {
        sumhero = hero.y + hero.x;
        diffhero = hero.y - hero.x;
    }
    pp = INDEX(hero.y, hero.x);
    pch = pp->p_ch;
    pfl = pp->p_flags;
    
    if (!on(player, ISBLIND))
    {
        visit_field_of_view(hero.x, hero.y, 20, [wakeup](int x, int y)
        {
            if (dist(hero.y, hero.x, y, x) < LAMPDIST || (flags_at(x, y) & F_ISLIT))
            {
                int ch = char_at_place(x, y);
                ch = trip_ch(y, x, ch);
                setMapDisplay(x, y, ch);
                flags_at(x, y) |= F_SEEN;
                
                if (wakeup && monster_at(x, y))
                {
                    wake_monster(y, x);
                    if (!firstmove)
                        running = false;
                }
            }
        });
    }

    for (y = sy; y <= ey; y++)
        if (y > 0 && y < NUMLINES - 1) for (x = sx; x <= ex; x++)
        {
            if (x < 0 || x >= NUMCOLS)
                continue;
            if (!on(player, ISBLIND))
            {
                if (y == hero.y && x == hero.x)
                    continue;
            }

            pp = INDEX(y, x);
            ch = pp->p_ch;
            if (ch == ' ')                /* nothing need be done with a ' ' */
                continue;
            fp = &pp->p_flags;
            if (pch != DOOR && ch != DOOR)
                if ((pfl & F_PASS) != (*fp & F_PASS))
                    continue;
            if (((*fp & F_PASS) || ch == DOOR) && 
                 ((pfl & F_PASS) || pch == DOOR))
            {
                if (hero.x != x && hero.y != y &&
                    !step_ok(char_at(hero.x, y)) && !step_ok(char_at(x, hero.y)))
                        continue;
            }

            if ((tp = pp->p_monst) == nullptr)
                ch = trip_ch(y, x, ch);
            else
                if (on(player, SEEMONST) && on(*tp, ISINVIS))
                {
                    if (door_stop && !firstmove)
                        running = false;
                    continue;
                }
                else
                {
                    if (see_monst(tp))
                    {
                        if (on(player, ISHALU))
                            ch = rnd(26) + 'A';
                        else
                            ch = tp->disguise;
                    }
                }
            if (on(player, ISBLIND) && (y != hero.y || x != hero.x))
                continue;
            
            if ((door_stop || (x == hero.x && y == hero.y)) && item_at(x, y) && !firstmove)
                running = false;

            if (door_stop && !firstmove && running)
            {
                switch (runch)
                {
                    case 'h': case K_LEFT:
                        if (x == ex)
                            continue;
                    when 'j': case K_DOWN:
                        if (y == sy)
                            continue;
                    when 'k': case K_UP:
                        if (y == ey)
                            continue;
                    when 'l': case K_RIGHT:
                        if (x == sx)
                            continue;
                    when 'y': case K_UP_LEFT:
                        if ((y + x) - sumhero >= 1)
                            continue;
                    when 'u': case K_UP_RIGHT:
                        if ((y - x) - diffhero >= 1)
                            continue;
                    when 'n': case K_DOWN_RIGHT:
                        if ((y + x) - sumhero <= -1)
                            continue;
                    when 'b': case K_DOWN_LEFT:
                        if ((y - x) - diffhero <= -1)
                            continue;
                }
                switch (ch)
                {
                    case DOOR:
                        if (x == hero.x || y == hero.y)
                            running = false;
                        break;
                    case PASSAGE:
                    case PASSAGE_UNLIT:
                        if (x == hero.x || y == hero.y)
                            passcount++;
                        break;
                    case FLOOR:
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
                        break;
                    default:
                        running = false;
                        break;
                }
            }
        }
    if (door_stop && !firstmove && passcount > 1)
        running = false;
    if (!running || !jump)
        setMapDisplay(hero.x, hero.y, PLAYER);
}

/*
 * trip_ch:
 *        Return the character appropriate for this space, taking into
 *        account whether or not the player is tripping.
 */
int trip_ch(int y, int x, int ch)
{
    if (on(player, ISHALU) && after)
        switch (ch)
        {
            case FLOOR:
            case ' ':
            case PASSAGE:
            case PASSAGE_UNLIT:
            case '-':
            case '|':
            case WALL_H:
            case WALL_V:
            case WALL_TL:
            case WALL_TR:
            case WALL_BL:
            case WALL_BR:
            case SOLID_WALL:
            case DOOR:
            case TRAP:
                break;
            default:
                if (ch >= 'A' && ch <= 'Z')
                    ch = rnd(26) + 'A';
                else if (y != stairs.y || x != stairs.x || !seenstairs)
                    ch = rnd_thing();
                break;
        }
    return ch;
}

/*
 * erase_lamp:
 *        Erase the area shown by a lamp in a dark room.
 */

void erase_lamp(coord& pos)
{
    visit_field_of_view(pos.x, pos.y, 20, [](int x, int y)
    {
        if (x == hero.x && y == hero.y)
            return;
        int ch = char_at_place(x, y);
        if (flags_at(x, y) & F_SEEN)
        {
            if (ch == PASSAGE)
                setMapDisplay(x, y, PASSAGE_UNLIT);
            else if (!IS_WALL(ch) && ch != CLOSED_DOOR && ch != DOOR && ch != STAIRS)
                setMapDisplay(x, y, ' ');
        }else{
            setMapDisplay(x, y, ' ');
        }
    });
}

/*
 * eat:
 *        She wants to eat something, so let her try
 */

void eat()
{
    ItemThing *obj;

    if ((obj = get_item("eat", FOOD)) == nullptr)
        return;
    if (obj->type == FOOD)
    {
        add_food(HUNGERTIME - 200 + rnd(400));

        if (obj->which == 1)
        {
            msg("my, that was a yummy %s", fruit);
        }
        else
        {
            if (rnd(100) > 70)
            {
                player.stats.s_exp++;
                msg("%s, this food tastes awful", choose_str("bummer", "yuk"));
                check_level();
            }
            else
            {
                msg("%s, that tasted good", choose_str("oh, wow", "yum"));
            }
        }
    }else if (obj->type == SCROLL)
    {
        //Sure, eat a scroll, it's good for you... (well, not really. But it does help with the hunger)
        msg("blegh, that was hard to digest");
        if (hungry_state)
            msg("but it helps stilling the hunger");
        add_food(400 + rnd(100));
        take_damage(roll(1, 3), 'e');
    }else if (obj->type == POTION)
    {
        msg("that's Inedible! Quaff potions instead.");
    }else{
        if (!terse)
            msg("ugh, you would get ill if you ate that");
        else
            msg("that's Inedible!");
        return;
    }

    if (obj == cur_weapon)
        cur_weapon = nullptr;
    leave_pack(obj, false, false);
}

void add_food(int amount)
{
    if (food_left < 0)
        food_left = 0;
    if ((food_left += amount) > STOMACHSIZE)
        food_left = STOMACHSIZE;
    if (food_left < MORETIME)
    {
        hungry_state = 2;
    }
    else if (food_left < 2 * MORETIME)
    {
        hungry_state = 1;
    }
    else
    {
        hungry_state = 0;
    }
}

/*
 * check_level:
 *        Check to see if the guy has gone up a level.
 */

void check_level()
{
    int i, add, olevel;

    for (i = 0; e_levels[i] != 0; i++)
        if (e_levels[i] > player.stats.s_exp)
            break;
    i++;
    olevel = player.stats.s_lvl;
    player.stats.s_lvl = i;
    if (i > olevel)
    {
        add = (i - olevel) * 5 + roll(i - olevel, 5);
        player.stats.s_maxhp += add;
        player.stats.s_hpt += add;
        msg("welcome to level %d", i);
    }
}

/*
 * chg_str:
 *        used to modify the playes strength.  It keeps track of the
 *        highest it has been, just in case
 */

void chg_str(int amt)
{
    str_t comp;

    if (amt == 0)
        return;
    add_str(&player.stats.s_str, amt);
    comp = player.stats.s_str;
    if (ISRING(LEFT, R_ADDSTR))
        add_str(&comp, -cur_ring[LEFT]->arm);
    if (ISRING(RIGHT, R_ADDSTR))
        add_str(&comp, -cur_ring[RIGHT]->arm);
    if (comp > max_stats.s_str)
        max_stats.s_str = comp;
}

/*
 * add_str:
 *        Perform the actual add, checking upper and lower bound limits
 */
void add_str(str_t *sp, int amt)
{
    if ((*sp += amt) < 3)
        *sp = 3;
    else if (*sp > 31)
        *sp = 31;
}

/*
 * add_haste:
 *        Add a haste to the player
 */
bool add_haste(bool potion)
{
    if (on(player, ISHASTE))
    {
        no_command += rnd(8);
        player.flags &= ~(ISRUN|ISHASTE);
        extinguish(nohaste);
        msg("you faint from exhaustion");
        return false;
    }
    else
    {
        player.flags |= ISHASTE;
        if (potion)
            fuse(nohaste, 0, rnd(4)+4, AFTER);
        return true;
    }
}

/*
 * aggravate:
 *        Aggravate all the monsters on this level
 */

void aggravate()
{
    for(MonsterThing *mp : mlist)
        runto(mp->pos);
}

/*
 * vowelstr:
 *      For printfs: if string starts with a vowel, return "n" for an
 *        "an".
 */
const char * vowelstr(const char *str)
{
    switch (*str)
    {
        case 'a': case 'A':
        case 'e': case 'E':
        case 'i': case 'I':
        case 'o': case 'O':
        case 'u': case 'U':
            return "n";
        default:
            return "";
    }
}

/* 
 * is_current:
 *        See if the object is one of the currently used items
 */
bool is_current(ItemThing *obj)
{
    if (obj == nullptr)
        return false;
    if (obj == cur_armor || obj == cur_weapon || obj == cur_ring[LEFT] || obj == cur_ring[RIGHT])
    {
        if (!terse)
            addmsg("That's already ");
        msg("in use");
        return true;
    }
    return false;
}

/*
 * get_dir:
 *      Set up the direction co_ordinate for use in varios "prefix"
 *        commands
 */
bool get_dir()
{
    const char *prompt;
    bool gotit;
    static coord last_delt= {0,0};

    if (again && last_dir != '\0')
    {
        delta.y = last_delt.y;
        delta.x = last_delt.x;
        dir_ch = last_dir;
    }
    else
    {
        if (!terse)
            dir_ch = displayMessage(prompt = "which direction? ");
        else
            prompt = "direction: ";
        do
        {
            gotit = true;
            switch (dir_ch)
            {
                case 'h': case'H': case K_LEFT:       delta.y =  0; delta.x = -1;
                when 'j': case'J': case K_DOWN:       delta.y =  1; delta.x =  0;
                when 'k': case'K': case K_UP:         delta.y = -1; delta.x =  0;
                when 'l': case'L': case K_RIGHT:      delta.y =  0; delta.x =  1;
                when 'y': case'Y': case K_UP_LEFT:    delta.y = -1; delta.x = -1;
                when 'u': case'U': case K_UP_RIGHT:   delta.y = -1; delta.x =  1;
                when 'b': case'B': case K_DOWN_LEFT:  delta.y =  1; delta.x = -1;
                when 'n': case'N': case K_DOWN_RIGHT: delta.y =  1; delta.x =  1;
                when ESCAPE: last_dir = '\0'; reset_last(); return false;
                otherwise:
                    dir_ch = displayMessage(prompt);
                    gotit = false;
            }
        } until (gotit);
        if (isupper(dir_ch))
            dir_ch = tolower(dir_ch);
        last_dir = dir_ch;
        last_delt.y = delta.y;
        last_delt.x = delta.x;
    }
    if (on(player, ISHUH) && rnd(5) == 0)
        do
        {
            delta.y = rnd(3) - 1;
            delta.x = rnd(3) - 1;
        } while (delta.y == 0 && delta.x == 0);
    return true;
}

/*
 * sign:
 *        Return the sign of the number
 */
int sign(int nm)
{
    if (nm < 0)
        return -1;
    else
        return (nm > 0);
}

/*
 * spread:
 *        Give a spread around a given number (+/- 20%)
 */
int spread(int nm)
{
    return nm - nm / 20 + rnd(nm / 10);
}

/*
 * call_it:
 *        Call an object something after use.
 */

void call_it(struct obj_info *info)
{
    if (info->oi_know)
    {
        if (info->oi_guess)
        {
            free(info->oi_guess);
            info->oi_guess = nullptr;
        }
    }
    else if (!info->oi_guess)
    {
        refreshMapWithMore(); //First show the map, as for some scrolls we need to see the effect
        if (askForInput(terse ? "call it:" : "what do you want to call it?", prbuf, MAXSTR) == NORM)
        {
            if (info->oi_guess != nullptr)
                free(info->oi_guess);
            info->oi_guess = (char*)malloc((unsigned int) strlen(prbuf) + 1);
            strcpy(info->oi_guess, prbuf);
        }
    }
}

/*
 * rnd_thing:
 *        Pick a random thing appropriate for this level
 */
char rnd_thing()
{
    int i;
    static char thing_list[] = {
        POTION, SCROLL, RING, STICK, FOOD, WEAPON, ARMOR, STAIRS, GOLD, AMULET
    };

    if (level >= AMULETLEVEL)
        i = rnd(sizeof thing_list / sizeof (char));
    else
        i = rnd(sizeof thing_list / sizeof (char) - 1);
    return thing_list[i];
}

/*
 str str:
 *        Choose the first or second string depending on whether it the
 *        player is tripping
 */
const char* choose_str(const char *ts, const char *ns)
{
    return (on(player, ISHALU) ? ts : ns);
}

/* Return the character that needs to be displayed at position x/y */
int char_at_place(int x, int y)
{
    MonsterThing* mp = monster_at(x, y);
    if (mp)
    {
        if (!on(*mp, ISINVIS) || on(player, CANSEE) || on(player, SEEMONST))
            return mp->disguise;
    }
    if (item_at(x, y))
        return item_at(x, y)->type;
    return char_at(x, y);
}
