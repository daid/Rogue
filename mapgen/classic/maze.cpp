#include "../../rogue.h"
#include "../util.h"
#include "maze.h"

ClassicMazeGenerator::ClassicMazeGenerator()
{
    SPOT *sp;
    
    for (sp = &maze[0][0]; sp <= &maze[NUMLINES-1][NUMCOLS-1]; sp++)
    {
        sp->used = FALSE;
        sp->nexits = 0;
    }
}
/*
 * do_maze:
 *        Dig a maze
 */
void ClassicMazeGenerator::generate(coord position, coord size)
{
    int starty, startx;
    coord pos;
    Maxy = size.y;
    Maxx = size.x;
    Starty = position.y;
    Startx = position.x;
    starty = (rnd(size.y) / 2) * 2;
    startx = (rnd(size.x) / 2) * 2;
    pos.y = starty + Starty;
    pos.x = startx + Startx;
    placePassage(pos);
    dig(starty, startx);
}

/*
 * dig:
 *        Dig out from around where we are now, if possible
 */

void ClassicMazeGenerator::dig(int y, int x)
{
    coord *cp;
    int cnt, newy, newx, nexty = 0, nextx = 0;
    static coord pos;
    static coord del[4] = {
        {2, 0}, {-2, 0}, {0, 2}, {0, -2}
    };

    for (;;)
    {
        cnt = 0;
        for (cp = del; cp <= &del[3]; cp++)
        {
            newy = y + cp->y;
            newx = x + cp->x;
            if (newy < 0 || newy > Maxy || newx < 0 || newx > Maxx)
                continue;
            if (flat(newy + Starty, newx + Startx) & F_PASS)
                continue;
            if (rnd(++cnt) == 0)
            {
                nexty = newy;
                nextx = newx;
            }
        }
        if (cnt == 0)
            return;
        accnt_maze(y, x, nexty, nextx);
        accnt_maze(nexty, nextx, y, x);
        if (nexty == y)
        {
            pos.y = y + Starty;
            if (nextx - x < 0)
                pos.x = nextx + Startx + 1;
            else
                pos.x = nextx + Startx - 1;
        }
        else
        {
            pos.x = x + Startx;
            if (nexty - y < 0)
                pos.y = nexty + Starty + 1;
            else
                pos.y = nexty + Starty - 1;
        }
        placePassage(pos);
        pos.y = nexty + Starty;
        pos.x = nextx + Startx;
        placePassage(pos);
        dig(nexty, nextx);
    }
}

/*
 * accnt_maze:
 *        Account for maze exits
 */

void ClassicMazeGenerator::accnt_maze(int y, int x, int ny, int nx)
{
    SPOT *sp;
    coord *cp;

    sp = &maze[y][x];
    for (cp = sp->exits; cp < &sp->exits[sp->nexits]; cp++)
        if (cp->y == ny && cp->x == nx)
            return;
    cp->y = ny;
    cp->x = nx;
}
