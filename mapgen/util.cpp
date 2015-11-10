#include <string.h>
#include "../rogue.h"
#include "../areas.h"
#include "util.h"

/*
 * placePassage:
 *        add a passage character or secret passage here
 */
void placePassage(coord co)
{
    PLACE* pp = INDEX(co.y, co.x);
    pp->p_flags |= F_PASS;
    if (rnd(10) + 1 < level && rnd(40) == 0)
        pp->p_flags &= ~F_REAL;
    else
        pp->p_ch = PASSAGE;
}

void placeRandomStairs()
{
    stairs = Area::random_position(Area::ForItem);
    char_at(stairs.x, stairs.y) = STAIRS;
}

void placeRandomHero()
{
    hero = Area::random_position(Area::ForMonster);
}

void placeRandomTraps()
{
    if (rnd(10) < level)
    {
        int ntraps = rnd(level / 4) + 1;
        if (ntraps > MAXTRAPS)
            ntraps = MAXTRAPS;
        int i = ntraps;
        while (i--)
        {
            /*
             * not only wouldn't it be NICE to have traps in mazes
             * (not that we care about being nice), since the trap
             * number is stored where the passage number is, we
             * can't actually do it.
             */
            coord co;
            do
            {
                co = Area::random_position(Area::ForItem);
            } while (char_at(co.x, co.y) != FLOOR);
            flags_at(co.x, co.y) &=~F_REAL;
            flags_at(co.x, co.y) |= rnd(NTRAPS);
        }
    }
}

void placeRandomItems()
{
    /*
     * Once you have found the amulet, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (amulet && level < max_level)
        return;
    /*
     * Do MAXOBJ attempts to put things on a level
     */
    for (int i = 0; i < MAXOBJ; i++)
    {
        if (rnd(100) < 36 + (std::max(0, 5 - level) * 5))
        {
            /*
             * Pick a new object and link it in the list
             */
            ItemThing* obj = new_thing();
            lvl_obj.push_front(obj);
            /*
             * Put it somewhere
             */
            obj->pos = Area::random_position(Area::ForItem);
            item_at(obj->pos.x, obj->pos.y) = obj;
        }
    }
    
}

void placeAmuletIfRequired()
{
    /*
     * If he is really deep in the dungeon and he hasn't found the
     * amulet yet, put it somewhere on the ground
     */
    if (level >= AMULETLEVEL && !amulet)
    {
        ItemThing* obj = new ItemThing();
        lvl_obj.push_front(obj);
        obj->hplus = 0;
        obj->dplus = 0;
        strncpy(obj->damage,"0x0",sizeof(obj->damage));
        strncpy(obj->hurldmg,"0x0",sizeof(obj->hurldmg));
        obj->arm = 11;
        obj->type = AMULET;
        /*
         * Put it somewhere
         */
        obj->pos = Area::random_position(Area::ForItem);
        item_at(obj->pos.x, obj->pos.y) = obj;
    }
}
