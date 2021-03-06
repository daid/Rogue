/*
 * File with various monster functions in it
 *
 * @(#)monsters.c        4.46 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <assert.h>
#include <string.h>
#include "rogue.h"
#include "areas.h"
#include <ctype.h>

/*
 * List of monsters in rough order of vorpalness
 */
static char lvl_mons[] =  {
    'K', 'E', 'B', 'S', 'H', 'I', 'R', 'O', 'Z', 'L', 'C', 'Q', 'A',
    'N', 'Y', 'F', 'T', 'W', 'P', 'X', 'U', 'M', 'V', 'G', 'J', 'D'
};

static char wand_mons[] = {
    'K', 'E', 'B', 'S', 'H',   0, 'R', 'O', 'Z',   0, 'C', 'Q', 'A',
      0, 'Y',   0, 'T', 'W', 'P',   0, 'U', 'M', 'V', 'G', 'J',   0
};

/*
 * randmonster:
 *        Pick a monster to show up.  The lower the level,
 *        the meaner the monster.
 */
char randmonster(bool wander)
{
    int d;
    char *mons;

    mons = (wander ? wand_mons : lvl_mons);
    do
    {
        d = level + (rnd(10) - 6);
        if (d < 0)
            d = rnd(5);
        if (d > 25)
            d = rnd(5) + 21;
    } while (mons[d] == 0);
    return mons[d];
}

MonsterThing::MonsterThing()
{
    pos.x = pos.y = 0;                        /* Position */
    turn_delay = 0;
    type = 0;                        /* What it is */
    disguise = 0;                /* What mimic looks like */
    dest = nullptr;                        /* Where it is running to */
    flags = 0;                        /* State word */
    stats.s_str = 0;                        /* Strength */
    stats.s_exp = 0;                                /* Experience */
    stats.s_lvl = 0;                                /* level of mastery */
    stats.s_arm = 0;                                /* Armor class */
    stats.s_hpt = 0;                        /* Hit points */
    stats.s_dmg[0] = 0;                        /* String describing damage done */
    stats.s_maxhp = 0;                        /* Max hit points */
    reserved = 0;
}

/*
 * new_monster:
 *        Pick a new monster and add it to the list
 */
MonsterThing::MonsterThing(char type, const coord& cp)
{
    int lev_add;
    struct monster* mp;

    if ((lev_add = level - AMULETLEVEL) < 0)
        lev_add = 0;
    mlist.push_back(this);
    this->type = type;
    this->disguise = type;
    this->pos = cp;
    monster_at(cp.x, cp.y) = this;
    mp = &monsters[this->type-'A'];
    this->stats.s_lvl = mp->m_stats.s_lvl + lev_add;
    this->stats.s_maxhp = this->stats.s_hpt = roll(this->stats.s_lvl, 8);
    this->stats.s_arm = mp->m_stats.s_arm - lev_add;
    strcpy(this->stats.s_dmg,mp->m_stats.s_dmg);
    this->stats.s_str = mp->m_stats.s_str;
    this->stats.s_exp = mp->m_stats.s_exp + lev_add * 10 + experienceAdd();
    this->flags = mp->m_flags;
    if (level > 29)
        this->flags |= ISHASTE;
    this->turn_delay = 0;
    this->dest = nullptr;
    if (ISWEARING(R_AGGR))
        runto(this);
    if (type == 'X')
        this->disguise = rnd_thing();
    this->reserved = 0;
}

void MonsterThing::polymorph(char new_type)
{
    struct monster* old_mp = &monsters[type-'A'];
    struct monster* new_mp = &monsters[new_type-'A'];
    
    //Change this monster into a different kind of monster.
    type = new_type;
    disguise = type;
    //Change all the stats to reflect the new monster. Use the previous monster template to find how to adjust the stats.
    stats.s_lvl = new_mp->m_stats.s_lvl + (stats.s_lvl - old_mp->m_stats.s_lvl);
    stats.s_arm = new_mp->m_stats.s_arm + (stats.s_arm - old_mp->m_stats.s_arm);
    strcpy(this->stats.s_dmg, new_mp->m_stats.s_dmg);
    stats.s_str = new_mp->m_stats.s_str;
    stats.s_exp = new_mp->m_stats.s_exp + (stats.s_lvl - new_mp->m_stats.s_lvl) * 10 + experienceAdd();
    //Keep all the flags that where not set by the monster template.
    flags &=~old_mp->m_flags;
    flags |= new_mp->m_flags;
    //Adjust the hitpoints to a new value, keep the hitpoints amount at about the same faction, rounded up.
    int new_maxhp = roll(stats.s_lvl, 8);
    stats.s_hpt = ((stats.s_hpt * new_maxhp) + stats.s_maxhp - 1) / stats.s_maxhp;
    stats.s_maxhp = new_maxhp;
    
    if (type == 'X')
        disguise = rnd_thing();
}

