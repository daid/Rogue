/*
 * All the fighting gets done here
 *
 * @(#)fight.c        4.67 (Berkeley) 09/06/83
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

#define        EQSTR(a, b)        (strcmp(a, b) == 0)

const char *h_names[] = {                /* strings for hitting */
        " scored an excellent hit on ",
        " hit ",
        " have injured ",
        " swing and hit ",
        " scored an excellent hit on ",
        " hit ",
        " has injured ",
        " swings and hits "
};

const char *m_names[] = {                /* strings for missing */
        " miss",
        " swing and miss",
        " barely miss",
        " don't hit",
        " misses",
        " swings and misses",
        " barely misses",
        " doesn't hit",
};

/*
 * adjustments to hit probabilities due to strength
 */
static int str_plus[] = {
    -7, -6, -5, -4, -3, -2, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
    1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3,
};

/*
 * adjustments to damage done due to strength
 */
static int add_dam[] = {
    -7, -6, -5, -4, -3, -2, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3,
    3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6
};

/*
 * fight:
 *        The player attacks the monster.
 */
int fight(coord *mp, ItemThing *weap, bool thrown)
{
    MonsterThing *tp;
    bool did_hit = TRUE;
    const char *mname;
    int ch;

    /*
     * Find the monster we want to fight
     */
    tp = monster_at(mp->x, mp->y);
    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    count = 0;
    quiet = 0;
    runto(*mp);
    /*
     * Let him know it was really a xeroc (if it was one).
     */
    ch = '\0';
    if (tp->type == 'X' && tp->disguise != 'X' && !on(player, ISBLIND))
    {
        tp->disguise = 'X';
        if (on(player, ISHALU)) {
            ch = (char)(rnd(26) + 'A');
            setMapDisplay(tp->pos.x, tp->pos.y, ch);
        }
        msg(choose_str("heavy!  That's a nasty critter!",
                       "wait!  That's a xeroc!"));
        if (!thrown)
            return FALSE;
    }
    mname = set_mname(tp);
    did_hit = FALSE;
    has_hit = (terse && !to_death);
    if (roll_em(&player, tp, weap, thrown))
    {
        did_hit = FALSE;
        if (thrown)
            thunk(weap, mname, terse);
        else
            hit(NULL, mname, terse);
        if (on(player, CANHUH))
        {
            did_hit = TRUE;
            tp->flags |= ISHUH;
            player.flags &= ~CANHUH;
            endmsg();
            has_hit = FALSE;
            msg("your hands stop glowing %s", pick_color("red"));
        }
        if (tp->stats.s_hpt <= 0)
            killed(tp, TRUE);
        else if (did_hit && !on(player, ISBLIND))
            msg("%s appears confused", mname);
        did_hit = TRUE;
    }
    else
        if (thrown)
            bounce(weap, mname, terse);
        else
            miss(NULL, mname, terse);
    return did_hit;
}

/*
 * attack:
 *        The monster attacks the player
 */
