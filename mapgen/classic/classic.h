#ifndef CLASSIC_MAP_GEN_H
#define CLASSIC_MAP_GEN_H

class ClassicMapGenerator
{
private:
    static constexpr int MAXROOMS = 9;
    static constexpr int MAXPASS  = 13;  /* upper limit on number of passages */

    /* flags for rooms */
    static constexpr int ISDARK = 0000001;                /* room is dark */
    static constexpr int ISGONE = 0000002;                /* room is gone (a corridor) */
    static constexpr int ISMAZE = 0000004;                /* room is a maze */

    /*
     * Room structure
     */
    struct room {
        coord r_pos;                        /* Upper left corner */
        coord r_max;                        /* Size of room */
        int r_goldval;                        /* How much the gold is worth */
        short r_flags;                        /* info about the room */
        int r_nexits;                        /* Number of exits */
        coord r_exit[12];                        /* Where the exits are */
    };

    struct room rooms[MAXROOMS];  /* One for each room -- A level */
    struct room passages[MAXPASS];/* One for each passage */

    void do_rooms();
    int rnd_room();
    bool find_floor(struct room *rp, coord *cp, int limit, bool monst);
    void rnd_pos(struct room *rp, coord *cp);

    void draw_room(struct room *rp);
    void vert(struct room *rp, int startx);
    void horiz(struct room *rp, int starty);

    void put_things();
    void treas_room();
    
    void do_passages();
    void conn(int r1, int r2);
    void door(struct room *rm, coord *cp);
    void passnum();
    void numpass(int y, int x);
public:
    ClassicMapGenerator();
    
    bool generate();
};

#endif//CLASSIC_MAP_GEN_H
