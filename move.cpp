/*
 * hero movement commands
 *
 * @(#)move.c        4.49 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <ctype.h>
#include "rogue.h"

/*
 * do_run:
 *        Start the hero running
 */

void do_run(int ch)
{
    running = true;
    after = false;
    runch = ch;
}

/*
 * do_move:
 *        Check to see that a move is legal.  If it is handle the
 * consequences (fighting, picking up, etc.)
 */

void do_move(int dy, int dx)
{
    int fl;
    coord nh;

    firstmove = false;
    if (no_move)
    {
        no_move--;
        msg("you are still stuck in the bear trap");
        return;
    }
    /*
     * Do a confused move (maybe)
     */
    if (on(player, ISHUH) && rnd(5) != 0)
    {
        nh = rndmove(&player);
        if (ce(nh, hero))
        {
            after = false;
            running = false;
            to_death = false;
            return;
        }
    }
    else
    {
        nh.y = hero.y + dy;
        nh.x = hero.x + dx;
    }

    /*
     * Check if he tried to move off the screen or make an illegal
     * diagonal move, and stop him if he did.
     */
    if (nh.x < 0 || nh.x >= NUMCOLS || nh.y <= 0 || nh.y >= NUMLINES - 1)
        goto hit_bound;
    if (!diag_ok(&hero, &nh))
    {
        after = false;
        running = false;
        return;
    }
    if (running && ce(hero, nh))
        after = running = false;
    fl = flags_at(nh.x, nh.y);
    if (!(fl & F_REAL) && char_at(nh.x, nh.y) == FLOOR)
    {
        if (!on(player, ISLEVIT))
        {
            char_at(nh.x, nh.y) = TRAP;
            flags_at(nh.x, nh.y) |= F_REAL;
        }
    }
    else if (on(player, ISHELD) && !monster_at(nh.x, nh.y))
    {
        msg("you are being held");
        return;
    }
    switch (char_at(nh.x, nh.y))
    {
        case ' ':
        case '|':
        case '-':
        case WALL_H:
        case WALL_V:
        case WALL_TL:
        case WALL_TR:
        case WALL_BL:
        case WALL_BR:
        case SOLID_WALL:
hit_bound:
            running = false;
            after = false;
            break;
        case CLOSED_DOOR:
            if (flags_at(nh.x, nh.y) & F_LOCKED)
            {
                //TODO: How to deal with locked doors? Right now locked doors are never placed.
                msg("The door is locked");
                after = false;
                return;
            }
            running = false;
            char_at(nh.x, nh.y) = DOOR;
            break;
        case DOOR:
            running = false;
            goto move_stuff;
        case TRAP:
            {
                int ch = be_trapped(&nh);
                if (ch == T_DOOR || ch == T_TELEP)
                    return;
                goto move_stuff;
            }
        case PASSAGE:
        case PASSAGE_UNLIT:
            goto move_stuff;
        case FLOOR:
            if (!(fl & F_REAL))
                be_trapped(&nh);
            goto move_stuff;
        case STAIRS:
            seenstairs = true;
            /* FALLTHROUGH */
        default:
            running = false;
move_stuff:
            if (monster_at(nh.x, nh.y))
                fight(&nh, cur_weapon, false);
            else
            {
                if (item_at(nh.x, nh.y))
                    take = item_at(nh.x, nh.y)->type;
                setMapDisplay(hero.x, hero.y, char_at_place(hero.x, hero.y));
                hero = nh;
            }
    }
}

/*
 * turn_ok:
 *        Decide whether it is legal to turn onto the given space
 */
bool turn_ok(int y, int x)
{
    PLACE *pp;

    pp = INDEX(y, x);
    return (pp->p_ch == DOOR
        || (pp->p_flags & (F_REAL|F_PASS)) == (F_REAL|F_PASS));
}

/*
 * be_trapped:
 *        The guy stepped on a trap.... Make him pay.
 */