int attack(MonsterThing *mp)
{
    const char *mname;
    int oldhp;

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    running = FALSE;
    count = 0;
    quiet = 0;
    if (to_death && !on(*mp, ISTARGET))
    {
        to_death = FALSE;
        kamikaze = FALSE;
    }
    if (mp->type == 'X' && mp->disguise != 'X' && !on(player, ISBLIND))
    {
        mp->disguise = 'X';
        if (on(player, ISHALU))
            setMapDisplay(mp->pos.x, mp->pos.y, rnd(26) + 'A');
    }
    mname = set_mname(mp);
    oldhp = player.stats.s_hpt;
    if (roll_em(mp, &player, NULL, FALSE))
    {
        if (mp->type != 'I')
        {
            if (has_hit)
                addmsg(".  ");
            hit(mname, NULL, FALSE);
        }
        else
            if (has_hit)
                endmsg();
        has_hit = FALSE;
        if (player.stats.s_hpt <= 0)
            death(mp->type);        /* Bye bye life ... */
        else if (!kamikaze)
        {
            oldhp -= player.stats.s_hpt;
            if (oldhp > max_hit)
                max_hit = oldhp;
            if (player.stats.s_hpt <= max_hit)
                to_death = FALSE;
        }
        if (!on(*mp, ISCANC))
            switch (mp->type)
            {
                case 'A':
                    /*
                     * If an aquator hits, you can lose armor class.
                     */
                    rust_armor(cur_armor);
                when 'I':
                    /*
                     * The ice monster freezes you
                     */
                    player.flags &= ~ISRUN;
                    if (!no_command)
                    {
                        addmsg("you are frozen");
                        if (!terse)
                            addmsg(" by the %s", mname);
                        endmsg();
                    }
                    no_command += rnd(2) + 2;
                    if (no_command > BORE_LEVEL)
                        death('h');
                when 'R':
                    /*
                     * Rattlesnakes have poisonous bites
                     */
                    if (!save(VS_POISON))
                    {
                        if (!ISWEARING(R_SUSTSTR))
                        {
                            chg_str(-1);
                            if (!terse)
                                msg("you feel a bite in your leg and now feel weaker");
                            else
                                msg("a bite has weakened you");
                        }
                        else if (!to_death)
                        {
                            if (!terse)
                                msg("a bite momentarily weakens you");
                            else
                                msg("bite has no effect");
                        }
                    }
                when 'W':
                case 'V':
                    /*
                     * Wraiths might drain energy levels, and Vampires
                     * can steal max_hp
                     */
                    if (rnd(100) < (mp->type == 'W' ? 15 : 30))
                    {
                        register int fewer;

                        if (mp->type == 'W')
                        {
                            if (player.stats.s_exp == 0)
                                death('W');                /* All levels gone */
                            if (--player.stats.s_lvl == 0)
                            {
                                player.stats.s_exp = 0;
                                player.stats.s_lvl = 1;
                            }
                            else
                                player.stats.s_exp = e_levels[player.stats.s_lvl-1]+1;
                            fewer = roll(1, 10);
                        }
                        else
                            fewer = roll(1, 3);
                        player.stats.s_hpt -= fewer;
                        player.stats.s_maxhp -= fewer;
                        if (player.stats.s_hpt <= 0)
                            player.stats.s_hpt = 1;
                        if (player.stats.s_maxhp <= 0)
                            death(mp->type);
                        msg("you suddenly feel weaker");
                    }
                when 'F':
                    /*
                     * Venus Flytrap stops the poor guy from moving
                     */
                    player.flags |= ISHELD;
                    sprintf(monsters['F'-'A'].m_stats.s_dmg,"%dx1", ++vf_hit);
                    if (--player.stats.s_hpt <= 0)
                        death('F');
                when 'L':
                {
                    /*
                     * Leperachaun steals some gold
                     */
                    register int lastpurse;

                    lastpurse = purse;
                    purse -= GOLDCALC;
                    if (!save(VS_MAGIC))
                        purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
                    if (purse < 0)
                        purse = 0;
                    remove_mon(&mp->pos, mp, FALSE);
                    mp=NULL;
                    if (purse != lastpurse)
                        msg("your purse feels lighter");
                }
                when 'N':
                {
                    ItemThing *steal;
                    int nobj;

                    /*
                     * Nymph's steal a magic item, look through the pack
                     * and pick out one we like.
                     */
                    steal = NULL;
                    nobj = 0;
                    for (ItemThing* obj : player.pack)
                        if (obj != cur_armor && obj != cur_weapon && obj != cur_ring[LEFT] && obj != cur_ring[RIGHT] && is_magic(obj) && rnd(++nobj) == 0)
                            steal = obj;
                    if (steal != NULL)
                    {
                        remove_mon(&mp->pos, monster_at(mp->pos.x, mp->pos.y), FALSE);
                        mp=NULL;
                        leave_pack(steal, FALSE, FALSE);
                        msg("she stole %s!", inv_name(steal, TRUE));
                        delete steal;
                    }
                }
                otherwise:
                    break;
            }
    }
    else if (mp->type != 'I')
    {
        if (has_hit)
        {
            addmsg(".  ");
            has_hit = FALSE;
        }
        if (mp->type == 'F')
        {
            player.stats.s_hpt -= vf_hit;
            if (player.stats.s_hpt <= 0)
                death(mp->type);        /* Bye bye life ... */
        }
        miss(mname, NULL, FALSE);
    }
    if (fight_flush && !to_death)
        flush_type();
    count = 0;
    status();
    if (mp == NULL)
        return(-1);
    else
        return(0);
}

/*
 * set_mname:
 *        return the monster name for the given monster
 */
