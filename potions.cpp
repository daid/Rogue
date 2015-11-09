/*
 * Function(s) for dealing with potions
 *
 * @(#)potions.c        4.46 (Berkeley) 06/07/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <ctype.h>
#include "rogue.h"

typedef struct
{
    int pa_flags;
    daemon_function_t pa_daemon;
    int pa_time;
    const char *pa_high, *pa_straight;
} PACT;

static PACT p_actions[] =
{
        { ISHUH,        unconfuse,        HUHDURATION,        /* P_CONFUSE */
                "what a tripy feeling!",
                "wait, what's going on here. Huh? What? Who?" },
        { ISHALU,        come_down,        SEEDURATION,        /* P_LSD */
                "Oh, wow!  Everything seems so cosmic!",
                "Oh, wow!  Everything seems so cosmic!" },
        { 0,                NULL,        0 },                        /* P_POISON */
        { 0,                NULL,        0 },                        /* P_STRENGTH */
        { CANSEE,        unsee,        SEEDURATION,                /* P_SEEINVIS */
                prbuf,
                prbuf },
        { 0,                NULL,        0 },                        /* P_HEALING */
        { 0,                NULL,        0 },                        /* P_MFIND */
        { 0,                NULL,        0 },                        /* P_TFIND  */
        { 0,                NULL,        0 },                        /* P_RAISE */
        { 0,                NULL,        0 },                        /* P_XHEAL */
        { 0,                NULL,        0 },                        /* P_HASTE */
        { 0,                NULL,        0 },                        /* P_RESTORE */
        { ISBLIND,        sight,        SEEDURATION,                /* P_BLIND */
                "oh, bummer!  Everything is dark!  Help!",
                "a cloak of darkness falls around you" },
        { ISLEVIT,        land,        HEALTIME,                /* P_LEVIT */
                "oh, wow!  You're floating in the air!",
                "you start to float in the air" }
};

/*
 * quaff:
 *        Quaff a potion from the pack
 */

