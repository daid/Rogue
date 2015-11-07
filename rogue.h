/*
 * Rogue definitions and variable declarations
 *
 * @(#)rogue.h        5.42 (Berkeley) 08/06/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */
#ifndef ROGUE_INCLUDE_H
#define ROGUE_INCLUDE_H

#include <list>
#include "extern.h"
#include "line_of_sight.h"

/*
 * Maximum number of different things
 */
#define MAXTHINGS        9
#define MAXOBJ           15   /* changed from 9 to increase number of items found in game */
#define MAXPACK          23
#define MAXTRAPS         10
#define AMULETLEVEL      26
#define NUMTHINGS        7    /* number of types of things */
#define NUMLINES         24
#define NUMCOLS          80
#define BORE_LEVEL       50

/*
 * return values for get functions
 */
#define        NORM        0        /* normal exit */
#define        QUIT        1        /* quit option setting */

/*
 * All the fun defines
 */
#define when                break;case
#define otherwise        break;default
#define until(expr)        while(!(expr))
#define ce(a,b)                ((a).x == (b).x && (a).y == (b).y)
#define hero                player.pos
template<typename T> T max(const T a, const T b) { if (a > b) return a; return b; }
#define on(thing,flag)        ((bool)(((thing).flags & (flag)) != 0))
#define GOLDCALC        (rnd(50 + 10 * level) + 2)
#define ISRING(h,r)        (cur_ring[h] != NULL && cur_ring[h]->which == r)
#define ISWEARING(r)        (ISRING(LEFT, r) || ISRING(RIGHT, r))
#define ISMULT(type)         (type == POTION || type == SCROLL || type == FOOD)
#define INDEX(y,x)        (&places[((x) << 5) + (y)])
#define char_at(x,y)     (places[((x) << 5) + (y)].p_ch)
#define flat(y,x)        (places[((x) << 5) + (y)].p_flags)
#define item_at(x,y)     (places[((x) << 5) + (y)].p_item)
#define monster_at(x,y)  (places[((x) << 5) + (y)].p_monst)
#define unc(cp)                (cp).y, (cp).x

/*
 * things that appear on the screens
 */
#define PASSAGE             '\x02'
#define PASSAGE_UNLIT       '\x01'
#define DOOR                '\x16'
#define FLOOR               '.'
#define PLAYER              '@'
#define TRAP                '\x05'
#define STAIRS              '\x07'
#define GOLD                '*'
#define POTION              '\x04'
#define SCROLL              '\x08'
#define MAGIC               '$'
#define FOOD                '\x09'
#define WEAPON              ')'
#define ARMOR               '\x03'
#define AMULET              '\x0a'
#define RING                '\x06'
#define STICK               '/'
#define WALL_V              '\x10'
#define WALL_H              '\x11'
#define WALL_TL             '\x12'
#define WALL_TR             '\x13'
#define WALL_BL             '\x14'
#define WALL_BR             '\x15'

//CALLABLE or RING/STICK defines to used in inventory selection lists
#define CALLABLE            -1
#define R_OR_S              -2

#define IS_WALL(n) ((n) == '|' || (n) == '-' || (n) == WALL_V || (n) == WALL_H || (n) == WALL_TL || (n) == WALL_TR || (n) == WALL_BL || (n) == WALL_BR)

/*
 * Various constants
 */
#define BEARTIME        spread(3)
#define SLEEPTIME       spread(5)
#define HOLDTIME        spread(2)
#define WANDERTIME      spread(70)
#define BEFORE          spread(1)
#define AFTER           spread(2)
#define HEALTIME        30
#define HUHDURATION     20
#define SEEDURATION     850
#define HUNGERTIME      1300
#define MORETIME        150
#define STOMACHSIZE     2000
#define STARVETIME      850
#define ESCAPE          27
#define LEFT            0
#define RIGHT           1
#define BOLT_LENGTH     6
#define LAMPDIST        11   /* Distance squared! */

/*
 * Save against things
 */
#define VS_POISON        00
#define VS_PARALYZATION        00
#define VS_DEATH        00
#define VS_BREATH        02
#define VS_MAGIC        03

