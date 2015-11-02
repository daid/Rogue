/*
 * Code for one creature to chase another
 *
 * @(#)chase.c        4.57 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include "rogue.h"

#define DRAGONSHOT  5        /* one chance in DRAGONSHOT that a dragon will flame */

static coord ch_ret;                                /* Where chasing takes you */

/*
 * runners:
 *        Make all the running monsters move.
 */
int runners(int arg)
{
    bool wastarget;
    static coord orig_pos;

    for(auto it = mlist.begin(); it != mlist.end(); )
    {
        /* Move the iterator before we start moving the monster, as the monster might die due to movement. */
        MonsterThing* tp = *it++;
        
        if (!on(*tp, ISHELD) && on(*tp, ISRUN))
        {
            orig_pos = tp->pos;
            wastarget = on(*tp, ISTARGET);
            if (move_monst(tp) == -1)
                continue;
            if (on(*tp, ISFLY) && dist_cp(&hero, &tp->pos) >= 3)
                move_monst(tp);
            if (wastarget && !ce(orig_pos, tp->pos))
            {
                tp->flags &= ~ISTARGET;
                to_death = FALSE;
            }
        }
    }
    if (has_hit)
    {
        endmsg();
        has_hit = FALSE;
    }
    return 0;
}

/*
 * move_monst:
 *        Execute a single turn of running for a monster
 */
int move_monst(MonsterThing *tp)
{
    if (!on(*tp, ISSLOW) || tp->turn)
        if (do_chase(tp) == -1)
            return(-1);
    if (on(*tp, ISHASTE))
        if (do_chase(tp) == -1)
            return(-1);
    tp->turn = !tp->turn;
    return(0);
}

/*
 * relocate:
 *        Make the monster's new location be the specified one, updating
 *        all the relevant state.
 */
void relocate(MonsterThing *th, coord *new_loc)
{
    struct room *oroom;

    if (!ce(*new_loc, th->pos))
    {
        if (see_monst(th))
            setMapDisplay(th->pos.x, th->pos.y, winat(th->pos.y, th->pos.x));
        th->room = roomin(*new_loc);
        oroom = th->room;
        moat(th->pos.y, th->pos.x) = NULL;

        if (oroom != th->room)
            th->dest = find_dest(th);
        th->pos = *new_loc;
        moat(new_loc->y, new_loc->x) = th;
    }
    if (see_monst(th))
        setMapDisplay(new_loc->x, new_loc->y, th->disguise);
    else if (on(player, SEEMONST))
    {
        setMapDisplay(new_loc->x, new_loc->y, th->type | DISPLAY_INVERT);
    }
}

/*
 * do_chase:
 *        Make one thing chase another.
 */
