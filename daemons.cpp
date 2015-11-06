/*
 * All the daemon and fuse functions are in here
 *
 * @(#)daemons.c        4.24 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include "rogue.h"

/*
 * doctor:
 *        A healing daemon that restors hit points after rest
 */
int doctor(int arg)
{
    register int lv, ohp;

    lv = player.stats.s_lvl;
    ohp = player.stats.s_hpt;
    quiet++;
    if (lv < 8)
    {
        if (quiet + (lv << 1) > 20)
            player.stats.s_hpt++;
    }
    else
        if (quiet >= 3)
            player.stats.s_hpt += rnd(lv - 7) + 1;
    if (ISRING(LEFT, R_REGEN))
        player.stats.s_hpt++;
    if (ISRING(RIGHT, R_REGEN))
        player.stats.s_hpt++;
    if (ohp != player.stats.s_hpt)
    {
        if (player.stats.s_hpt > player.stats.s_maxhp)
            player.stats.s_hpt = player.stats.s_maxhp;
        quiet = 0;
    }
    return 0;
}

/*
 * Swander:
 *        Called when it is time to start rolling for wandering monsters
 */
int swander(int arg)
{
    start_daemon(rollwand, 0, BEFORE);
    return 0;
}

/*
 * rollwand:
 *        Called to roll to see if a wandering monster starts up
 */
int between = 0;
int rollwand(int arg)
{

    if (++between >= 4)
    {
        if (roll(1, 6) == 4)
        {
            wanderer();
            kill_daemon(rollwand);
            fuse(swander, 0, WANDERTIME, BEFORE);
        }
        between = 0;
    }
    return 0;
}

/*
 * unconfuse:
 *        Release the poor player from his confusion
 */
int unconfuse(int arg)
{
    player.flags &= ~ISHUH;
    msg("you feel less %s now", choose_str("trippy", "confused"));
    return 0;
}

/*
 * unsee:
 *        Turn off the ability to see invisible
 */
int unsee(int arg)
{
    for(MonsterThing *th : mlist)
    {
        if (on(*th, ISINVIS) && see_monst(th))
        {
            player.flags &=~CANSEE; //Swap this flag to cheat the char_at_place function into giving us what we want, without causing problems for the see_monst function.
            setMapDisplay(th->pos.x, th->pos.y, char_at_place(th->pos.x, th->pos.y));
            player.flags |= CANSEE;
        }
    }
    player.flags &= ~CANSEE;
    return 0;
}

/*
 * sight:
 *        He gets his sight back
 */
int sight(int arg)
{
    if (on(player, ISBLIND))
    {
        extinguish(sight);
        player.flags &= ~ISBLIND;
        msg(choose_str("far out!  Everything is all cosmic again",
                       "the veil of darkness lifts"));
    }
    return 0;
}

/*
 * nohaste:
 *        End the hasting
 */
int nohaste(int arg)
{
    player.flags &= ~ISHASTE;
    msg("you feel yourself slowing down");
    return 0;
}

/*
 * stomach:
 *        Digest the hero's food
 */
int stomach(int arg)
{
    register int oldfood;
    int orig_hungry = hungry_state;

    if (food_left <= 0)
    {
        if (food_left-- < -STARVETIME)
            death('s');
        /*
         * the hero is fainting
         */
        if (no_command || rnd(5) != 0)
            return 0;
        no_command += rnd(8) + 4;
        hungry_state = 3;
        if (!terse)
            addmsg(choose_str("the munchies overpower your motor capabilities.",
                              "you feel too weak from lack of food."));
        msg(choose_str("You freak out", "You faint"));
    }
    else
    {
        oldfood = food_left;
        food_left -= ring_eat(LEFT) + ring_eat(RIGHT) + 1 - amulet;

        if (food_left < MORETIME && oldfood >= MORETIME)
        {
            hungry_state = 2;
            msg(choose_str("the munchies are interfering with your motor capabilites",
                           "you are starting to feel weak"));
        }
        else if (food_left < 2 * MORETIME && oldfood >= 2 * MORETIME)
        {
            hungry_state = 1;
            if (terse)
                msg(choose_str("getting the munchies", "getting hungry"));
            else
                msg(choose_str("you are getting the munchies",
                               "you are starting to get hungry"));
        }
    }
    if (hungry_state != orig_hungry) { 
        player.flags &= ~ISRUN; 
        running = FALSE; 
        to_death = FALSE; 
        count = 0; 
    }
    return 0;
}

/*
 * come_down:
 *        Take the hero down off her acid trip.
 */
int come_down(int arg)
{
    bool seemonst;

    if (!on(player, ISHALU))
        return 0;

    kill_daemon(visuals);
    player.flags &= ~ISHALU;

    if (on(player, ISBLIND))
        return 0;

    /*
     * undo the things
     */
    for (ItemThing* tp : lvl_obj)
        if (cansee(tp->pos.y, tp->pos.x))
            setMapDisplay(tp->pos.x, tp->pos.y, char_at_place(tp->pos.x, tp->pos.y));

    /*
     * undo the monsters
     */
    seemonst = on(player, SEEMONST);
    for(MonsterThing* mp : mlist)
    {
        if (cansee(mp->pos.y, mp->pos.x) || seemonst)
        {
            setMapDisplay(mp->pos.x, mp->pos.y, char_at_place(mp->pos.x, mp->pos.y));
        }
    }
    msg("Everything looks SO boring now.");
    return 0;
}

/*
 * visuals:
 *        change the characters for the player
 */
int visuals(int arg)
{
    register bool seemonst;

    if (!after || (running && jump))
        return 0;
    /*
     * change the things
     */
    for (ItemThing* tp : lvl_obj)
        if (cansee(tp->pos.y, tp->pos.x))
            setMapDisplay(tp->pos.x, tp->pos.y, rnd_thing());

    /*
     * change the stairs
     */
    if (!seenstairs && cansee(stairs.y, stairs.x))
        setMapDisplay(stairs.x, stairs.y, rnd_thing());

    /*
     * change the monsters
     */
    seemonst = on(player, SEEMONST);
    for (MonsterThing *mp : mlist)
    {
        if (see_monst(mp))
        {
            if (mp->type == 'X' && mp->disguise != 'X')
                setMapDisplay(mp->pos.x, mp->pos.y, rnd_thing());
            else
                setMapDisplay(mp->pos.x, mp->pos.y, rnd(26) + 'A');
        }
        else if (seemonst)
        {
            setMapDisplay(mp->pos.x, mp->pos.y, (rnd(26) + 'A') | DISPLAY_INVERT);
        }
    }
    return 0;
}

/*
 * land:
 *        Land from a levitation potion
 */
int land(int arg)
{
    player.flags &= ~ISLEVIT;
    msg(choose_str("bummer!  You've hit the ground",
                   "you float gently to the ground"));
    return 0;
}
