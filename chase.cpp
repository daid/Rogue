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
        if (char_at(x, y) == CLOSED_DOOR)
        {
            if (flags_at(x, y) & F_LOCKED)
                return;
            base_score += 2;
        }else{
            if (!step_ok(char_at(x, y)))
                return;
        }
        if (monster_at(x, y))
            return;
        if (item_at(x, y) && item_at(x, y)->type == SCROLL && item_at(x, y)->which == S_SCARE)
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
        
        co.x = start.x - 1; co.y = start.y - 1; if (diag_ok(&start, &co)) addToList(co.x, co.y, co, 1);
        co.x = start.x + 1; co.y = start.y - 1; if (diag_ok(&start, &co)) addToList(co.x, co.y, co, 1);
        co.x = start.x - 1; co.y = start.y + 1; if (diag_ok(&start, &co)) addToList(co.x, co.y, co, 1);
        co.x = start.x + 1; co.y = start.y + 1; if (diag_ok(&start, &co)) addToList(co.x, co.y, co, 1);
        
        while(open_list.size() > 0)
        {
            int pos = open_list.front();
            open_list.pop_front();
            
            int score = info[pos].base_score;
            coord origin = info[pos].origin;
            
            co.x = pos & 0xFFFF;
            co.y = pos >> 16;
            if (ce(co, end))
                return origin;
            
            addToList(co.x,     co.y - 1, origin, score + 1);
            addToList(co.x - 1, co.y,     origin, score + 1);
            addToList(co.x + 1, co.y,     origin, score + 1);
            addToList(co.x,     co.y + 1, origin, score + 1);
            
            coord ct;
            ct.x = co.x - 1; ct.y = co.y - 1; if (diag_ok(&co, &ct)) addToList(ct.x, ct.y, origin, score + 1);
            ct.x = co.x + 1; ct.y = co.y - 1; if (diag_ok(&co, &ct)) addToList(ct.x, ct.y, origin, score + 1);
            ct.x = co.x - 1; ct.y = co.y + 1; if (diag_ok(&co, &ct)) addToList(ct.x, ct.y, origin, score + 1);
            ct.x = co.x + 1; ct.y = co.y + 1; if (diag_ok(&co, &ct)) addToList(ct.x, ct.y, origin, score + 1);
        }
        return start;
    }
};

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
        /* Move the iterator before we start moving the monster, as the monster might be deleted when moving (L hitting you and stealing your gold for example). */
        MonsterThing* tp = *it++;
        
        if (!on(*tp, ISHELD) && on(*tp, ISRUN))
        {
            check_change_target(tp);
            orig_pos = tp->pos;
            wastarget = on(*tp, ISTARGET);
            if (move_monst(tp))
                continue;
            if (on(*tp, ISFLY) && dist_cp(&hero, &tp->pos) >= 7)
                if(move_monst(tp))
                    continue;
            if (wastarget && !ce(orig_pos, tp->pos))
            {
                tp->flags &= ~ISTARGET;
                to_death = false;
            }
        }
    }
    if (has_hit)
    {
        endmsg();
        has_hit = false;
    }
    return 0;
}

bool check_change_target(MonsterThing* tp)
{
    if ((tp->type == 'O' || tp->type == 'T') && tp->dest == &hero)
    {
        //Orcs and trolls love ham.
        visit_field_of_view(tp->pos.x, tp->pos.y, 1, [tp](int x, int y)
        {
            if (item_at(x, y) && item_at(x, y)->type == FOOD && item_at(x, y)->which == F_CANNED_HAM)
            {
                tp->dest = &item_at(x, y)->pos;
            }
        });
    }
    return false;
}

/*
 * move_monst:
 *        Execute a single turn of running for a monster
 */
bool move_monst(MonsterThing *tp)
{
    tp->turn_delay--;
    if (tp->turn_delay <= 0)
    {
        tp->turn_delay = 1;
        if (on(*tp, ISSLOW))
            tp->turn_delay = 2;
        if (do_chase(tp))
            return true;
    }
    if (on(*tp, ISHASTE))
        if (do_chase(tp))
            return true;
    return false;
}

/*
 * relocate:
 *        Make the monster's new location be the specified one, updating
 *        all the relevant state.
 */
void relocate(MonsterThing *th, coord *new_loc)
{
    if (!step_ok(char_at(new_loc->x, new_loc->y)))
        return;
    if (monster_at(new_loc->x, new_loc->y))
        return;
    if (!ce(*new_loc, th->pos))
    {
        monster_at(th->pos.x, th->pos.y) = nullptr;
        if (see_monst(th))
            setMapDisplay(th->pos.x, th->pos.y, char_at_place(th->pos.x, th->pos.y));

        if (!has_line_of_sight(th->pos.x, th->pos.y, new_loc->x, new_loc->y))
            th->dest = find_dest(th);
        th->pos = *new_loc;
        monster_at(new_loc->x, new_loc->y) = th;
    }
    if (see_monst(th))
    {
        setMapDisplay(new_loc->x, new_loc->y, th->disguise);
    }
    else if (on(player, SEEMONST))
    {
        setMapDisplay(new_loc->x, new_loc->y, th->type | DISPLAY_INVERT);
    }
}

