#ifndef AREAS_H
#define AREAS_H

#include <vector>

#include "rogue.h"

/**
    Areas are used to designated square areas in the map as "rooms". This is used in the code to spawn random monster, and teleport.
    Monsters only spawn in areas, and teleport only happens towards areas.
    
    If the map generator does not define any areas, monsters will spawn everywhere possible, as well as random teleportation is allowed to everywhere.
*/
class Area
{
public:
    coord position;  /* Upper left corner of the area */
    coord size;      /* Size of the area */
    
    Area();
    Area(coord position, coord size);
    
    enum ERandomPositionType
    {
        ForMonster,
        ForItem
    };
    static coord random_position(ERandomPositionType type);
};

extern std::vector<Area> areas;

#endif//AREAS_H