/*
 * Various flag bits
 */
/* flags for objects */
#define ISCURSED 000001                /* object is cursed */
#define ISKNOW        0000002                /* player knows details about the object */
#define ISMISL        0000004                /* object is a missile type */
#define ISMANY        0000010                /* object comes in groups */
/*        ISFOUND 0000020                ...is used for both objects and creatures */
#define        ISPROT        0000040                /* armor is permanently protected */

/* flags for creatures */
#define CANHUH       0000001                /* creature can confuse */
#define CANSEE       0000002                /* creature can see invisible creatures */
#define ISBLIND      0000004                /* creature is blind */
#define ISCANC       0000010                /* creature has special qualities cancelled */
#define ISLEVIT      0000010                /* hero is levitating */
#define ISFOUND      0000020                /* creature has been seen (used for objects) */
#define ISGREED      0000040                /* creature runs to protect gold */
#define ISHASTE      0000100                /* creature has been hastened */
#define ISTARGET     000200                /* creature is the target of an 'f' command */
#define ISHELD       0000400                /* creature has been held */
#define ISHUH        0001000                /* creature is confused */
#define ISINVIS      0002000                /* creature is invisible */
#define ISMEAN       0004000                /* creature can wake when player enters room */
#define ISHALU       0004000                /* hero is on acid trip */
#define ISREGEN      0010000                /* creature can regenerate */
#define ISRUN        0020000                /* creature is running at the player */
#define SEEMONST     0040000                /* hero can detect unseen monsters */
#define ISFLY        0040000                /* creature can fly */
#define ISSLOW       0100000                /* creature has been slowed */

/*
 * Flags for level map
 */
#define F_PASS                0x0080                /* is a passageway */
#define F_SEEN                0x0040                /* have seen this spot before */
#define F_TMASK               0x0007                /* trap number mask */
#define F_LOCKED              0x0020                /* door is locked */
#define F_REAL                0x0010                /* what you see is what you get */
#define F_PNUM                0x000f                /* passage number mask */
#define F_TMASK               0x0007                /* trap number mask */
#define F_ISLIT               0x0100                /* place is lit by static light */

/*
 * Trap types
 */
#define T_DOOR        00
#define T_ARROW        01
#define T_SLEEP        02
#define T_BEAR        03
#define T_TELEP        04
#define T_DART        05
#define T_RUST        06
#define T_MYST  07
#define NTRAPS        8

/*
 * Potion types
 */
#define P_CONFUSE       0
#define P_LSD           1
#define P_POISON        2
#define P_STRENGTH      3
#define P_SEEINVIS      4
#define P_HEALING       5
#define P_MFIND         6
#define P_TFIND         7
#define P_RAISE         8
#define P_XHEAL         9
#define P_HASTE         10
#define P_RESTORE       11
#define P_BLIND         12
#define P_LEVIT         13
#define MAXPOTIONS      14

/*
 * Scroll types
 */
#define S_CONFUSE        0
#define S_MAP                1
#define S_HOLD                2
#define S_SLEEP                3
#define S_ARMOR                4
#define S_ID_POTION        5
#define S_ID_SCROLL        6
#define S_ID_WEAPON        7
#define S_ID_ARMOR        8
#define S_ID_R_OR_S        9
#define S_SCARE                10
#define S_FDET                11
#define S_TELEP                12
#define S_ENCH                13
#define S_CREATE        14
#define S_REMOVE        15
#define S_AGGR                16
#define S_PROTECT        17
#define MAXSCROLLS        18

/*
 * Weapon types
 */
#define MACE                0
#define SWORD                1
#define BOW                2
#define ARROW                3
#define DAGGER                4
#define TWOSWORD        5
#define DART                6
#define SHIRAKEN        7
#define SPEAR                8
#define FLAME                9        /* fake entry for dragon breath (ick) */
#define MAXWEAPONS        9        /* this should equal FLAME */

/*
 * Armor types
 */
