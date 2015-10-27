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

    lv = pstats.s_lvl;
    ohp = pstats.s_hpt;
    quiet++;
    if (lv < 8)
    {
        if (quiet + (lv << 1) > 20)
            pstats.s_hpt++;
    }
    else
        if (quiet >= 3)
            pstats.s_hpt += rnd(lv - 7) + 1;
    if (ISRING(LEFT, R_REGEN))
        pstats.s_hpt++;
    if (ISRING(RIGHT, R_REGEN))
        pstats.s_hpt++;
    if (ohp != pstats.s_hpt)
    {
        if (pstats.s_hpt > max_hp)
            pstats.s_hpt = max_hp;
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
    player.t_flags &= ~ISHUH;
    msg("you feel less %s now", choose_str("trippy", "confused"));
    return 0;
}

/*
 * unsee:
 *        Turn off the ability to see invisible
 */
int unsee(int arg)
{
    register THING *th;

    for (th = mlist; th != NULL; th = next(th))
        if (on(*th, ISINVIS) && see_monst(th))
            setMapDisplay(th->t_pos.x, th->t_pos.y, th->t_oldch);
    player.t_flags &= ~CANSEE;
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
        player.t_flags &= ~ISBLIND;
        if (!(proom->r_flags & ISGONE))
            enter_room(&hero);
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
    player.t_flags &= ~ISHASTE;
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
            addmsg(choose_str("the munchies overpower your motor capabilities.  ",
                              "you feel too weak from lack of food.  "));
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
        player.t_flags &= ~ISRUN; 
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
    register THING *tp;
    register bool seemonst;

    if (!on(player, ISHALU))
        return 0;

    kill_daemon(visuals);
    player.t_flags &= ~ISHALU;

    if (on(player, ISBLIND))
        return 0;

    /*
     * undo the things
     */
    for (tp = lvl_obj; tp != NULL; tp = next(tp))
        if (cansee(tp->o_pos.y, tp->o_pos.x))
            setMapDisplay(tp->o_pos.x, tp->o_pos.y, tp->o_type);

    /*
     * undo the monsters
     */
    seemonst = on(player, SEEMONST);
    for (tp = mlist; tp != NULL; tp = next(tp))
    {
        if (cansee(tp->t_pos.y, tp->t_pos.x))
        {
            if (!on(*tp, ISINVIS) || on(player, CANSEE))
                setMapDisplay(tp->t_pos.x, tp->t_pos.y, tp->t_disguise);
            else
                setMapDisplay(tp->t_pos.x, tp->t_pos.y, chat(tp->t_pos.y, tp->t_pos.x));
        }
        else if (seemonst)
        {
            setMapDisplay(tp->t_pos.x, tp->t_pos.y, tp->t_type | DISPLAY_INVERT);
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
    register THING *tp;
    register bool seemonst;

    if (!after || (running && jump))
        return 0;
    /*
     * change the things
     */
    for (tp = lvl_obj; tp != NULL; tp = next(tp))
        if (cansee(tp->o_pos.y, tp->o_pos.x))
            setMapDisplay(tp->o_pos.x, tp->o_pos.y, rnd_thing());

    /*
     * change the stairs
     */
    if (!seenstairs && cansee(stairs.y, stairs.x))
        setMapDisplay(stairs.x, stairs.y, rnd_thing());

    /*
     * change the monsters
     */
    seemonst = on(player, SEEMONST);
    for (tp = mlist; tp != NULL; tp = next(tp))
    {
        if (see_monst(tp))
        {
            if (tp->t_type == 'X' && tp->t_disguise != 'X')
                setMapDisplay(tp->t_pos.x, tp->t_pos.y, rnd_thing());
            else
                setMapDisplay(tp->t_pos.x, tp->t_pos.y, rnd(26) + 'A');
        }
        else if (seemonst)
        {
            setMapDisplay(tp->t_pos.x, tp->t_pos.y, (rnd(26) + 'A') | DISPLAY_INVERT);
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
    player.t_flags &= ~ISLEVIT;
    msg(choose_str("bummer!  You've hit the ground",
                   "you float gently to the ground"));
    return 0;
}
