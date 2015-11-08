#include <string.h>
#include "../../rogue.h"
#include "../../areas.h"
#include "../util.h"
#include "classic.h"
#include "maze.h"

#define TREAS_ROOM 20        /* one chance in TREAS_ROOM for a treasure room */
#define MAXTREAS 10        /* maximum number of treasures in a treasure room */
#define MINTREAS 2        /* minimum number of treasures in a treasure room */

ClassicMapGenerator::ClassicMapGenerator()
{
    memset(rooms, 0, sizeof(rooms));
    memset(passages, 0, sizeof(passages));
    for(int n=0; n<MAXPASS; n++)
        passages[n].r_flags = ISGONE|ISDARK;
}

bool ClassicMapGenerator::generate()
{
    do_rooms();     /* Draw rooms */
    //Build the area definitions from the room definitions.
    for(int n=0; n<MAXROOMS; n++)
    {
        if (rooms[n].r_flags & ISGONE)
            continue;
        Area area;
        area.position = rooms[n].r_pos;
        area.position.x += 1;
        area.position.y += 1;
        area.size = rooms[n].r_max;
        area.size.x -= 2;
        area.size.y -= 2;
        areas.push_back(area);
    }
    do_passages();  /* Draw passages */

    /*
     * check for treasure rooms, and if so, put it in.
     */
    if (!amulet && level == max_level && rnd(TREAS_ROOM) == 0)
        treas_room();

    placeRandomItems();
    placeAmuletIfRequired();
    placeRandomTraps();
    placeRandomStairs();
    placeRandomHero();
    
    return true;
}

/*
 * do_rooms:
 *        Create rooms and corridors with a connectivity graph
 */

void ClassicMapGenerator::do_rooms()
{
    const int GOLDGRP = 1;
    int i;
    struct room *rp;
    MonsterThing *tp;
    int left_out;
    static coord top;
    coord bsze;                                /* maximum room size */
    coord mp;

    bsze.x = NUMCOLS / 3;
    bsze.y = NUMLINES / 3;
    /*
     * Clear things for a new level
     */
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
    {
        rp->r_goldval = 0;
        rp->r_nexits = 0;
        rp->r_flags = 0;
    }
    /*
     * Put the gone rooms, if any, on the level
     */
    left_out = rnd(4);
    for (i = 0; i < left_out; i++)
        rooms[rnd_room()].r_flags |= ISGONE;
    /*
     * dig and populate all the rooms on the level
     */
    for (i = 0, rp = rooms; i < MAXROOMS; rp++, i++)
    {
        /*
         * Find upper left corner of box that this room goes in
         */
        top.x = (i % 3) * bsze.x + 1;
        top.y = (i / 3) * bsze.y;
        if (rp->r_flags & ISGONE)
        {
            /*
             * Place a gone room.  Make certain that there is a blank line
             * for passage drawing.
             */
            do
            {
                rp->r_pos.x = top.x + rnd(bsze.x - 2) + 1;
                rp->r_pos.y = top.y + rnd(bsze.y - 2) + 1;
                rp->r_max.x = -NUMCOLS;
                rp->r_max.y = -NUMLINES;
            } until (rp->r_pos.y > 0 && rp->r_pos.y < NUMLINES-1);
            continue;
        }
        /*
         * set room type
         */
        if (rnd(10) < level - 1)
        {
            rp->r_flags |= ISDARK;                /* dark room */
            if (rnd(15) == 0)
                rp->r_flags = ISMAZE;                /* maze room */
        }
        /*
         * Find a place and size for a random room
         */
        if (rp->r_flags & ISMAZE)
        {
            rp->r_max.x = bsze.x - 1;
            rp->r_max.y = bsze.y - 1;
            if ((rp->r_pos.x = top.x) == 1)
                rp->r_pos.x = 0;
            if ((rp->r_pos.y = top.y) == 0)
            {
                rp->r_pos.y++;
                rp->r_max.y--;
            }
        }
        else
        {
            do
            {
                rp->r_max.x = rnd(bsze.x - 4) + 4;
                rp->r_max.y = rnd(bsze.y - 4) + 4;
                rp->r_pos.x = top.x + rnd(bsze.x - rp->r_max.x);
                rp->r_pos.y = top.y + rnd(bsze.y - rp->r_max.y);
            } until (rp->r_pos.y != 0);
        }
        draw_room(rp);
        /*
         * Put the gold in
         */
        if (rnd(2) == 0 && (!amulet || level >= max_level))
        {
            ItemThing *gold;

            gold = new ItemThing();
            gold->arm = rp->r_goldval = GOLDCALC;//gold value is stored in arm field (used to be handled with ugly define)
            find_floor(rp, &gold->pos, FALSE, FALSE);
            gold->flags = ISMANY;
            gold->group = GOLDGRP;
            gold->type = GOLD;
            lvl_obj.push_front(gold);
            item_at(gold->pos.x, gold->pos.y) = gold;
        }
        /*
         * Put the monster in
         */
        if (rnd(100) < (rp->r_goldval > 0 ? 80 : 25))
        {
            find_floor(rp, &mp, FALSE, TRUE);
            tp = new MonsterThing(randmonster(FALSE), mp);
            give_pack(tp);
        }
    }
}

