/*
 * global variable initializaton
 *
 * @(#)init.c        4.31 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "rogue.h"

/*
 * init_player:
 *        Roll her up
 */
void
init_player()
{
    ItemThing *obj;

    player.stats = max_stats;
    food_left = HUNGERTIME;
    /*
     * Give him some food
     */
    obj = new ItemThing();
    obj->type = FOOD;
    obj->count = 1;
    add_pack(obj, TRUE);
    /*
     * And his suit of armor
     */
    obj = new ItemThing();
    obj->type = ARMOR;
    obj->which = RING_MAIL;
    obj->arm = a_class[RING_MAIL] - 1;
    obj->flags |= ISKNOW;
    obj->count = 1;
    cur_armor = obj;
    add_pack(obj, TRUE);
    /*
     * Give him his weaponry.  First a mace.
     */
    obj = new ItemThing();
    init_weapon(obj, MACE);
    obj->hplus = 1;
    obj->dplus = 1;
    obj->flags |= ISKNOW;
    add_pack(obj, TRUE);
    cur_weapon = obj;
    /*
     * Now a +1 bow
     */
    obj = new ItemThing();
    init_weapon(obj, BOW);
    obj->hplus = 1;
    obj->flags |= ISKNOW;
    add_pack(obj, TRUE);
    /*
     * Now some arrows
     */
    obj = new ItemThing();
    init_weapon(obj, ARROW);
    obj->count = rnd(15) + 25;
    obj->flags |= ISKNOW;
    add_pack(obj, TRUE);

#ifdef DEBUG
    obj = new ItemThing();
    obj->type = SCROLL;
    obj->which = S_HINT;
    obj->count = 5;
    add_pack(obj, TRUE);

    obj = new ItemThing();
    obj->type = STICK;
    obj->which = WS_LIGHT;
    obj->flags |= ISKNOW;
    obj->arm = 1;
    obj->count = 1;
    add_pack(obj, TRUE);
#endif
}

/*
 * Contains defintions and functions for dealing with things like
 * potions and scrolls
 */

const char *rainbow[] = {
    "amber",
    "aquamarine",
    "black",
    "blue",
    "brown",
    "clear",
    "crimson",
    "cyan",
    "ecru",
    "gold",
    "green",
    "grey",
    "magenta",
    "orange",
    "pink",
    "plaid",
    "purple",
    "red",
    "silver",
    "tan",
    "tangerine",
    "topaz",
    "turquoise",
    "vermilion",
    "violet",
    "white",
    "yellow",
};

#define NCOLORS (sizeof rainbow / sizeof (char *))
int cNCOLORS = NCOLORS;

static const char *sylls[] = {
    "a", "ab", "ag", "aks", "ala", "an", "app", "arg", "arze", "ash",
    "bek", "bie", "bit", "bjor", "blu", "bot", "bu", "byt", "comp",
    "con", "cos", "cre", "dalf", "dan", "den", "do", "e", "eep", "el",
    "eng", "er", "ere", "erk", "esh", "evs", "fa", "fid", "fri", "fu",
    "gan", "gar", "glen", "gop", "gre", "ha", "hyd", "i", "ing", "ip",
    "ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
    "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur", "nej",
    "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od", "ood",
    "org", "orn", "ox", "oxy", "pay", "ple", "plu", "po", "pot",
    "prok", "re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol", "sa",
    "san", "sat", "sef", "seh", "shu", "ski", "sna", "sne", "snik",
    "sno", "so", "sol", "sri", "sta", "sun", "ta", "tab", "tem",
    "ther", "ti", "tox", "trol", "tue", "turs", "u", "ulk", "um", "un",
    "uni", "ur", "val", "viv", "vly", "vom", "wah", "wed", "werg",
    "wex", "whon", "wun", "xo", "y", "yot", "yu", "zant", "zeb", "zim",
    "zok", "zon", "zum",
};

STONE stones[] = {
    { "agate",                 25},
    { "alexandrite",         40},
    { "amethyst",         50},
    { "carnelian",         40},
    { "diamond",        300},
    { "emerald",        300},
    { "germanium",        225},
    { "granite",          5},
    { "garnet",                 50},
    { "jade",                150},
    { "kryptonite",        300},
    { "lapis lazuli",         50},
    { "moonstone",         50},
    { "obsidian",         15},
    { "onyx",                 60},
    { "opal",                200},
    { "pearl",                220},
    { "peridot",         63},
    { "ruby",                350},
    { "sapphire",        285},
    { "stibotantalite",        200},
    { "tiger eye",         50},
    { "topaz",                 60},
    { "turquoise",         70},
    { "taaffeite",        300},
    { "zircon",                  80},
};

#define NSTONES (sizeof stones / sizeof (STONE))
int cNSTONES = NSTONES;

const char *wood[] = {
    "avocado wood",
    "balsa",
    "bamboo",
    "banyan",
    "birch",
    "cedar",
    "cherry",
    "cinnibar",
    "cypress",
    "dogwood",
    "driftwood",
    "ebony",
    "elm",
    "eucalyptus",
    "fall",
    "hemlock",
    "holly",
    "ironwood",
    "kukui wood",
    "mahogany",
    "manzanita",
    "maple",
    "oaken",
    "persimmon wood",
    "pecan",
    "pine",
    "poplar",
    "redwood",
    "rosewood",
    "spruce",
    "teak",
    "walnut",
    "zebrawood",
};