void quaff()
{
    ItemThing *obj;
    bool discardit = FALSE;
    bool show, trip;

    obj = get_item("quaff", POTION);
    /*
     * Make certain that it is somethings that we want to drink
     */
    if (obj == NULL)
        return;
    if (obj->type != POTION)
    {
        if (!terse)
            msg("yuk! Why would you want to drink that?");
        else
            msg("that's undrinkable");
        return;
    }
    if (obj == cur_weapon)
        cur_weapon = NULL;

    //Add some food for drinking a potion
    add_food(50 + rnd(100));
    /*
     * Calculate the effect it has on the poor guy.
     */
    trip = on(player, ISHALU);
    discardit = (bool)(obj->count == 1);
    leave_pack(obj, FALSE, FALSE);
    switch (obj->which)
    {
        case P_CONFUSE:
            do_pot(P_CONFUSE, !trip);
        when P_POISON:
            pot_info[P_POISON].oi_know = TRUE;
            if (ISWEARING(R_SUSTSTR))
                msg("you feel momentarily sick");
            else
            {
                chg_str(-(rnd(3) + 1));
                msg("you feel very sick now");
                come_down(0);
            }
        when P_HEALING:
            pot_info[P_HEALING].oi_know = TRUE;
            if ((player.stats.s_hpt += roll(player.stats.s_lvl, 4)) > player.stats.s_maxhp)
                player.stats.s_hpt = ++player.stats.s_maxhp;
            sight(0);
            msg("you begin to feel better");
        when P_STRENGTH:
            pot_info[P_STRENGTH].oi_know = TRUE;
            chg_str(1);
            msg("you feel stronger, now.  What bulging muscles!");
        when P_MFIND:
            player.flags |= SEEMONST;
            fuse(turn_see, TRUE, HUHDURATION, AFTER);
            if (!turn_see(FALSE))
                msg("you have a %s feeling for a moment, then it passes",
                    choose_str("normal", "strange"));
        when P_TFIND:
            /*
             * Potion of magic detection.  Show the potions and scrolls
             */
            show = FALSE;
            for(ItemThing* tp : lvl_obj)
            {
                if (is_magic(tp))
                {
                    show = TRUE;
                    setMapDisplay(tp->pos.x, tp->pos.y, MAGIC);
                    pot_info[P_TFIND].oi_know = TRUE;
                }
            }
            for (MonsterThing* mp : mlist)
            {
                for(ItemThing* tp : mp->pack)
                {
                    if (is_magic(tp))
                    {
                        show = TRUE;
                        setMapDisplay(mp->pos.x, mp->pos.y, MAGIC);
                    }
                }
            }
            if (show)
            {
                pot_info[P_TFIND].oi_know = TRUE;
                displayMessage("You sense the presence of magic on this level.");
            }
            else
                msg("you have a %s feeling for a moment, then it passes",
                    choose_str("normal", "strange"));
        when P_LSD:
            if (!trip)
            {
                if (on(player, SEEMONST))
                    turn_see(FALSE);
                start_daemon(visuals, 0, BEFORE);
                seenstairs = seen_stairs();
            }
            do_pot(P_LSD, TRUE);
        when P_SEEINVIS:
            sprintf(prbuf, "this potion tastes like %s juice", fruit);
            show = on(player, CANSEE);
            do_pot(P_SEEINVIS, FALSE);
            if (!show)
                invis_on();
            sight(0);
        when P_RAISE:
            pot_info[P_RAISE].oi_know = TRUE;
            msg("you suddenly feel much more skillful");
            raise_level();
        when P_XHEAL:
            pot_info[P_XHEAL].oi_know = TRUE;
            if ((player.stats.s_hpt += roll(player.stats.s_lvl, 8)) > player.stats.s_maxhp)
            {
                if (player.stats.s_hpt > player.stats.s_maxhp + player.stats.s_lvl + 1)
                    ++player.stats.s_maxhp;
                player.stats.s_hpt = ++player.stats.s_maxhp;
            }
            sight(0);
            come_down(0);
            msg("you begin to feel much better");
        when P_HASTE:
            pot_info[P_HASTE].oi_know = TRUE;
            after = FALSE;
            if (add_haste(TRUE))
                msg("you feel yourself moving much faster");
        when P_RESTORE:
            if (ISRING(LEFT, R_ADDSTR))
                add_str(&player.stats.s_str, -cur_ring[LEFT]->arm);
            if (ISRING(RIGHT, R_ADDSTR))
                add_str(&player.stats.s_str, -cur_ring[RIGHT]->arm);
            if (player.stats.s_str < max_stats.s_str)
                player.stats.s_str = max_stats.s_str;
            if (ISRING(LEFT, R_ADDSTR))
                add_str(&player.stats.s_str, cur_ring[LEFT]->arm);
            if (ISRING(RIGHT, R_ADDSTR))
                add_str(&player.stats.s_str, cur_ring[RIGHT]->arm);
            msg("hey, this tastes great.  It make you feel warm all over");
        when P_BLIND:
            do_pot(P_BLIND, TRUE);
        when P_LEVIT:
            do_pot(P_LEVIT, TRUE);
    }
    status();
    /*
     * Throw the item away
     */

    call_it(&pot_info[obj->which]);

    if (discardit)
        delete obj;
    return;
}

/*
 * is_magic:
 *        Returns true if an object radiates magic
 */
bool is_magic(ItemThing *obj)
{
    switch (obj->type)
    {
        case ARMOR:
            return (bool)((obj->flags&ISPROT) || obj->arm != a_class[obj->which]);
        case WEAPON:
            return (bool)(obj->hplus != 0 || obj->dplus != 0);
        case SCROLL:
            return obj->which != S_HINT;
        case POTION:
        case STICK:
        case RING:
        case AMULET:
            return TRUE;
    }
    return FALSE;
}

/*
 * invis_on:
 *        Turn on the ability to see invisible
 */

void invis_on()
{
    player.flags |= CANSEE;
    for(MonsterThing *mp : mlist)
        if (on(*mp, ISINVIS) && see_monst(mp) && !on(player, ISHALU))
            setMapDisplay(mp->pos.x, mp->pos.y, mp->disguise);
}

/*
 * turn_see:
 *        Put on or off seeing monsters on this level
 */
