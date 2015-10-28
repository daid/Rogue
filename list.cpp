/*
 * Functions for dealing with linked lists of goodies
 *
 * @(#)list.c        4.12 (Berkeley) 02/05/99
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include "rogue.h"


/*
 * new_item
 *        Get a new item with a specified size
 */
ITEM_THING* new_item()
{
    ITEM_THING *item;

    item = (ITEM_THING*)calloc(1, sizeof *item);
    item->next = NULL;
    item->prev = NULL;
    return item;
}

MONSTER_THING *new_monster_thing()
{
    MONSTER_THING *item;

    item = (MONSTER_THING*)calloc(1, sizeof *item);
    item->next = NULL;
    item->prev = NULL;
    return item;
}