#define LEATHER                0
#define RING_MAIL        1
#define STUDDED_LEATHER        2
#define SCALE_MAIL        3
#define CHAIN_MAIL        4
#define SPLINT_MAIL        5
#define BANDED_MAIL        6
#define PLATE_MAIL        7
#define MAXARMORS        8

/*
 * Ring types
 */
#define R_PROTECT        0
#define R_ADDSTR        1
#define R_SUSTSTR        2
#define R_SEARCH        3
#define R_SEEINVIS        4
#define R_NOP                5
#define R_AGGR                6
#define R_ADDHIT        7
#define R_ADDDAM        8
#define R_REGEN                9
#define R_DIGEST        10
#define R_TELEPORT        11
#define R_STEALTH        12
#define R_SUSTARM        13
#define MAXRINGS        14

/*
 * Rod/Wand/Staff types
 */
#define WS_LIGHT        0
#define WS_INVIS        1
#define WS_ELECT        2
#define WS_FIRE                3
#define WS_COLD                4
#define WS_POLYMORPH        5
#define WS_MISSILE        6
#define WS_HASTE_M        7
#define WS_SLOW_M        8
#define WS_DRAIN        9
#define WS_NOP                10
#define WS_TELAWAY        11
#define WS_TELTO        12
#define WS_CANCEL        13
#define MAXSTICKS        14

/*
 * Now we define the structures and types
 */

/*
 * Help list
 */
struct h_list {
    int h_ch;
    const char *h_desc;
    bool h_print;
};

/*
 * Coordinate data type
 */
typedef struct {
    int x;
    int y;
} coord;

typedef unsigned int str_t;

/*
 * Stuff about objects
 */
struct obj_info {
    const char *oi_name;
    int oi_prob;
    int oi_worth;
    char *oi_guess;
    bool oi_know;
};

/*
 * Structure describing a fighting being
 */
struct stats {
    str_t s_str;                        /* Strength */
    int s_exp;                                /* Experience */
    int s_lvl;                                /* level of mastery */
    int s_arm;                                /* Armor class */
    int s_hpt;                        /* Hit points */
    char s_dmg[13];                        /* String describing damage done */
    int  s_maxhp;                        /* Max hit points */
};

/*
 * Structure for monsters and player
 */
/* structure for items/objects */
class ItemThing
{
public:
    ItemThing();
    
    int type;                        /* What kind of object it is */
    coord pos;                        /* Where it lives on the screen */
    int  launch;                        /* What you need to launch it */
    char packch;                        /* What character it is in the pack */
    char damage[8];                /* Damage if used like sword */
    char hurldmg[8];                /* Damage if thrown */
    int count;                        /* count for plural objects */
    int which;                        /* Which object of a type it is */
    int hplus;                        /* Plusses to hit */
    int dplus;                        /* Plusses to damage */
    int arm;                        /* Armor protection */
    int flags;                        /* information about objects */
    int group;                        /* group number for this object */
    char *label;                        /* Label for object */
};

class MonsterThing
{
public:    
    coord pos;                          /* Position */
    bool turn;                          /* If slowed, is it a turn to move */
    char type;                          /* What it is */
    char disguise;                      /* What mimic looks like */
    coord *dest;                        /* Where it is running to */
    short flags;                        /* State word */
    struct stats stats;                 /* Physical description */
    std::list<ItemThing*> pack;         /* What the thing is carrying */
    int reserved;                       /* Used during saving/loading */

public:
    MonsterThing();
    MonsterThing(char type, const coord& cp);

    void polymorph(char new_type);
    
    int experienceAdd();
};

typedef int(*daemon_function_t)(int);

/*
 * describe a place on the level map
 */
typedef struct {
    char p_ch;
    int p_flags;
    ItemThing* p_item;
    MonsterThing* p_monst;
} PLACE;

/*
 * Array containing information on all the various types of monsters
 */
struct monster {
    const char *m_name;                 /* What to call the monster */
    int m_carry;                        /* Probability of carrying something */
    short m_flags;                      /* things about the monster */
    struct stats m_stats;               /* Initial stats */
};

/*
 * External variables
 */