const char* set_mname(MonsterThing *tp)
{
    int ch;
    const char *mname;
    static char tbuf[MAXSTR] = { 't', 'h', 'e', ' ' };

    if (!see_monst(tp) && !on(player, SEEMONST))
        return (terse ? "it" : "something");
    else if (on(player, ISHALU))
    {
        ch = getMapDisplay(tp->pos.x, tp->pos.y);
        if (!isupper(ch))
            ch = rnd(26);
        else
            ch -= 'A';
        mname = monsters[ch].m_name;
    }
    else
        mname = monsters[tp->type - 'A'].m_name;
    strcpy(&tbuf[4], mname);
    return tbuf;
}

/*
 * swing:
 *        Returns true if the swing hits
 */
int
swing(int at_lvl, int op_arm, int wplus)
{
    //Instead of 1d20, we throw 3d20 and select the middle one.
    //This favors middle rolls more, making the game feel less random. While still making the average roll 10.5
    int res0 = rnd(20);
    int res1 = rnd(20);
    int res2 = rnd(20);
    int need = (20 - at_lvl) - op_arm;
    if (res0 > res1) { int tmp = res0; res0 = res1; res1 = tmp; }
    if (res1 > res2) { int tmp = res1; res1 = res2; res2 = tmp; }
    if (res0 > res1) { int tmp = res0; res0 = res1; res1 = tmp; }

    return (res1 + wplus >= need);
}

/*
 * roll_em:
 *        Roll several attacks
 */
bool roll_em(MonsterThing *thatt, MonsterThing *thdef, ItemThing *weap, bool hurl)
{
    register struct stats *att, *def;
    register char *cp;
    register int ndice, nsides, def_arm;
    register bool did_hit = FALSE;
    register int hplus;
    register int dplus;
    register int damage;

    att = &thatt->stats;
    def = &thdef->stats;
    if (weap == NULL)
    {
        cp = att->s_dmg;
        dplus = 0;
        hplus = 0;
    }
    else
    {
        hplus = (weap == NULL ? 0 : weap->hplus);
        dplus = (weap == NULL ? 0 : weap->dplus);
        if (weap == cur_weapon)
        {
            if (ISRING(LEFT, R_ADDDAM))
                dplus += cur_ring[LEFT]->arm;
            else if (ISRING(LEFT, R_ADDHIT))
                hplus += cur_ring[LEFT]->arm;
            if (ISRING(RIGHT, R_ADDDAM))
                dplus += cur_ring[RIGHT]->arm;
            else if (ISRING(RIGHT, R_ADDHIT))
                hplus += cur_ring[RIGHT]->arm;
        }
        cp = weap->damage;
        if (hurl)
        {
            if ((weap->flags&ISMISL) && cur_weapon != NULL && cur_weapon->which == weap->launch)
            {
                cp = weap->hurldmg;
                hplus += cur_weapon->hplus;
                dplus += cur_weapon->dplus;
            }
            else if (weap->launch < 0)
                cp = weap->hurldmg;
        }
    }
    /*
     * If the creature being attacked is not running (alseep or held)
     * then the attacker gets a plus four bonus to hit.
     */
    if (!on(*thdef, ISRUN))
        hplus += 4;
    def_arm = def->s_arm;
    if (def == &player.stats)
    {
        if (cur_armor != NULL)
            def_arm = cur_armor->arm;
        if (ISRING(LEFT, R_PROTECT))
            def_arm -= cur_ring[LEFT]->arm;
        if (ISRING(RIGHT, R_PROTECT))
            def_arm -= cur_ring[RIGHT]->arm;
    }
    while(cp != NULL && *cp != '\0')
    {
        ndice = atoi(cp);
        if ((cp = strchr(cp, 'x')) == NULL)
            break;
        nsides = atoi(++cp);
        if (swing(att->s_lvl, def_arm, hplus + str_plus[att->s_str]))
        {
            int proll;

            proll = roll(ndice, nsides);
            damage = dplus + proll + add_dam[att->s_str];
            def->s_hpt -= max(0, damage);
            did_hit = TRUE;
        }
        if ((cp = strchr(cp, '/')) == NULL)
            break;
        cp++;
    }
    return did_hit;
}

/*
 * prname:
 *        The print name of a combatant
 */