/*
 * do_chase:
 *        Make one thing chase another.
 */
bool do_chase(MonsterThing *th)
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
        if (on(*th, ISHUH))
        {
            //If confused, dragons fire in random directions.
            do
            {
                delta.x = rnd(2) - 1;
                delta.y = rnd(2) - 1;
            }while(delta.x == 0 && delta.y == 0);
        }
        if (has_hit)
            endmsg();
        fire_bolt(&th->pos, &delta, "flame");
        running = false;
        count = 0;
        quiet = 0;
        if (to_death && !on(*th, ISTARGET))
        {
            to_death = false;
            kamikaze = false;
        }
        return false;
    }
        
    coord target = th->pos;
    /*
     * If the thing is confused, let it move randomly. Invisible
     * Stalkers are slightly confused all of the time, and bats are
     * quite confused all the time
     */
    if ((on(*th, ISHUH) && rnd(5) != 0) || (th->type == 'P' && rnd(5) == 0) || (th->type == 'B' && rnd(2) == 0))
    {
        /*
         * get a valid random move
         */
        target = rndmove(th);
        /*
         * Small chance that it will become un-confused 
         */
        if (rnd(20) == 0)
            th->flags &= ~ISHUH;
    }else{
        //Not confused. Use A* to solve where we want to go.
        AStarSolver solver;
        target = solver.solve(th->pos, *th->dest);
    }
    
    /*
     * This now contains what we want to run to this time
     * so we run to it.  If we hit it we either want to fight it
     * or stop running
     */
    if (ce(target, hero))
    {
        return( attack(th) );
    }
    else if(th->type == 'F')
    {
        //venus flytrap never moves
        return false;
    }
    relocate(th, &target);

    if (ce(th->pos, *th->dest))
    {
        ItemThing* obj = item_at(th->dest->x, th->dest->y);
        if (obj && &obj->pos == th->dest)
        {
            lvl_obj.remove(obj);
            item_at(obj->pos.x, obj->pos.y) = nullptr;
            th->pack.push_front(obj);
            th->dest = find_dest(th);
            th->turn_delay = 5;
            if (cansee(obj->pos.y, obj->pos.x))
            {
                msg("%s picked up the %s", set_mname(th), inv_name(obj, false));
            }
        }
    }

    /*
     * And stop running if need be
     */
    if (ce(th->pos, *(th->dest)))
        th->flags &= ~ISRUN;

    return false;
}

/*
 * see_monst:
 *        Return true if the hero can see the monster
 */
bool see_monst(MonsterThing *mp)
{
    if (on(player, ISBLIND))
        return false;
    if (on(*mp, ISINVIS) && !on(player, CANSEE))
        return false;
    return cansee(mp->pos.y, mp->pos.x);
}

/*
 * runto:
 *        Set a monster running after the hero.
 */
void runto(MonsterThing *tp)
{
    /*
     * Start the beastie running
     */
    tp->flags |= ISRUN;
    tp->flags &= ~ISHELD;
    tp->dest = find_dest(tp);
}

/*
 * diag_ok:
 *        Check to see if the move is legal if it is diagonal
 */
bool diag_ok(coord *sp, coord *ep)
{
    if (ep->x < 0 || ep->x >= NUMCOLS || ep->y <= 0 || ep->y >= NUMLINES - 1)
        return false;
    if (ep->x == sp->x || ep->y == sp->y)
        return true;
    return (bool)(step_ok(char_at(ep->x, sp->y)) && step_ok(char_at(sp->x, ep->y)));
}

/*
 * cansee:
 *        Returns true if the hero can see a certain coordinate.
 */
bool cansee(int y, int x)
{
    if (on(player, ISBLIND))
        return false;
    if (!has_line_of_sight(hero.x, hero.y, x, y))
        return false;
    if (dist(y, x, hero.y, hero.x) < LAMPDIST)
        return true;
    if (flags_at(x, y) & F_ISLIT)
        return true;
    return false;
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
            if (tp == nullptr)
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
int dist(int y1, int x1, int y2, int x2)
{
    return ((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

/*
 * dist_cp:
 *        Call dist() with appropriate arguments for coord pointers
 */
int dist_cp(coord *c1, coord *c2)
{
    return dist(c1->y, c1->x, c2->y, c2->x);
}