/*
 * rnd_room:
 *        Pick a room that is really there
 */
int ClassicMapGenerator::rnd_room()
{
    int rm;

    do
    {
        rm = rnd(MAXROOMS);
    } while (rooms[rm].r_flags & ISGONE);
    return rm;
}

/*
 * find_floor:
 *        Find a valid floor spot in this room.  If rp is NULL, then
 *        pick a new room each time around the loop.
 */
bool ClassicMapGenerator::find_floor(struct room *rp, coord *cp, int limit, bool monst)
{
    PLACE *pp;
    int cnt;
    char compchar = 0;
    bool pickroom;

    pickroom = (bool)(rp == NULL);
if(!limit) limit = 50;
    if (!pickroom)
        compchar = ((rp->r_flags & ISMAZE) ? PASSAGE : FLOOR);
    cnt = limit;
    for (;;)
    {
        if (limit && cnt-- == 0)
            return FALSE;
        if (pickroom)
        {
            rp = &rooms[rnd_room()];
            compchar = ((rp->r_flags & ISMAZE) ? PASSAGE : FLOOR);
        }
        rnd_pos(rp, cp);
        pp = INDEX(cp->y, cp->x);
        if (monst)
        {
            if (pp->p_monst == NULL && step_ok(pp->p_ch))
                return TRUE;
        }
        else if (pp->p_ch == compchar)
            return TRUE;
    }
}

/*
 * draw_room:
 *        Draw a box around a room and lay down the floor for normal
 *        rooms; for maze rooms, draw maze.
 */

void ClassicMapGenerator::draw_room(struct room *rp)
{
    int y, x;

    if (rp->r_flags & ISMAZE)
    {
        ClassicMazeGenerator maze_generator;
        maze_generator.generate(rp->r_pos, rp->r_max);
    }
    else
    {
        vert(rp, rp->r_pos.x);                                /* Draw left side */
        vert(rp, rp->r_pos.x + rp->r_max.x - 1);        /* Draw right side */
        horiz(rp, rp->r_pos.y);                                /* Draw top */
        horiz(rp, rp->r_pos.y + rp->r_max.y - 1);        /* Draw bottom */
        char_at(rp->r_pos.x, rp->r_pos.y) = WALL_TL;
        char_at(rp->r_pos.x, rp->r_pos.y + rp->r_max.y - 1) = WALL_BL;
        char_at(rp->r_pos.x + rp->r_max.x - 1, rp->r_pos.y) = WALL_TR;
        char_at(rp->r_pos.x + rp->r_max.x - 1, rp->r_pos.y + rp->r_max.y - 1) = WALL_BR;

        /*
         * Put the floor down
         */
        for (y = rp->r_pos.y + 1; y < rp->r_pos.y + rp->r_max.y - 1; y++)
            for (x = rp->r_pos.x + 1; x < rp->r_pos.x + rp->r_max.x - 1; x++)
                char_at(x, y) = FLOOR;
        /* if this room is not dark, lite up the whole floor/wall/doors */
        if (!(rp->r_flags & ISDARK))
        {
            for (y = rp->r_pos.y; y < rp->r_pos.y + rp->r_max.y; y++)
                for (x = rp->r_pos.x; x < rp->r_pos.x + rp->r_max.x; x++)
                    flat(y, x) |= F_ISLIT;
        }
    }
}

