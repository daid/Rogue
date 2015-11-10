#ifndef MAPGEN_UTILITIES_H
#define MAPGEN_UTILITIES_H

void placePassage(coord co);
void placeRandomTraps();
void placeRandomStairs();
void placeRandomHero();
void placeRandomItems();
void placeAmuletIfRequired();
void placeDoor(int x, int y);

#endif//MAPGEN_UTILITIES_H
