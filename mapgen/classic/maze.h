#ifndef CLASSIC_MAZE_GEN_H
#define CLASSIC_MAZE_GEN_H

class ClassicMazeGenerator
{
private:
    typedef struct {                /* position matrix for maze positions */
        int    nexits;
        coord  exits[4];
        int    used;
    } SPOT;

    int Maxy, Maxx, Starty, Startx;

    SPOT maze[NUMLINES/3+1][NUMCOLS/3+1];//TODO: This is fixed size by assumption

    void dig(int y, int x);
    void accnt_maze(int y, int x, int ny, int nx);
public:
    ClassicMazeGenerator();
    
    void generate(coord position, coord size);
};

#endif//CLASSIC_MAZE_GEN_H