/*
 * vert:
 *        Draw a vertical line
 */
void ClassicMapGenerator::vert(struct room *rp, int startx)
{
    int y;

    for (y = rp->r_pos.y + 1; y <= rp->r_max.y + rp->r_pos.y - 1; y++)
        char_at(startx, y) = WALL_V;
}

/*
 * horiz:
 *        Draw a horizontal line
 */
void ClassicMapGenerator::horiz(struct room *rp, int starty)
{
    int x;

    for (x = rp->r_pos.x; x <= rp->r_pos.x + rp->r_max.x - 1; x++)
        char_at(x, starty) = WALL_H;
}

/*
 * rnd_pos:
 *        Pick a random spot in a room
 */
void ClassicMapGenerator::rnd_pos(struct room *rp, coord *cp)
{
    cp->x = rp->r_pos.x + rnd(rp->r_max.x - 2) + 1;
    cp->y = rp->r_pos.y + rnd(rp->r_max.y - 2) + 1;
}

/*
 * treas_room:
 *        Add a treasure room
 */
void ClassicMapGenerator::treas_room()
{
    const int MAXTRIES = 10;        /* max number of tries to put down a monster */
    int nm;
    ItemThing *tp;
    MonsterThing *monster_p;
    struct room *rp;
    int spots, num_monst;
    static coord mp;

    rp = &rooms[rnd_room()];
    spots = (rp->r_max.y - 2) * (rp->r_max.x - 2) - MINTREAS;
    if (spots > (MAXTREAS - MINTREAS))
        spots = (MAXTREAS - MINTREAS);
    num_monst = nm = rnd(spots) + MINTREAS;
    while (nm--)
    {
        find_floor(rp, &mp, 2 * MAXTRIES, FALSE);
        tp = new_thing();
        tp->pos = mp;
        lvl_obj.push_front(tp);
        item_at(tp->pos.x, tp->pos.y) = tp;
    }

    /*
     * fill up room with monsters from the next level down
     */

    if ((nm = rnd(spots) + MINTREAS) < num_monst + 2)
        nm = num_monst + 2;
    spots = (rp->r_max.y - 2) * (rp->r_max.x - 2);
    if (nm > spots)
        nm = spots;
    level++;
    while (nm--)
    {
        spots = 0;
        if (find_floor(rp, &mp, MAXTRIES, TRUE))
        {
            monster_p = new MonsterThing(randmonster(FALSE), mp);
            monster_p->flags |= ISMEAN;        /* no sloughers in THIS room */
            give_pack(monster_p);
        }
    }
    level--;
}

/*
 * do_passages:
 *        Draw all the passages on a level.
 */

