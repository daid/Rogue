#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rogue.h"

/*
 * eat:
 *        She wants to eat something, so let her try
 */

void eat()
{
    ItemThing *obj;

    if ((obj = get_item("eat", FOOD)) == nullptr)
        return;
    if (obj->type == FOOD)
    {
        if (obj->which == F_SALTY_CRACKER)
            add_food(HUNGERTIME / 2 - 200 + rnd(400));
        else
            add_food(HUNGERTIME - 200 + rnd(400));
        
        if (obj->which == F_RAINBOW_POO)
        {
            msg("This does not taste like poo at all! Yummy!");
            //Restore health when eating rainbow poo.
            if ((player.stats.s_hpt += roll(player.stats.s_lvl, 4)) > player.stats.s_maxhp)
            {
                player.stats.s_hpt = player.stats.s_maxhp;
            }
        }
        else if (obj->which == F_ICECREAM_BUCKET && rnd(100) < 40)
        {
            msg("Ugh, brainfreeze!");
            waste_time();
        }else{
            if (rnd(100) > 30)
            {
                msg("%s, this %s tasted awful", choose_str("bummer", "yuk"), food_info[obj->which].oi_name);
            }
            else
            {
                msg("%s, that tasted good", choose_str("oh, wow", "yum"));
                player.stats.s_exp++;
                check_level();
            }
        }
    }else if (obj->type == SCROLL)
    {
        //Sure, eat a scroll, it's good for you... (well, not really. But it does help with the hunger)
        msg("blegh, that was hard to digest");
        if (hungry_state)
            msg("but it helps stilling the hunger");
        add_food(400 + rnd(100));
        take_damage(roll(1, 3), 'e');
    }else if (obj->type == POTION)
    {
        msg("that's Inedible! Quaff potions instead.");
    }else{
        if (!terse)
            msg("ugh, you would get ill if you ate that");
        else
            msg("that's Inedible!");
        return;
    }

    if (obj == cur_weapon)
        cur_weapon = nullptr;
    leave_pack(obj, false, false);
}

void add_food(int amount)
{
    if (food_left < 0)
        food_left = 0;
    if ((food_left += amount) > STOMACHSIZE)
        food_left = STOMACHSIZE;
    if (food_left < MORETIME)
    {
        hungry_state = 2;
    }
    else if (food_left < 2 * MORETIME)
    {
        hungry_state = 1;
    }
    else
    {
        hungry_state = 0;
    }
}

void check_spoiled_food()
{
    for(ItemThing* obj : player.pack)
    {
        if (obj->type == FOOD)
        {
            if (obj->which == F_ICECREAM_BUCKET)
            {
                if (obj->count > 1)
                    msg("One of your icecreambuckets has melted. %s", choose_str("Chill!", "Shame."));
                else
                    msg("You notice your icecream has melted. %s", choose_str("Chill!", "Shame."));
                leave_pack(obj, false, false);
                break;
            }
        }
    }
}