#define NWOOD (sizeof wood / sizeof (char *))
int cNWOOD = NWOOD;

const char *metal[] = {
    "aluminum",
    "beryllium",
    "bone",
    "brass",
    "bronze",
    "copper",
    "electrum",
    "gold",
    "iron",
    "lead",
    "magnesium",
    "mercury",
    "nickel",
    "pewter",
    "platinum",
    "steel",
    "silver",
    "silicon",
    "tin",
    "titanium",
    "tungsten",
    "zinc",
};

#define NMETAL (sizeof metal / sizeof (char *))
int cNMETAL = NMETAL;
#define MAX3(a,b,c)        (a > b ? (a > c ? a : c) : (b > c ? b : c))

static bool used[MAX3(NCOLORS, NSTONES, NWOOD)];

/*
 * init_colors:
 *        Initialize the potion color scheme for this time
 */
void
init_colors()
{
    unsigned int i, j;

    for (i = 0; i < NCOLORS; i++)
        used[i] = FALSE;
    for (i = 0; i < MAXPOTIONS; i++)
    {
        do
            j = rnd(NCOLORS);
        until (!used[j]);
        used[j] = TRUE;
        p_colors[i] = rainbow[j];
    }
}

/*
 * init_names:
 *        Generate the names of the various scrolls
 */
#define MAXNAME        40        /* Max number of characters in a name */

void init_names()
{
    int nsyl;
    char *cp;
    const char *sp;
    int i, nwords;

    for (i = 0; i < MAXSCROLLS; i++)
    {
        cp = prbuf;
        nwords = rnd(3) + 2;
        while (nwords--)
        {
            nsyl = rnd(3) + 1;
            while (nsyl--)
            {
                sp = sylls[rnd((sizeof sylls) / (sizeof (char *)))];
                if (&cp[strlen(sp)] > &prbuf[MAXNAME])
                        break;
                while (*sp)
                    *cp++ = *sp++;
            }
            *cp++ = ' ';
        }
        *--cp = '\0';
        s_names[i] = (char *) malloc((unsigned) strlen(prbuf)+1);
        strcpy(s_names[i], prbuf);
    }
}

/*
 * init_stones:
 *        Initialize the ring stone setting scheme for this time
 */
void
init_stones()
{
    unsigned int i, j;

    for (i = 0; i < NSTONES; i++)
        used[i] = FALSE;
    for (i = 0; i < MAXRINGS; i++)
    {
        do
            j = rnd(NSTONES);
        until (!used[j]);
        used[j] = TRUE;
        r_stones[i] = stones[j].st_name;
        ring_info[i].oi_worth += stones[j].st_value;
    }
}

/*
 * init_materials:
 *        Initialize the construction materials for wands and staffs
 */
void
init_materials()
{
    unsigned int i, j;
    const char *str;
    static bool metused[NMETAL];

    for (i = 0; i < NWOOD; i++)
        used[i] = FALSE;
    for (i = 0; i < NMETAL; i++)
        metused[i] = FALSE;
    for (i = 0; i < MAXSTICKS; i++)
    {
        for (;;)
            if (rnd(2) == 0)
            {
                j = rnd(NMETAL);
                if (!metused[j])
                {
                    ws_type[i] = "wand";
                    str = metal[j];
                    metused[j] = TRUE;
                    break;
                }
            }
            else
            {
                j = rnd(NWOOD);
                if (!used[j])
                {
                    ws_type[i] = "staff";
                    str = wood[j];
                    used[j] = TRUE;
                    break;
                }
            }
        ws_made[i] = str;
    }
}

/*
 * sumprobs:
 *        Sum up the probabilities for items appearing
 */
void sumprobs(struct obj_info *info, int bound, const char *name)
{
#ifdef DEBUG
    struct obj_info *start = info;
#endif
    struct obj_info *endp;

    endp = info + bound;
    while (++info < endp)
        info->oi_prob += (info - 1)->oi_prob;
#ifdef DEBUG
    badcheck(name, start, bound);
#endif
}

/*
 * init_probs:
 *        Initialize the probabilities for the various items
 */
void init_probs()
{
    sumprobs(things, NUMITEMTYPES, "things");
    sumprobs(pot_info, MAXPOTIONS, "potions");
    sumprobs(scr_info, MAXSCROLLS, "scrolls");
    sumprobs(ring_info, MAXRINGS, "rings");
    sumprobs(ws_info, MAXSTICKS, "sticks");
    sumprobs(weap_info, MAXWEAPONS, "weapons");
    sumprobs(arm_info, MAXARMORS, "armor");
}

/*
 * badcheck:
 *        Check to see if a series of probabilities sums to 100
 */
void badcheck(const char *name, struct obj_info *info, int bound)
{
    register struct obj_info *end;

    if (info[bound - 1].oi_prob == 100)
        return;
    printf("\nBad percentages for %s (bound = %d):\n", name, bound);
    for (end = &info[bound]; info < end; info++)
        printf("%3d%% %s\n", info->oi_prob, info->oi_name);
}

/*
 * pick_color:
 *        If he is halucinating, pick a random color name and return it,
 *        otherwise return the given color.
 */
const char* pick_color(const char* col)
{
    return (on(player, ISHALU) ? rainbow[rnd(NCOLORS)] : col);
}
