#include "areas.h"

std::vector<Area> areas;


Area::Area()
{
    position.x = position.y = 0;
    size.x = size.y = 0;
}

Area::Area(coord position, coord size)
: position(position), size(size)
{
}

coord Area::random_position(ERandomPositionType type)
{
    Area area;
    area.size.x = NUMCOLS;
    area.size.y = NUMLINES;
    
    while(true)
    {
        if (areas.size() > 0)
        {
            area = areas[rnd(areas.size())];
        }
        coord co;
        co.x = area.position.x + rnd(area.size.x);
        co.y = area.position.y + rnd(area.size.y);
        if (step_ok(char_at(co.x, co.y)))
        {
            switch(type)
            {
            case ForMonster:
                if (!monster_at(co.x, co.y) && !ce(co, hero))
                    return co;
                break;
            case ForItem:
                if (!item_at(co.x, co.y) && !ce(co, hero))
                    return co;
                break;
            }
        }
    }
}