void ClassicMapGenerator::do_passages()
{
    struct rdes *r1, *r2 = NULL;
    int i, j;
    int roomcount;
    static struct rdes
    {
        bool        conn[MAXROOMS];                /* possible to connect to room i? */
        bool        isconn[MAXROOMS];        /* connection been made to room i? */
        bool        ingraph;                /* this room in graph already? */
    } rdes[MAXROOMS] = {
        { { 0, 1, 0, 1, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
        { { 1, 0, 1, 0, 1, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
        { { 0, 1, 0, 0, 0, 1, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
        { { 1, 0, 0, 0, 1, 0, 1, 0, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
        { { 0, 1, 0, 1, 0, 1, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
        { { 0, 0, 1, 0, 1, 0, 0, 0, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
        { { 0, 0, 0, 1, 0, 0, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
        { { 0, 0, 0, 0, 1, 0, 1, 0, 1 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
        { { 0, 0, 0, 0, 0, 1, 0, 1, 0 }, { 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 0 },
    };

    /*
     * reinitialize room graph description
     */
    for (r1 = rdes; r1 <= &rdes[MAXROOMS-1]; r1++)
    {
        for (j = 0; j < MAXROOMS; j++)
            r1->isconn[j] = FALSE;
        r1->ingraph = FALSE;
    }

    /*
     * starting with one room, connect it to a random adjacent room and
     * then pick a new room to start with.
     */
    roomcount = 1;
    r1 = &rdes[rnd(MAXROOMS)];
    r1->ingraph = TRUE;
    do
    {
        /*
         * find a room to connect with
         */
        j = 0;
        for (i = 0; i < MAXROOMS; i++)
            if (r1->conn[i] && !rdes[i].ingraph && rnd(++j) == 0)
                r2 = &rdes[i];
        /*
         * if no adjacent rooms are outside the graph, pick a new room
         * to look from
         */
        if (j == 0)
        {
            do
                r1 = &rdes[rnd(MAXROOMS)];
            until (r1->ingraph);
        }
        /*
         * otherwise, connect new room to the graph, and draw a tunnel
         * to it
         */
        else
        {
            r2->ingraph = TRUE;
            i = (int)(r1 - rdes);
            j = (int)(r2 - rdes);
            conn(i, j);
            r1->isconn[j] = TRUE;
            r2->isconn[i] = TRUE;
            roomcount++;
        }
    } while (roomcount < MAXROOMS);

    /*
     * attempt to add passages to the graph a random number of times so
     * that there isn't always just one unique passage through it.
     */
    for (roomcount = rnd(5); roomcount > 0; roomcount--)
    {
        r1 = &rdes[rnd(MAXROOMS)];        /* a random room to look from */
        /*
         * find an adjacent room not already connected
         */
        j = 0;
        for (i = 0; i < MAXROOMS; i++)
            if (r1->conn[i] && !r1->isconn[i] && rnd(++j) == 0)
                r2 = &rdes[i];
        /*
         * if there is one, connect it and look for the next added
         * passage
         */
        if (j != 0)
        {
            i = (int)(r1 - rdes);
            j = (int)(r2 - rdes);
            conn(i, j);
            r1->isconn[j] = TRUE;
            r2->isconn[i] = TRUE;
        }
    }
    passnum();
}

/*
 * conn:
 *        Draw a corridor from a room in a certain direction.
 */

void ClassicMapGenerator::conn(int r1, int r2)
{
    struct room *rpf, *rpt = NULL;
    int rmt;
    int distance = 0, turn_spot, turn_distance = 0;
    int rm;
    char direc;
    static coord del, curr, turn_delta, spos, epos;

    if (r1 < r2)
    {
        rm = r1;
        if (r1 + 1 == r2)
            direc = 'r';
        else
            direc = 'd';
    }
    else
    {
        rm = r2;
        if (r2 + 1 == r1)
            direc = 'r';
        else
            direc = 'd';
    }
    rpf = &rooms[rm];
    /*
     * Set up the movement variables, in two cases:
     * first drawing one down.
     */
    if (direc == 'd')
    {
        rmt = rm + 3;                                /* room # of dest */
        rpt = &rooms[rmt];                        /* room pointer of dest */
        del.x = 0;                                /* direction of move */
        del.y = 1;
        spos.x = rpf->r_pos.x;                        /* start of move */
        spos.y = rpf->r_pos.y;
        epos.x = rpt->r_pos.x;                        /* end of move */
        epos.y = rpt->r_pos.y;
        if (!(rpf->r_flags & ISGONE))                /* if not gone pick door pos */
            do
            {
                spos.x = rpf->r_pos.x + rnd(rpf->r_max.x - 2) + 1;
                spos.y = rpf->r_pos.y + rpf->r_max.y - 1;
            } while ((rpf->r_flags&ISMAZE) && !(flat(spos.y, spos.x)&F_PASS));
        if (!(rpt->r_flags & ISGONE))
            do
            {
                epos.x = rpt->r_pos.x + rnd(rpt->r_max.x - 2) + 1;
            } while ((rpt->r_flags&ISMAZE) && !(flat(epos.y, epos.x)&F_PASS));
        distance = abs(spos.y - epos.y) - 1;        /* distance to move */
        turn_delta.y = 0;                        /* direction to turn */
        turn_delta.x = (spos.x < epos.x ? 1 : -1);
        turn_distance = abs(spos.x - epos.x);        /* how far to turn */
    }
    else if (direc == 'r')                        /* setup for moving right */
    {
        rmt = rm + 1;
        rpt = &rooms[rmt];
        del.x = 1;
        del.y = 0;
        spos.x = rpf->r_pos.x;
        spos.y = rpf->r_pos.y;
        epos.x = rpt->r_pos.x;
        epos.y = rpt->r_pos.y;
        if (!(rpf->r_flags & ISGONE))
            do
            {
                spos.x = rpf->r_pos.x + rpf->r_max.x - 1;
                spos.y = rpf->r_pos.y + rnd(rpf->r_max.y - 2) + 1;
            } while ((rpf->r_flags&ISMAZE) && !(flat(spos.y, spos.x)&F_PASS));
        if (!(rpt->r_flags & ISGONE))
            do
            {
                epos.y = rpt->r_pos.y + rnd(rpt->r_max.y - 2) + 1;
            } while ((rpt->r_flags&ISMAZE) && !(flat(epos.y, epos.x)&F_PASS));
        distance = abs(spos.x - epos.x) - 1;
        turn_delta.y = (spos.y < epos.y ? 1 : -1);
        turn_delta.x = 0;
        turn_distance = abs(spos.y - epos.y);
    }

    turn_spot = rnd(distance - 1) + 1;                /* where turn starts */

    /*
     * Draw in the doors on either side of the passage or just put #'s
     * if the rooms are gone.
     */
    if (!(rpf->r_flags & ISGONE))
        door(rpf, &spos);
    else
        placePassage(spos);
    if (!(rpt->r_flags & ISGONE))
        door(rpt, &epos);
    else
        placePassage(epos);
    /*
     * Get ready to move...
     */
    curr.x = spos.x;
    curr.y = spos.y;
    while (distance > 0)
    {
        /*
         * Move to new position
         */
        curr.x += del.x;
        curr.y += del.y;
        /*
         * Check if we are at the turn place, if so do the turn
         */
        if (distance == turn_spot)
            while (turn_distance--)
            {
                placePassage(curr);
                curr.x += turn_delta.x;
                curr.y += turn_delta.y;
            }
        /*
         * Continue digging along
         */
        placePassage(curr);
        distance--;
    }
    curr.x += del.x;
    curr.y += del.y;
    if (!ce(curr, epos))
        msg("warning, connectivity problem on this level");
}

/*
 * door:
 *        Add a door or possibly a secret door.  Also enters the door in
 *        the exits array of the room.
 */

void ClassicMapGenerator::door(struct room *rm, coord *cp)
{
    PLACE *pp;

    rm->r_exit[rm->r_nexits++] = *cp;

    if (rm->r_flags & ISMAZE)
        return;

    pp = INDEX(cp->y, cp->x);
    if (rnd(10) + 1 < level && rnd(5) == 0)
    {
        if (cp->y == rm->r_pos.y || cp->y == rm->r_pos.y + rm->r_max.y - 1)
            pp->p_ch = WALL_H;
        else
            pp->p_ch = WALL_V;
        pp->p_flags &= ~F_REAL;
    }
    else
        pp->p_ch = DOOR;
}

/*
 * passnum:
 *        Assign a number to each passageway
 */
static int pnum;
static bool newpnum;

void ClassicMapGenerator::passnum()
{
    struct room *rp;
    int i;

    pnum = 0;
    newpnum = FALSE;
    for (rp = passages; rp < &passages[MAXPASS]; rp++)
        rp->r_nexits = 0;
    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
        for (i = 0; i < rp->r_nexits; i++)
        {
            newpnum++;
            numpass(rp->r_exit[i].y, rp->r_exit[i].x);
        }
}

/*
 * numpass:
 *        Number a passageway square and its brethren
 */

void ClassicMapGenerator::numpass(int y, int x)
{
    int *fp;
    struct room *rp;
    int ch;

    if (x >= NUMCOLS || x < 0 || y >= NUMLINES || y <= 0)
        return;
    fp = &flat(y, x);
    if (*fp & F_PNUM)
        return;
    if (newpnum)
    {
        pnum++;
        newpnum = FALSE;
    }
    /*
     * check to see if it is a door or secret door, i.e., a new exit,
     * or a numerable type of place
     */
    if ((ch = char_at(x, y)) == DOOR ||
        (!(*fp & F_REAL) && IS_WALL(ch)))
    {
        rp = &passages[pnum];
        rp->r_exit[rp->r_nexits].y = y;
        rp->r_exit[rp->r_nexits++].x = x;
    }
    else if (!(*fp & F_PASS))
        return;
    *fp |= pnum;
    /*
     * recurse on the surrounding places
     */
    numpass(y + 1, x);
    numpass(y - 1, x);
    numpass(y, x + 1);
    numpass(y, x - 1);
}