extern bool     after, again, allscore, amulet, door_stop, fight_flush,
                firstmove, has_hit, inv_describe, jump, kamikaze,
                lower_msg, move_on, pack_used[],
                passgo, playing, running, save_msg,
                seenstairs, stat_msg, terse, to_death, tombstone;

extern char     file_name[], huh[], *Numname, outbuf[], take;

extern const char* version;
extern const char* release;

extern const char *p_colors[];
extern const char *r_stones[];
extern char *s_names[];
extern const char *tr_name[];
extern const char *ws_made[];
extern const char *ws_type[];

extern int      a_class[], count, food_left, hungry_state, inpack,
                inv_type, lastscore, level, max_hit, max_level, runch,
                n_objs, no_command, no_food, no_move, purse,
                l_last_comm, l_last_dir, last_comm, last_dir, dir_ch,
                quiet, vf_hit;

extern unsigned int        numscores;

extern int        dnum, e_levels[];

extern coord        delta, oldpos, stairs;

extern PLACE        places[];

extern ItemThing *cur_armor, *cur_ring[], *cur_weapon, *l_last_pick,
                  *last_pick;
extern std::list<ItemThing*> lvl_obj;
extern std::list<MonsterThing*> mlist;
extern MonsterThing player;

extern struct h_list        helpstr[];

extern struct stats        max_stats;

extern struct monster        monsters[];

extern struct obj_info        arm_info[], pot_info[], ring_info[],
                        scr_info[], things[], ws_info[], weap_info[];

/*
 * Function types
 */
void        addmsg(const char *fmt, ...);
bool        add_haste(bool potion);
void        add_pack(ItemThing *obj, bool silent);
void        add_pass();
void        add_str(str_t *sp, int amt);
void        aggravate();
int        attack(MonsterThing *mp);
void        badcheck(const char *name, struct obj_info *info, int bound);
void        bounce(ItemThing *weap, const char *mname, bool noend);
void        call();
void        call_it(struct obj_info *info);
bool        cansee(int y, int x);
void        chg_str(int amt);
void        check_level();
void        command();
void        create_obj();
void        current(ItemThing *cur, const char *how, const char *where);
void        d_level();
void        death(char monst);
char        death_monst();
void        discovered();
int        dist(int y1, int x1, int y2, int x2);
int        dist_cp(coord *c1, coord *c2);
int        do_chase(MonsterThing *th);
void        do_daemons(int flag);
void        do_fuses(int flag);
void        do_motion(ItemThing *obj, int ydelta, int xdelta);
void        do_move(int dy, int dx);
void        do_pot(int type, bool knowit);
void        hit_by_potion(int type, MonsterThing* thing);
void        do_run(int ch);
void        do_zap();
void        doadd(const char *fmt, va_list args);
void        drain();
void        drop();
void        eat();
size_t  encread(void *start, size_t size, FILE *inf);
size_t  encwrite(const void *start, size_t size, FILE *outf);
int        endmsg();
void        erase_lamp(coord& pos);
void        extinguish(daemon_function_t func);
void        fall(ItemThing *obj, bool pr);
void        fire_bolt(coord *start, coord *dir, const char *name);
void        flush_type();
int        fight(coord *mp, ItemThing *weap, bool thrown);
void        fix_stick(ItemThing *cur);
void        fuse(daemon_function_t func, int arg, int time, int type);
bool        get_dir();
int        gethand();
void        give_pack(MonsterThing *tp);
void        help();
void        hit(const char *er, const char *ee, bool noend);
void        lengthen(daemon_function_t func, int xtime);
void        look(bool wakeup);
int        hit_monster(int y, int x, ItemThing *obj);
void        identify();
void        illcom(int ch);
void        init_check();
void        init_colors();
void        init_materials();
void        init_names();
void        init_player();
void        init_probs();
void        init_stones();
void        init_weapon(ItemThing *weap, int which);
int         inventory(int type);
void        invis_on();
void        killed(MonsterThing *tp, bool pr);
void        kill_daemon(daemon_function_t func);
bool        lock_sc();
void        miss(const char *er, const char *ee, bool noend);
void        missile(int ydelta, int xdelta);
void        money(int value);
int        move_monst(MonsterThing *tp);
void        move_msg(ItemThing *obj);
int        msg(const char *fmt, ...);
void        nameit(ItemThing *obj, const char *type, const char *which, struct obj_info *op, const char *(*prfunc)(ItemThing *));
void        new_level();
void        option();
void        open_score();
const char* pick_color(const char *col);
int         pick_one(struct obj_info *info, int nitems);
void        pick_up(int ch);
void        picky_inven();
void        pr_spec(struct obj_info *info, int nitems);
void        pr_list();
void        quaff();
void        raise_level();
char        randmonster(bool wander);
void        read_scroll();
void    relocate(MonsterThing *th, coord *new_loc);
void        remove_mon(coord *mp, MonsterThing *tp, bool waskill);
void        reset_last();
bool        restore(const char *file);
int        ring_eat(int hand);
void        ring_on();
void        ring_off();
int        rnd(int range);
int        roll(int number, int sides);
int        rs_save_file(FILE *savef);
int        rs_restore_file(FILE *inf);
void        runto(const coord& runner);
void        rust_armor(ItemThing *arm);
int        save(int which);
void        save_file(FILE *savef);
void        save_game();
int        save_throw(int which, MonsterThing *tp);
void        score(int amount, int flags, char monst);
void        search();
void        set_know(ItemThing *obj, struct obj_info *info);
void        setup();
void        show_map();
int        sign(int nm);
int        spread(int nm);
void        start_daemon(daemon_function_t func, int arg, int type);
void        start_score();
void        status();
int        step_ok(int ch);
int        swing(int at_lvl, int op_arm, int wplus);
void        take_off();
void        teleport();
void        total_winner();
void        thunk(ItemThing *weap, const char *mname, bool noend);
void        turnref();
void        u_level();
void        uncurse(ItemThing *obj);
void        unlock_sc();
MonsterThing *wake_monster(int y, int x);
void        wanderer();
void        waste_time();
void        wear();
void        whatis(bool insist, int type);
void        wield();