int do_chase(MonsterThing *th)
{
    coord *cp;
    struct room *rer, *ree;        /* room of chaser, room of chasee */
    int mindist = 32767, curdist;
    bool stoprun = FALSE;        /* TRUE means we are there */
    bool door;
    static coord _this;                        /* Temporary destination for chaser */

    rer = th->room;                /* Find room of chaser */
    if (on(*th, ISGREED) && rer->r_goldval == 0)
        th->dest = &hero;        /* If gold has been taken, run after hero */
    if (th->dest == &hero)        /* Find room of chasee */
        ree = player.room;
    else
        ree = roomin(*th->dest);
    /*
     * We don't count doors as inside rooms for this routine
     */
    door = (char_at(th->pos.x, th->pos.y) == DOOR);
    /*
     * If the object of our desire is in a different room,
     * and we are not in a corridor, run to the door nearest to
     * our goal.
     */
over:
    if (rer != ree)
    {
        for (cp = rer->r_exit; cp < &rer->r_exit[rer->r_nexits]; cp++)
        {
            curdist = dist_cp(th->dest, cp);
            if (curdist < mindist)
            {
                _this = *cp;
                mindist = curdist;
            }
        }
        if (door)
        {
            rer = &passages[flat(th->pos.y, th->pos.x) & F_PNUM];
            door = FALSE;
            goto over;
        }
    }
    else
    {
        _this = *th->dest;
        /*
         * For dragons check and see if (a) the hero is on a straight
         * line from it, and (b) that it is within shooting distance,
         * but outside of striking range.
         */
        if (th->type == 'D' && (th->pos.y == hero.y || th->pos.x == hero.x
            || abs(th->pos.y - hero.y) == abs(th->pos.x - hero.x))
            && dist_cp(&th->pos, &hero) <= BOLT_LENGTH * BOLT_LENGTH
            && !on(*th, ISCANC) && rnd(DRAGONSHOT) == 0)
        {
            delta.y = sign(hero.y - th->pos.y);
            delta.x = sign(hero.x - th->pos.x);
            if (has_hit)
                endmsg();
            fire_bolt(&th->pos, &delta, "flame");
            running = FALSE;
            count = 0;
            quiet = 0;
            if (to_death && !on(*th, ISTARGET))
            {
                to_death = FALSE;
                kamikaze = FALSE;
            }
            return(0);
        }
    }
    /*
     * This now contains what we want to run to this time
     * so we run to it.  If we hit it we either want to fight it
     * or stop running
     */
    if (!chase(th, &_this))
    {
        if (ce(_this, hero))
        {
            return( attack(th) );
        }
        else if (ce(_this, *th->dest))
        {
            for (ItemThing* obj : lvl_obj)
            {
                if (th->dest == &obj->pos)
                {
                    lvl_obj.remove(obj);
                    item_at(obj->pos.x, obj->pos.y) = nullptr;
                    th->pack.push_front(obj);
                    th->dest = find_dest(th);
                    break;
                }
            }
            if (th->type != 'F')
                stoprun = TRUE;
        }
    }
    else
    {
        if (th->type == 'F')
            return(0);
    }
    relocate(th, &ch_ret);
    /*
     * And stop running if need be
     */
    if (stoprun && ce(th->pos, *(th->dest)))
        th->flags &= ~ISRUN;
    return(0);
}

/*
 * see_monst:
 *        Return TRUE if the hero can see the monster
 */
bool see_monst(MonsterThing *mp)
{
    if (on(player, ISBLIND))
        return FALSE;
    if (on(*mp, ISINVIS) && !on(player, CANSEE))
        return FALSE;
    return cansee(mp->pos.y, mp->pos.x);
}

/*
 * runto:
 *        Set a monster running after the hero.
 */
void runto(const coord& runner)
{
    MonsterThing *tp;

    /*
     * If we couldn't find him, something is funny
     */
    tp = moat(runner.y, runner.x);
    /*
     * Start the beastie running
     */
    tp->flags |= ISRUN;
    tp->flags &= ~ISHELD;
    tp->dest = find_dest(tp);
}

/*
 * chase:
 *        Find the spot for the chaser(er) to move closer to the
 *        chasee(ee).  Returns TRUE if we want to keep on chasing later
 *        FALSE if we reach the goal.
 */
bool
chase(MonsterThing *tp, coord *ee)
{
    int x, y;
    int curdist, thisdist;
    coord *er = &tp->pos;
    int ch;
    int plcnt = 1;
    static coord tryp;

    /*
     * If the thing is confused, let it move randomly. Invisible
     * Stalkers are slightly confused all of the time, and bats are
     * quite confused all the time
     */
    if ((on(*tp, ISHUH) && rnd(5) != 0) || (tp->type == 'P' && rnd(5) == 0)
        || (tp->type == 'B' && rnd(2) == 0))
    {
        /*
         * get a valid random move
         */
        ch_ret = *rndmove(tp);
        curdist = dist_cp(&ch_ret, ee);
        /*
         * Small chance that it will become un-confused 
         */
        if (rnd(20) == 0)
            tp->flags &= ~ISHUH;
    }
    /*
     * Otherwise, find the empty spot next to the chaser that is
     * closest to the chasee.
     */
    else
    {
        register int ey, ex;
        /*
         * This will eventually hold where we move to get closer
         * If we can't find an empty spot, we stay where we are.
         */
        curdist = dist_cp(er, ee);
        ch_ret = *er;

        ey = er->y + 1;
        if (ey >= NUMLINES - 1)
            ey = NUMLINES - 2;
        ex = er->x + 1;
        if (ex >= NUMCOLS)
            ex = NUMCOLS - 1;

        for (x = er->x - 1; x <= ex; x++)
        {
            if (x < 0)
                continue;
            tryp.x = x;
            for (y = er->y - 1; y <= ey; y++)
            {
                tryp.y = y;
                if (!diag_ok(er, &tryp))
                    continue;
                ch = winat(y, x);
                if (step_ok(ch))
                {
                    /*
                     * If it is a scroll, it might be a scare monster scroll
                     * so we need to look it up to see what type it is.
                     */
                    if (ch == SCROLL)
                    {
                        ItemThing* obj = find_obj(y, x);
                        if (obj != NULL && obj->which == S_SCARE)
                            continue;
                    }
                    /*
                     * It can also be a Xeroc, which we shouldn't step on
                     */
                    if (moat(y, x) != NULL && moat(y, x)->type == 'X')
                        continue;
                    /*
                     * If we didn't find any scrolls at this place or it
                     * wasn't a scare scroll, then this place counts
                     */
                    thisdist = dist(y, x, ee->y, ee->x);
                    if (thisdist < curdist)
                    {
                        plcnt = 1;
                        ch_ret = tryp;
                        curdist = thisdist;
                    }
                    else if (thisdist == curdist && rnd(++plcnt) == 0)
                    {
                        ch_ret = tryp;
                        curdist = thisdist;
                    }
                }
            }
        }
    }
    return (bool)(curdist != 0 && !ce(ch_ret, hero));
}