/*
 * expadd:
 *        Experience to add for this monster's level/hit points
 */
int MonsterThing::experienceAdd()
{
    int mod;

    if (stats.s_lvl == 1)
        mod = stats.s_maxhp / 8;
    else
        mod = stats.s_maxhp / 6;
    if (stats.s_lvl > 9)
        mod *= 20;
    else if (stats.s_lvl > 6)
        mod *= 4;
    return mod;
}

/*
 * wanderer:
 *        Create a new wandering monster and aim it at the player
 */

void wanderer()
{
    MonsterThing *tp;
    static coord cp;

    do
    {
        cp = Area::random_position(Area::ForMonster);
    } while (has_line_of_sight(cp.x, cp.y, hero.x, hero.y));
    tp = new MonsterThing(randmonster(true), cp);
    if (on(player, SEEMONST))
    {
        if (!on(player, ISHALU))
            setMapDisplay(cp.x, cp.y, tp->type | DISPLAY_INVERT);
        else
            setMapDisplay(cp.x, cp.y, (rnd(26) + 'A') | DISPLAY_INVERT);
    }
    runto(tp);
}

/*
 * wake_monster:
 *        What to do when the hero steps next to a monster
 */
MonsterThing* wake_monster(int y, int x)
{
    MonsterThing *tp;
    int ch;
    const char *mname;

    tp = monster_at(x, y);
    assert(tp != nullptr);
    ch = tp->type;
    /*
     * Every time he sees mean monster, it might start chasing him
     */
    if (!on(*tp, ISRUN) && rnd(5) != 0 && on(*tp, ISMEAN) && !on(*tp, ISHELD)
        && !ISWEARING(R_STEALTH) && !on(player, ISLEVIT))
    {
        tp->dest = &hero;
        tp->flags |= ISRUN;
    }
    
    /* handle medusa's, which can confuse you on sight. */
    if (ch == 'M' && !on(player, ISBLIND) && !on(player, ISHALU) && !on(*tp, ISFOUND) && !on(*tp, ISCANC) && on(*tp, ISRUN))
    {
        if ((flags_at(x, y) & F_ISLIT) || dist(y, x, hero.y, hero.x) < LAMPDIST)
        {
            tp->flags |= ISFOUND;
            if (!save(VS_MAGIC))
            {
                if (on(player, ISHUH))
                    lengthen(unconfuse, spread(HUHDURATION));
                else
                    fuse(unconfuse, 0, spread(HUHDURATION), AFTER);
                player.flags |= ISHUH;
                mname = set_mname(tp);
                addmsg("%s", mname);
                if (strcmp(mname, "it") != 0)
                    addmsg("'");
                msg("s gaze has confused you");
            }
        }
    }
    /*
     * Let greedy ones guard gold
     */
    if (on(*tp, ISGREED) && !on(*tp, ISRUN))
    {
        tp->flags |= ISRUN;
        tp->dest = nullptr;
        visit_field_of_view(tp->pos.x, tp->pos.y, 10, [tp](int x, int y)
        {
            if (item_at(x, y) && item_at(x, y)->type == GOLD)
                tp->dest = &item_at(x, y)->pos;
        });
        if (tp->dest == nullptr)
            tp->dest = &hero;
    }
    return tp;
}

/*
 * give_pack:
 *        Give a pack to a monster if it deserves one
 */

void give_pack(MonsterThing *tp)
{
    if (level >= max_level && rnd(100) < monsters[tp->type-'A'].m_carry)
        tp->pack.push_front(new_thing());
    
    //Give Icemonsters icecream buckets by a random chance
    if (tp->type == 'I' && rnd(100) < 20)
    {
        ItemThing* obj = new ItemThing();
        obj->type = FOOD;
        obj->which = F_ICECREAM_BUCKET;
        obj->count = 1;
        tp->pack.push_back(obj);
    }
    //Give Unicorns rainbow poo by chance.
    if (tp->type == 'U' && rnd(100) < 20)
    {
        ItemThing* obj = new ItemThing();
        obj->type = FOOD;
        obj->which = F_RAINBOW_POO;
        obj->count = 1;
        tp->pack.push_back(obj);
    }
}

/*
 * save_throw:
 *        See if a creature save against something
 */
int
save_throw(int which, MonsterThing *tp)
{
    int need;

    need = 14 + which - tp->stats.s_lvl / 2;
    return (roll(1, 20) >= need);
}

/*
 * save:
 *        See if he saves against various nasty things
 */
int save(int which)
{
    if (which == VS_MAGIC)
    {
        if (ISRING(LEFT, R_PROTECT))
            which -= cur_ring[LEFT]->arm;
        if (ISRING(RIGHT, R_PROTECT))
            which -= cur_ring[RIGHT]->arm;
    }
    return save_throw(which, &player);
}