bool        chase(MonsterThing *tp, coord *ee);
bool        diag_ok(coord *sp, coord *ep);
bool        dropcheck(ItemThing *obj);
bool        fallpos(coord *pos, coord *newpos);
bool        is_magic(ItemThing *obj);
bool        levit_check();
bool        pack_room(bool from_floor, ItemThing *obj);
bool        roll_em(MonsterThing *thatt, MonsterThing *thdef, ItemThing *weap, bool hurl);
bool        see_monst(MonsterThing *mp);
bool        seen_stairs();
bool        turn_ok(int y, int x);
int         turn_see(int turn_off);
bool        is_current(ItemThing *obj);

char        be_trapped(coord *tc);
char        pack_char();
char        rnd_thing();

const char  *charge_str(ItemThing *obj);
const char  *choose_str(const char *ts, const char *ns);
const char  *inv_name(ItemThing *obj, bool drop);
const char  *nullstr(ItemThing *ignored);
const char  *num(int n1, int n2, char type);
const char  *ring_num(ItemThing *obj);
const char  *set_mname(MonsterThing *tp);
const char  *vowelstr(const char *str);

int        trip_ch(int y, int x, int ch);
int        char_at_place(int x, int y);

coord        *find_dest(MonsterThing *tp);
coord        *rndmove(MonsterThing *who);

ItemThing   *get_item(const char *purpose, int type);
ItemThing   *leave_pack(ItemThing *obj, bool newobj, bool all);
ItemThing   *new_thing();

#define MAXDAEMONS 20

extern struct delayed_action {
    int d_type;
    daemon_function_t d_func;
    int d_arg;
    int d_time;
} d_list[MAXDAEMONS];

typedef struct {
    const char *st_name;
    int         st_value;
} STONE;

extern int        between;
extern int        group;
extern coord      nh;
extern const char *rainbow[];
extern int        cNCOLORS;
extern STONE      stones[];
extern int        cNSTONES;
extern const char *wood[];
extern int        cNWOOD;
extern const char *metal[];
extern int        cNMETAL;

#endif//ROGUE_INCLUDE_H