/*
 * roomin:
 *        Find what room some coordinates are in. NULL means they aren't
 *        in any room.
 */
struct room * roomin(const coord& cp)
{
    register struct room *rp;
    register char *fp;


    fp = &flat(cp.y, cp.x);
    if (*fp & F_PASS)
        return &passages[*fp & F_PNUM];

    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
        if (cp.x <= rp->r_pos.x + rp->r_max.x && rp->r_pos.x <= cp.x
         && cp.y <= rp->r_pos.y + rp->r_max.y && rp->r_pos.y <= cp.y)
            return rp;

    return NULL;
}

/*
 * diag_ok:
 *        Check to see if the move is legal if it is diagonal
 */
bool diag_ok(coord *sp, coord *ep)
{
    if (ep->x < 0 || ep->x >= NUMCOLS || ep->y <= 0 || ep->y >= NUMLINES - 1)
        return FALSE;
    if (ep->x == sp->x || ep->y == sp->y)
        return TRUE;
    return (bool)(step_ok(char_at(ep->x, sp->y)) && step_ok(char_at(sp->x, ep->y)));
}

/*
 * cansee:
 *        Returns true if the hero can see a certain coordinate.
 */
bool cansee(int y, int x)
{
    register struct room *rer;
    static coord tp;

    if (on(player, ISBLIND))
        return FALSE;
    if (!has_line_of_sight(hero.x, hero.y, x, y))
        return FALSE;
    if (dist(y, x, hero.y, hero.x) < LAMPDIST)
        return TRUE;
    tp.y = y;
    tp.x = x;
    rer = roomin(tp);
    return (bool)(!(rer->r_flags & ISDARK));
}

/*
 * find_dest:
 *        find the proper destination for the monster
 */
coord* find_dest(MonsterThing *tp)
{
    
    int prob;

    if ((prob = monsters[tp->type - 'A'].m_carry) <= 0 || tp->room == player.room || see_monst(tp))
        return &hero;
    for(ItemThing* obj : lvl_obj)
    {
        if (obj->type == SCROLL && obj->which == S_SCARE)
            continue;
        if (roomin(obj->pos) == tp->room && rnd(100) < prob)
        {
            for(MonsterThing* mt : mlist)
            {
                if (mt->dest == &obj->pos)
                {
                    tp = mt;
                    break;
                }
            }
            if (tp == NULL)
                return &obj->pos;
        }
    }
    return &hero;
}

/*
 * dist:
 *        Calculate the "distance" between to points.  Actually,
 *        this calculates d^2, not d, but that's good enough for
 *        our purposes, since it's only used comparitively.
 */
int
dist(int y1, int x1, int y2, int x2)
{
    return ((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/*
 * dist_cp:
 *        Call dist() with appropriate arguments for coord pointers
 */
int
dist_cp(coord *c1, coord *c2)
{
    return dist(c1->y, c1->x, c2->y, c2->x);
}
