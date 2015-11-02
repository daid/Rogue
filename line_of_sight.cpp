#include <functional>
#include "rogue.h"

bool has_line_of_sight(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1-x0);
    int sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0);
    int sy = y0<y1 ? 1 : -1; 
    int err = (dx>dy ? dx : -dy)/2, e2;

    for(;;)
    {
        int ch = ' ';
        if (x0 >= 0 && x0 < MAXCOLS && y0 >= 0 && y0 < MAXLINES)
            ch = chat(y0, x0);
        if (IS_WALL(ch) || ch == ' ')
            return false;
        if (x0==x1 && y0==y1)
            break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }

        if (x0==x1 && y0==y1 && !(ch == PASSAGE || ch == PASSAGE2))
            break;
    }
    return true;
}

void visit_field_of_view(int x, int y, int distance, std::function<void(int, int)> callback)
{
    for(int _x=x-distance; _x<=x+distance; _x++)
    {
        for(int _y=y-distance; _y<=y+distance; _y++)
        {
            if (has_line_of_sight(x, y, _x, _y))
            {
                coord cp = {_x, _y};
                room* r = roomin(cp);
                if (!r || !r->r_flags || dist(y, x, _y, _x) < LAMPDIST)
                {
                    callback(_x, _y);
                }
            }
        }
    }
}