char be_trapped(coord *tc)
{
    PLACE *pp;
    ItemThing *arrow;
    char tr;

    if (on(player, ISLEVIT))
        return T_RUST;        /* anything that's not a door or teleport, as returning those causes different handling */
    running = false;
    count = false;
    pp = INDEX(tc->y, tc->x);
    pp->p_ch = TRAP;
    tr = pp->p_flags & F_TMASK;
    pp->p_flags |= F_SEEN;
    switch (tr)
    {
        case T_DOOR:
            level++;
            new_level();
            msg("you fell into a trap!");
        when T_BEAR:
            no_move += BEARTIME;
            msg("you are caught in a bear trap");
        when T_MYST:
            switch(rnd(11))
            {
                case 0: msg("you are suddenly in a parallel dimension");
                when 1: msg("the light in here suddenly seems %s", rainbow[rnd(cNCOLORS)]);
                when 2: msg("you feel a sting in the side of your neck");
                when 3: msg("multi-colored lines swirl around you, then fade");
                when 4: msg("a %s light flashes in your eyes", rainbow[rnd(cNCOLORS)]);
                when 5: msg("a spike shoots past your ear!");
                when 6: msg("%s sparks dance across your armor", rainbow[rnd(cNCOLORS)]);
                when 7: msg("you suddenly feel very thirsty");
                when 8: msg("you feel time speed up suddenly");
                when 9: msg("time now seems to be going slower");
                when 10: msg("you pack turns %s!", rainbow[rnd(cNCOLORS)]);
            }
        when T_SLEEP:
            no_command += SLEEPTIME;
            player.flags &= ~ISRUN;
            msg("a strange white mist envelops you and you fall asleep");
        when T_ARROW:
            if (swing(player.stats.s_lvl - 1, player.stats.s_arm, 1))
            {
                msg("oh no! An arrow shot you");
                take_damage(roll(1, 6), 'a');
            }
            else
            {
                arrow = new ItemThing();
                init_weapon(arrow, ARROW);
                arrow->count = 1;
                arrow->pos = hero;
                fall(arrow, false);
                msg("an arrow shoots past you");
            }
        when T_TELEP:
            /*
             * since the hero's leaving, look() won't put a TRAP
             * down for us, so we have to do it ourself
             */
            teleport();
            setMapDisplay(tc->x, tc->y, TRAP);
        when T_DART:
            if (!swing(player.stats.s_lvl+1, player.stats.s_arm, 1))
                msg("a small dart whizzes by your ear and vanishes");
            else
            {
                msg("a small dart just hit you in the shoulder");
                take_damage(roll(1, 4), 'd');
                if (!ISWEARING(R_SUSTSTR) && !save(VS_POISON))
                    chg_str(-1);
            }
        when T_RUST:
            msg("a gush of water hits you on the head");
            rust_armor(cur_armor);
    }
    flush_type();
    return tr;
}

/*
 * rndmove:
 *        Move in a random direction if the monster/person is confused
 */
coord rndmove(MonsterThing *who)
{
    int x, y;
    coord ret;  /* what we will be returning */

    y = ret.y = who->pos.y + rnd(3) - 1;
    x = ret.x = who->pos.x + rnd(3) - 1;
    /*
     * Now check to see if that's a legal move.  If not, don't move.
     * (I.e., bump into the wall or whatever)
     */
    if (y == who->pos.y && x == who->pos.x)
        return ret;
    if (!diag_ok(&who->pos, &ret))
        goto bad;
    else
    {
        if (!step_ok(char_at(x, y)))
            goto bad;
        if (monster_at(x, y))
            goto bad;
        if (item_at(x, y) && item_at(x, y)->type == SCROLL && item_at(x, y)->which == S_SCARE)
            goto bad;
    }
    return ret;

bad:
    ret = who->pos;
    return ret;
}

/*
 * rust_armor:
 *        Rust the given armor, if it is a legal kind to rust, and we
 *        aren't wearing a magic ring.
 */

void rust_armor(ItemThing *arm)
{
    if (arm == nullptr || arm->type != ARMOR || arm->which == LEATHER || arm->arm >= 9)
        return;

    if ((arm->flags & ISPROT) || ISWEARING(R_SUSTARM))
    {
        if (!to_death)
            msg("the rust vanishes instantly");
    }
    else
    {
        arm->arm++;
        if (!terse)
            msg("your armor appears to be weaker now. Oh my!");
        else
            msg("your armor weakens");
    }
}
