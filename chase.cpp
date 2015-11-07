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

#include <map>
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
            if (on(*tp, ISFLY) && dist_cp(&hero, &tp->pos) >= 7)
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
    if (!ce(*new_loc, th->pos))
    {
        if (see_monst(th))
            setMapDisplay(th->pos.x, th->pos.y, char_at_place(th->pos.x, th->pos.y));
        monster_at(th->pos.x, th->pos.y) = NULL;

        if (has_line_of_sight(th->pos.x, th->pos.y, new_loc->x, new_loc->y))
            th->dest = find_dest(th);
        th->pos = *new_loc;
        monster_at(new_loc->x, new_loc->y) = th;
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
    /*
     * For dragons check and see if (a) the hero is on a straight
     * line from it, and (b) that it is within shooting distance,
     * but outside of striking range.
     */
    if (th->type == 'D' && (th->pos.y == hero.y || th->pos.x == hero.x || abs(th->pos.y - hero.y) == abs(th->pos.x - hero.x))
        && dist_cp(&th->pos, &hero) <= BOLT_LENGTH * BOLT_LENGTH && !on(*th, ISCANC) && rnd(DRAGONSHOT) == 0 &&
        has_line_of_sight(hero.x, hero.y, th->pos.x, th->pos.y))
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
    
    class AStarSolver
    {
        class AStarInfo
        {
        public:
            coord origin;
            int base_score;
        };
        coord end;
        
        std::map<int, AStarInfo> info;
        std::list<int> open_list;
        
        int heuristicScore(int x, int y)
        {
            return abs(x - end.x) + abs(y - end.y);
        }
        
        void addToList(int x, int y, coord origin, int base_score)
        {
            if (x < 0 || x >= NUMCOLS || y < 0 || y >= NUMLINES)
                return;
            if (!step_ok(char_at(x, y)))
                return;
            if (monster_at(x, y))
                return;
            int pos = x | y << 16;
            if (info.find(pos) != info.end())
            {
                if (info[pos].base_score <= base_score)
                    return;
            }
            info[pos].base_score = base_score;
            info[pos].origin = origin;
            
            int score = heuristicScore(x, y) + base_score;
            for(auto it=open_list.begin(); it != open_list.end(); it++)
            {
                int p = *it;
                int s = info[p].base_score + heuristicScore(p & 0xFFFF, p >> 16);
                if (score < s)
                {
                    open_list.insert(it, pos);
                    return;
                }
            }
            open_list.push_back(pos);
        }
    
    public:
        coord solve(coord start, coord end)
        {
            this->end = end;
            
            coord co;
            co.x = start.x;     co.y = start.y - 1; addToList(co.x, co.y, co, 1);
            co.x = start.x - 1; co.y = start.y;     addToList(co.x, co.y, co, 1);
            co.x = start.x + 1; co.y = start.y;     addToList(co.x, co.y, co, 1);
            co.x = start.x;     co.y = start.y + 1; addToList(co.x, co.y, co, 1);
            
            co.x = start.x - 1; co.y = start.y - 1; addToList(co.x, co.y, co, 1);
            co.x = start.x + 1; co.y = start.y - 1; addToList(co.x, co.y, co, 1);
            co.x = start.x - 1; co.y = start.y + 1; addToList(co.x, co.y, co, 1);
            co.x = start.x + 1; co.y = start.y + 1; addToList(co.x, co.y, co, 1);
            
            while(open_list.size() > 0)
            {
                int pos = open_list.front();
                open_list.pop_front();
                
                int score = info[pos].base_score;
                coord origin = info[pos].origin;
                
                coord co = {pos & 0xFFFF, pos >> 16};
                if (ce(co, end))
                    return origin;
                
                addToList(co.x,     co.y - 1, origin, score + 1);
                addToList(co.x - 1, co.y,     origin, score + 1);
                addToList(co.x + 1, co.y,     origin, score + 1);
                addToList(co.x,     co.y + 1, origin, score + 1);
                
                addToList(co.x - 1, co.y - 1, origin, score + 1);
                addToList(co.x + 1, co.y - 1, origin, score + 1);
                addToList(co.x - 1, co.y + 1, origin, score + 1);
                addToList(co.x + 1, co.y + 1, origin, score + 1);
            }
            return start;
        }
    };
    
    AStarSolver solver;
    coord target = solver.solve(th->pos, *th->dest);
    
    bool stoprun = false;
    /*
     * This now contains what we want to run to this time
     * so we run to it.  If we hit it we either want to fight it
     * or stop running
     */
    if (!chase(th, &target))
    {
        if (ce(target, hero))
        {
            return( attack(th) );
        }
        else if (ce(target, *th->dest))
        {
            ItemThing* obj = item_at(th->dest->x, th->dest->y);
            if (obj && &obj->pos == th->dest)
            {
                lvl_obj.remove(obj);
                item_at(obj->pos.x, obj->pos.y) = nullptr;
                th->pack.push_front(obj);
                th->dest = find_dest(th);
            }
            if (th->type != 'F')
                stoprun = true;
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
    tp = monster_at(runner.x, runner.y);
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
bool chase(MonsterThing *tp, coord *ee)
{
    int x, y;
    int curdist, thisdist;
    coord *er = &tp->pos;
    int plcnt = 1;
    static coord tryp;

    /*
     * If the thing is confused, let it move randomly. Invisible
     * Stalkers are slightly confused all of the time, and bats are
     * quite confused all the time
     */
    if ((on(*tp, ISHUH) && rnd(5) != 0) || (tp->type == 'P' && rnd(5) == 0) || (tp->type == 'B' && rnd(2) == 0))
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
                if (step_ok(char_at(x, y)) && !monster_at(x, y))
                {
                    /*
                     * If it is a scroll, it might be a scare monster scroll
                     * so we need to look it up to see what type it is.
                     */
                    if (item_at(x, y))
                    {
                        ItemThing* obj = item_at(x, y);
                        if (obj != NULL  && obj->type == SCROLL && obj->which == S_SCARE)
                            continue;
                    }
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
    if (on(player, ISBLIND))
        return FALSE;
    if (!has_line_of_sight(hero.x, hero.y, x, y))
        return FALSE;
    if (dist(y, x, hero.y, hero.x) < LAMPDIST)
        return TRUE;
    if (flat(y, x) & F_ISLIT)
        return TRUE;
    return FALSE;
}

/*
 * find_dest:
 *        find the proper destination for the monster
 */
coord* find_dest(MonsterThing *tp)
{
    
    int prob;

    if ((prob = monsters[tp->type - 'A'].m_carry) <= 0 || has_line_of_sight(hero.x, hero.y, tp->pos.x, tp->pos.y))
        return &hero;
    for(ItemThing* obj : lvl_obj)
    {
        if (obj->type == SCROLL && obj->which == S_SCARE)
            continue;
        if (has_line_of_sight(obj->pos.x, obj->pos.y, tp->pos.x, tp->pos.y) && rnd(100) < prob)
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
