#include "areas.h"

std::vector<Area> areas;


Area::Area()
{
    position.x = position.y = 0;
    size.x = size.y = 0;
    flags = 0;
}

Area::Area(coord position, coord size)
: position(position), size(size)
{
    flags = 0;
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
            do
            {
                area = areas[rnd(areas.size())];
            }while(!area.allowsRandomPositionType(type));
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

bool Area::allowsRandomPositionType(ERandomPositionType type)
{
    switch(type)
    {
    case ForMonster:
        return !(flags & NoMonsters);
    case ForItem:
        return !(flags & NoItems);
    default:
        return true;
    }
}
