#include <string.h>
#include "../../rogue.h"
#include "../../areas.h"
#include "../util.h"
#include "cave.h"

CaveMapGenerator::CaveMapGenerator()
{
}

bool CaveMapGenerator::generate()
{
    //First fill the whole map with solid walls
    for(int y=0; y<NUMLINES; y++)
        for(int x=0; x<NUMCOLS; x++)
            char_at(x, y) = SOLID_WALL;

    //Now randomly walk the map and cut out walls, till 1/4 of the map is cleared.
    int x = NUMCOLS / 2;
    int y = NUMLINES / 2;
    for(int n=0; n<NUMCOLS*NUMLINES/4; n++)
    {
        int dir = -1;
        while(char_at(x, y) == FLOOR)
        {
            if (rnd(5) == 0) dir = rnd(6);
            switch(dir)
            {
            case 0:
            case 1:
                x++;
                break;
            case 2:
            case 3:
                x--;
                break;
            case 4:
                y++;
                break;
            case 5:
                y--;
                break;
            }
            if (x < 1 || x >= NUMCOLS - 1 || y < 1 || y >= NUMLINES - 1)
            {
                do
                {
                    x = rnd(NUMCOLS - 2) + 1;
                    y = rnd(NUMLINES - 2) + 1;
                }while(char_at(x, y) != FLOOR);
            }
        }
        char_at(x, y) = FLOOR;
    }
    
    //Add some random monsters on the map
    for(int n=0; n<8; n++)
    {
        if (rnd(100) < 50)
        {
            coord co = Area::random_position(Area::ForMonster);
            MonsterThing* mp = new MonsterThing(randmonster(FALSE), co);
            give_pack(mp);
            
            if (on(*mp, ISGREED) || rnd(2) == 0)
            {
                //When a monster is greedy, or by chance, add gold nearby.
                ItemThing* gold = new ItemThing();
                gold->type = GOLD;
                gold->arm = GOLDCALC;
                gold->pos = co;
                fall(gold, false);
            }
        }
    }

    placeRandomItems();
    placeAmuletIfRequired();
    placeRandomTraps();
    placeRandomStairs();
    placeRandomHero();
    
    return true;
}