const char * prname(const char *mname, bool upper)
{
    static char tbuf[MAXSTR];

    *tbuf = '\0';
    if (mname == 0)
        strcpy(tbuf, "you"); 
    else
        strcpy(tbuf, mname);
    if (upper)
        *tbuf = (char) toupper(*tbuf);
    return tbuf;
}

/*
 * thunk:
 *        A missile hits a monster
 */
void thunk(ItemThing *weap, const char *mname, bool noend)
{
    if (to_death)
        return;
    if (weap->type == WEAPON)
        addmsg("the %s hits ", weap_info[weap->which].oi_name);
    else
        addmsg("you hit ");
    addmsg("%s", mname);
    if (!noend)
        endmsg();
}

/*
 * hit:
 *        Print a message to indicate a succesful hit
 */

void
hit(const char *er, const char *ee, bool noend)
{
    int i;
    const char *s;

    if (to_death)
        return;
    addmsg(prname(er, TRUE));
    if (terse)
        s = " hit";
    else
    {
        i = rnd(4);
        if (er != NULL)
            i += 4;
        s = h_names[i];
    }
    addmsg(s);
    if (!terse)
        addmsg(prname(ee, FALSE));
    if (!noend)
        endmsg();
}

/*
 * miss:
 *        Print a message to indicate a poor swing
 */
void miss(const char *er, const char *ee, bool noend)
{
    int i;

    if (to_death)
        return;
    addmsg(prname(er, TRUE));
    if (terse)
        i = 0;
    else
        i = rnd(4);
    if (er != NULL)
        i += 4;
    addmsg(m_names[i]);
    if (!terse)
        addmsg(" %s", prname(ee, FALSE));
    if (!noend)
        endmsg();
}

/*
 * bounce:
 *        A missile misses a monster
 */
void bounce(ItemThing *weap, const char *mname, bool noend)
{
    if (to_death)
        return;
    if (weap->type == WEAPON)
        addmsg("the %s misses ", weap_info[weap->which].oi_name);
    else
        addmsg("you missed ");
    addmsg(mname);
    if (!noend)
        endmsg();
}

/*
 * remove_mon:
 *        Remove a monster from the screen
 */
void remove_mon(coord *mp, MonsterThing *tp, bool waskill)
{
    for(auto it = tp->pack.begin(); it != tp->pack.end(); )
    {
        ItemThing* obj = *it++;
        obj->pos = tp->pos;
        tp->pack.remove(obj);
        if (waskill)
            fall(obj, FALSE);
        else
            delete obj;
    }
    monster_at(mp->x, mp->y) = nullptr;
    mlist.remove(tp);
    if (see_monst(tp))
        setMapDisplay(mp->x, mp->y, char_at_place(mp->x, mp->y));
    if (on(*tp, ISTARGET))
    {
        kamikaze = FALSE;
        to_death = FALSE;
        if (fight_flush)
            flush_type();
    }
    delete tp;
}

/*
 * killed:
 *        Called to put a monster to death
 */
void
killed(MonsterThing *tp, bool pr)
{
    const char *mname;

    player.stats.s_exp += tp->stats.s_exp;

    /*
     * If the monster was a venus flytrap, un-hold him
     */
    switch (tp->type)
    {
        case 'F':
            player.flags &= ~ISHELD;
            vf_hit = 0;
            strcpy(monsters['F'-'A'].m_stats.s_dmg, "000x0");
        when 'L':
        {
            ItemThing *gold;

            if (fallpos(&tp->pos, &tp->room->r_gold) && level >= max_level)
            {
                gold = new ItemThing();
                gold->type = GOLD;
                gold->arm = GOLDCALC;
                if (save(VS_MAGIC))
                    gold->arm += GOLDCALC + GOLDCALC
                                     + GOLDCALC + GOLDCALC;
                tp->pack.push_front(gold);
            }
        }
    }
    /*
     * Get rid of the monster.
     */
    mname = set_mname(tp);
    remove_mon(&tp->pos, tp, TRUE);
    if (pr)
    {
        if (has_hit)
        {
            addmsg(".  Defeated ");
            has_hit = FALSE;
        }
        else
        {
            if (!terse)
                addmsg("you have ");
            addmsg("defeated ");
        }
        msg(mname);
    }
    /*
     * Do adjustments if he went up a level
     */
    check_level();
    if (fight_flush)
        flush_type();
}
