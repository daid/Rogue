#include "../rogue.h"
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