int turn_see(int turn_off)
{
    bool can_see, add_new;

    add_new = FALSE;
    for (MonsterThing* mp : mlist)
    {
        can_see = see_monst(mp);
        if (turn_off)
        {
            if (!can_see)
                setMapDisplay(mp->pos.x, mp->pos.y, char_at(mp->pos.x, mp->pos.y));
        }
        else
        {
            if (!can_see)
            {
                if (!on(player, ISHALU))
                    setMapDisplay(mp->pos.x, mp->pos.y, mp->type | DISPLAY_INVERT);
                else
                    setMapDisplay(mp->pos.x, mp->pos.y, (rnd(26) + 'A')  | DISPLAY_INVERT);
                add_new++;
            }else{
                if (!on(player, ISHALU))
                    setMapDisplay(mp->pos.x, mp->pos.y, mp->type);
                else
                    setMapDisplay(mp->pos.x, mp->pos.y, rnd(26) + 'A');
            }
        }
    }
    if (turn_off)
        player.flags &= ~SEEMONST;
    else
        player.flags |= SEEMONST;
    return add_new;
}

/*
 * seen_stairs:
 *        Return TRUE if the player has seen the stairs, called when starting to hallucinate
 */
bool seen_stairs()
{
    MonsterThing        *tp;

    if (getMapDisplay(stairs.x, stairs.y) == STAIRS)                        /* it's on the map */
        return TRUE;
    if (ce(hero, stairs))                        /* It's under him */
        return TRUE;

    /*
     * if a monster is on the stairs, this gets hairy
     */
    if ((tp = monster_at(stairs.x, stairs.y)) != NULL)
    {
        if (see_monst(tp) && on(*tp, ISRUN))        /* if it's visible and awake */
            return TRUE;                        /* it must have moved there */

        if (on(player, SEEMONST))                /* if she can detect monster */
            return TRUE;                        /* it must have moved there */
    }
    return FALSE;
}

/*
 * raise_level:
 *        The guy just magically went up a level.
 */

void raise_level()
{
    player.stats.s_exp = e_levels[player.stats.s_lvl-1] + 1L;
    check_level();
}

/*
 * do_pot:
 *        Do a potion with standard setup.  This means it uses a fuse and
 *        turns on a flag
 */

void do_pot(int type, bool knowit)
{
    PACT *pp;
    int t;

    pp = &p_actions[type];
    if (!pot_info[type].oi_know)
        pot_info[type].oi_know = knowit;
    t = spread(pp->pa_time);
    if (!on(player, pp->pa_flags))
    {
        player.flags |= pp->pa_flags;
        fuse(pp->pa_daemon, 0, t, AFTER);
        look(FALSE);
    }
    else
        lengthen(pp->pa_daemon, t);
    msg(choose_str(pp->pa_high, pp->pa_straight));
}

void hit_by_potion(int type, MonsterThing* mp)
{
    switch(type)
    {
    case P_CONFUSE:
        if (!save_throw(VS_POISON, mp))
        {
            mp->flags |= ISHUH;
        }
    when P_LSD:
        if (!save_throw(VS_POISON, mp))
        {
            mp->flags |= ISHUH;
        }
    when P_POISON:
        if (!save_throw(VS_POISON, mp))
        {
            //Hit by a potion of poison. Do 1d6 damage. Plus 1d3 strength damage.
            mp->stats.s_hpt -= roll(1, 6);
            mp->stats.s_str -= roll(1, 3);
            msg("%s melts from the liquid", set_mname(mp));
        }
    when P_STRENGTH:
        if (!save_throw(VS_POISON, mp))
        {
            //Hit by a potion of strength, add a strength and tell the thrower that this was a bad move.
            mp->stats.s_str += 1;
            msg("%s looks stronger now...", set_mname(mp));
        }
    when P_SEEINVIS:
    when P_HEALING:
        if (!save_throw(VS_POISON, mp))
        {
            if ((mp->stats.s_hpt += roll(mp->stats.s_lvl, 4)) > mp->stats.s_maxhp)
                mp->stats.s_hpt = ++mp->stats.s_maxhp;
            msg("%s looks refreshed", set_mname(mp));
        }
    when P_MFIND:
    when P_TFIND:
    when P_RAISE:
    when P_XHEAL:
        if (!save_throw(VS_POISON, mp))
        {
            if ((mp->stats.s_hpt += roll(mp->stats.s_lvl, 8)) > mp->stats.s_maxhp)
                mp->stats.s_hpt = ++mp->stats.s_maxhp;
            msg("%s looks very refreshed", set_mname(mp));
        }
    when P_HASTE:
    when P_RESTORE:
    when P_BLIND:
    when P_LEVIT:
        break;
    }
}
