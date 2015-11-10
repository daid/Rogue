/*
 * Read and execute the user commands
 *
 * @(#)command.c        4.73 (Berkeley) 08/06/83
 *
 * Rogue: Exploring the Dungeons of Doom
 * Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and Glenn Wichman
 * All rights reserved.
 *
 * See the file LICENSE.TXT for full copyright and licensing information.
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rogue.h"

/*
 * command:
 *        Process the user commands
 */
void
command()
{
    register int ch;
    register int ntimes = 1;                        /* Number of player moves */
    int *fp;
    MonsterThing *mp;
    static char countch, direction, newcount = FALSE;

    if (on(player, ISHASTE))
        ntimes++;
    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    while (ntimes--)
    {
        again = FALSE;
        if (has_hit)
        {
            endmsg();
            has_hit = FALSE;
        }
        /*
         * these are illegal things for the player to be, so if any are
         * set, someone's been poking in memeory
         */
        if (on(player, ISSLOW|ISGREED|ISINVIS|ISREGEN|ISTARGET))
            exit(1);

        look(TRUE);
        if (!running)
            door_stop = FALSE;
        status(false);
        lastscore = purse;
        setMapViewTarget(hero.x, hero.y);
        if (!((running || count) && jump))
            refreshMap();                        /* Draw screen */
        take = 0;
        after = TRUE;
        /*
         * Read command or continue run
         */
        if (!no_command)
        {
            if (running || to_death)
                ch = runch;
            else if (count)
                ch = countch;
            else
            {
                ch = md_readchar();
                move_on = FALSE;
            }
        }
        else
            ch = '.';
        if (no_command)
        {
            if (--no_command == 0)
            {
                player.flags |= ISRUN;
                msg("you can move again");
            }
        }
        else
        {
            /*
             * check for prefixes
             */
            newcount = FALSE;
            if (isdigit(ch))
            {
                count = 0;
                newcount = TRUE;
                while (isdigit(ch))
                {
                    count = count * 10 + (ch - '0');
                    if (count > 255)
                        count = 255;
                    ch = md_readchar();
                }
                countch = ch;
                /*
                 * turn off count for commands which don't make sense
                 * to repeat
                 */
                switch (ch)
                {
                    case CTRL('B'): case CTRL('H'): case CTRL('J'):
                    case CTRL('K'): case CTRL('L'): case CTRL('N'):
                    case CTRL('U'): case CTRL('Y'):
                    case '.': case 'a': case 'b': case 'h': case 'j':
                    case 'k': case 'l': case 'm': case 'n': case 'q':
                    case 'r': case 's': case 't': case 'u': case 'y':
                    case 'z': case 'B': case 'C': case 'H': case 'I':
                    case 'J': case 'K': case 'L': case 'N': case 'U':
                    case 'Y': case 'M':
                    case K_UP_LEFT: case K_UP: case K_UP_RIGHT: case K_LEFT:
                    case K_RIGHT: case K_DOWN_LEFT: case K_DOWN: case K_DOWN_RIGHT:
                    case K_SHIFT_UP_LEFT: case K_SHIFT_UP: case K_SHIFT_UP_RIGHT: case K_SHIFT_LEFT:
                    case K_SHIFT_RIGHT: case K_SHIFT_DOWN_LEFT: case K_SHIFT_DOWN: case K_SHIFT_DOWN_RIGHT:
                    case CTRL(K_SHIFT_UP_LEFT): case CTRL(K_SHIFT_UP): case CTRL(K_SHIFT_UP_RIGHT): case CTRL(K_SHIFT_LEFT):
                    case CTRL(K_SHIFT_RIGHT): case CTRL(K_SHIFT_DOWN_LEFT): case CTRL(K_SHIFT_DOWN): case CTRL(K_SHIFT_DOWN_RIGHT):
                        break;
                    default:
                        count = 0;
                }
            }
            /*
             * execute a command
             */
            if (count && !running)
                count--;
            if (ch != 'a' && ch != ESCAPE && !(running || count || to_death))
            {
                l_last_comm = last_comm;
                l_last_dir = last_dir;
                l_last_pick = last_pick;
                last_comm = ch;
                last_dir = '\0';
                last_pick = NULL;
            }
over:
            switch (ch)
            {
                case ',': {
                    ItemThing *found = NULL;
                    for (ItemThing* obj : lvl_obj)
                    {
                        if (obj->pos.y == hero.y && obj->pos.x == hero.x)
                        {
                            found = obj;
                            break;
                        }
                    }

                    if (found) {
                        if (levit_check())
                            ;
                        else
                            pick_up(found->type);
                    } else {
                        if (!terse)
                            addmsg("there is ");
                        addmsg("nothing here");
                        if (!terse)
                            addmsg(" to pick up");
                        endmsg();
                    }
                }
                when 'h': case K_LEFT: do_move(0, -1);
                when 'j': case K_DOWN: do_move(1, 0);
                when 'k': case K_UP: do_move(-1, 0);
                when 'l': case K_RIGHT: do_move(0, 1);
                when 'y': case K_UP_LEFT: do_move(-1, -1);
                when 'u': case K_UP_RIGHT: do_move(-1, 1);
                when 'b': case K_DOWN_LEFT: do_move(1, -1);
                when 'n': case K_DOWN_RIGHT: do_move(1, 1);
                when 'H': case K_SHIFT_LEFT: do_run('h');
                when 'J': case K_SHIFT_DOWN: do_run('j');
                when 'K': case K_SHIFT_UP: do_run('k');
                when 'L': case K_SHIFT_RIGHT: do_run('l');
                when 'Y': case K_SHIFT_UP_LEFT: do_run('y');
                when 'U': case K_SHIFT_UP_RIGHT: do_run('u');
                when 'B': case K_SHIFT_DOWN_LEFT: do_run('b');
                when 'N': case K_SHIFT_DOWN_RIGHT: do_run('n');
                when CTRL('H'): case CTRL('J'): case CTRL('K'): case CTRL('L'):
                case CTRL('Y'): case CTRL('U'): case CTRL('B'): case CTRL('N'):
                case CTRL(K_SHIFT_UP_LEFT): case CTRL(K_SHIFT_UP): case CTRL(K_SHIFT_UP_RIGHT): case CTRL(K_SHIFT_LEFT):
                case CTRL(K_SHIFT_RIGHT): case CTRL(K_SHIFT_DOWN_LEFT): case CTRL(K_SHIFT_DOWN): case CTRL(K_SHIFT_DOWN_RIGHT):
                {
                    if (!on(player, ISBLIND))
                    {
                        door_stop = TRUE;
                        firstmove = TRUE;
                    }
                    if (count && !newcount)
                        ch = direction;
                    else
                    {
                        ch += ('A' - CTRL('A'));
                        direction = ch;
                    }
                    goto over;
                }
                when 'F':
                    kamikaze = TRUE;
                    /* FALLTHROUGH */
                case 'f':
                    if (!get_dir())
                    {
                        after = FALSE;
                        break;
                    }
                    delta.y += hero.y;
                    delta.x += hero.x;
                    if ( ((mp = monster_at(delta.x, delta.y)) == NULL)
                        || ((!see_monst(mp)) && !on(player, SEEMONST)))
                    {
                        if (!terse)
                            addmsg("I see ");
                        msg("no monster there");
                        after = FALSE;
                    }
                    else if (diag_ok(&hero, &delta))
                    {
                        to_death = TRUE;
                        max_hit = 0;
                        mp->flags |= ISTARGET;
                        runch = ch = dir_ch;
                        goto over;
                    }
                when 't':
                    if (!get_dir())
                        after = FALSE;
                    else
                        missile(delta.y, delta.x);
                when 'a':
                    if (last_comm == '\0')
                    {
                        msg("you haven't typed a command yet");
                        after = FALSE;
                    }
                    else
                    {
                        ch = last_comm;
                        again = TRUE;
                        goto over;
                    }
                when 'q': quaff();
                when 'Q':
                    after = FALSE;
                    quit(0);
                when 'i': after = FALSE; inventory(0);
                when 'I': after = FALSE; picky_inven();
                when 'd': drop();
                when 'r': read_scroll();
                when 'e': eat();
                when 'w': wield();
                when 'W': wear();
                when 'T': take_off();
                when 'P': ring_on();
                when 'R': ring_off();
                when 'c': call(); after = FALSE;
                when '>': after = FALSE; d_level();
                when '<': after = FALSE; u_level();
                when '?': after = FALSE; help();
                when '/': after = FALSE; identify();
                when 's': search();
                when 'z':
                    do_zap();
                when 'D': after = FALSE; discovered();
                when CTRL('P'): after = FALSE; msg(huh);
                when CTRL('R'):
                    after = FALSE;
                    refreshMap();
                when 'v':
                    after = FALSE;
                    msg("version %s", release);
                when 'M':
                    after = FALSE;
                    displayLargeMap();
                when 'S': 
                case K_EXIT:
                    after = FALSE;
                    save_game();
                when '.': ;                        /* Rest command */
                when ' ': after = FALSE;        /* "Legal" illegal command */
                when '^':
                    after = FALSE;
                    if (get_dir())
                    {
                        delta.y += hero.y;
                        delta.x += hero.x;
                        fp = &flags_at(delta.x, delta.y);
                        if (!terse)
                            addmsg("You have found ");
                        if (char_at(delta.x, delta.y) != TRAP)
                            msg("no trap there");
                        else if (on(player, ISHALU))
                            msg(tr_name[rnd(NTRAPS)]);
                        else {
                            msg(tr_name[*fp & F_TMASK]);
                            *fp |= F_SEEN;
                        }
                    }
                when ESCAPE:        /* Escape */
                    door_stop = FALSE;
                    count = 0;
                    after = FALSE;
                    again = FALSE;
                when 'm':
                    move_on = TRUE;
                    if (!get_dir())
                        after = FALSE;
                    else
                    {
                        ch = dir_ch;
                        countch = dir_ch;
                        goto over;
                    }
                when ')': current(cur_weapon, "wielding", NULL);
                when ']': current(cur_armor, "wearing", NULL);
                when '=':
                    current(cur_ring[LEFT], "wearing",
                                            terse ? "(L)" : "on left hand");
                    current(cur_ring[RIGHT], "wearing",
                                            terse ? "(R)" : "on right hand");
                when '@':
                    status(true);
                    after = FALSE;
                otherwise:
                    after = FALSE;
                    illcom(ch);
            }
            /*
             * turn off flags if no longer needed
             */
            if (!running)
                door_stop = FALSE;
        }
        /*
         * If he ran into something to take, let him pick it up.
         */
        if (take != 0)
            pick_up(take);
        if (!running)
            door_stop = FALSE;
        if (!after)
            ntimes++;
    }
    do_daemons(AFTER);
    do_fuses(AFTER);
    if (ISRING(LEFT, R_SEARCH))
        search();
    else if (ISRING(LEFT, R_TELEPORT) && rnd(50) == 0)
        teleport();
    if (ISRING(RIGHT, R_SEARCH))
        search();
    else if (ISRING(RIGHT, R_TELEPORT) && rnd(50) == 0)
        teleport();
}

/*
 * illcom:
 *        What to do with an illegal command
 */
void
illcom(int ch)
{
    save_msg = FALSE;
    count = 0;
    msg("illegal command '%s'", getKeyName(ch));
    save_msg = TRUE;
}

/*
 * search:
 *        player gropes about him to find hidden things.
 */
void search()
{
    register int y, x;
    int *fp;
    int ey, ex;
    int probinc;
    bool found;

    ey = hero.y + 1;
    ex = hero.x + 1;
    probinc = (on(player, ISHALU) ? 3 : 0);
    probinc += (on(player, ISBLIND) ? 2 : 0);
    found = FALSE;
    for (y = hero.y - 1; y <= ey; y++) 
        for (x = hero.x - 1; x <= ex; x++)
        {
            if (y == hero.y && x == hero.x)
                continue;
            fp = &flags_at(x, y);
            if (!(*fp & F_REAL))
            {
                switch (char_at(x, y))
                {
                    case WALL_V:
                    case WALL_H:
                    case '|':
                    case '-':
                        if (rnd(5 + probinc) != 0)
                            break;
                        char_at(x, y) = DOOR;
                        msg("a secret door");
foundone:
                        found = TRUE;
                        *fp |= F_REAL;
                        count = FALSE;
                        running = FALSE;
                        break;
                    case FLOOR:
                        if (rnd(2 + probinc) != 0)
                            break;
                        char_at(x, y) = TRAP;
                        if (!terse)
                            addmsg("you found ");
                        if (on(player, ISHALU))
                            msg(tr_name[rnd(NTRAPS)]);
                        else {
                            msg(tr_name[*fp & F_TMASK]);
                            *fp |= F_SEEN;
                        }
                        goto foundone;
                        break;
                    case ' ':
                        if (rnd(3 + probinc) != 0)
                            break;
                        char_at(x, y) = PASSAGE;
                        goto foundone;
                }
            }
        }
    if (found)
        look(FALSE);
}

/*
 * help:
 *        Give single character help, or the whole mess if he wants it
 */
void help()
{
    register struct h_list *strp;
    /*
    register char helpch;
    helpch = displayMessage("character you want help for (* for all):");
    // If its not a *, print the right help string or an error if he typed a funny character.
    if (helpch != '*')
    {
        for (strp = helpstr; strp->h_desc != NULL; strp++)
            if (strp->h_ch == helpch)
            {
                lower_msg = TRUE;
                msg("%s%s", getKeyName(strp->h_ch), strp->h_desc);
                lower_msg = FALSE;
                return;
            }
        msg("unknown character '%s'", getKeyName(helpch));
        return;
    }
    */
    startDisplayOfStringList();
    for (strp = helpstr; strp->h_desc != NULL; strp++)
        if (strp->h_print)
            displayStringListItem("%s%s", getKeyName(strp->h_ch), strp->h_desc);
    finishDisplayOfStringList();
    
    refreshMap();
}

/*
 * identify:
 *        Tell the player what a certain thing is.
 */
void
identify()
{
    int ch;
    struct h_list *hp;
    const char *str;
    
    static struct h_list ident_list[] = {
        {WALL_H,        "wall of a room",                FALSE},
        {WALL_V,        "wall of a room",                FALSE},
        {WALL_TL,       "wall of a room",                FALSE},
        {WALL_TR,       "wall of a room",                FALSE},
        {WALL_BL,       "wall of a room",                FALSE},
        {WALL_BR,       "wall of a room",                FALSE},
        {SOLID_WALL,    "wall",                          FALSE},
        {'|',           "wall of a room",                FALSE},
        {'-',           "wall of a room",                FALSE},
        {GOLD,          "gold",                          FALSE},
        {STAIRS,        "a staircase",                   FALSE},
        {DOOR,          "door",                          FALSE},
        {CLOSED_DOOR,   "closed door",                   FALSE},
        {FLOOR,         "room floor",                    FALSE},
        {PLAYER,        "you",                           FALSE},
        {PASSAGE_UNLIT, "passage",                       FALSE},
        {PASSAGE,       "passage",                       FALSE},
        {TRAP,          "trap",                          FALSE},
        {POTION,        "potion",                        FALSE},
        {SCROLL,        "scroll",                        FALSE},
        {FOOD,          "food",                          FALSE},
        {WEAPON,        "weapon",                        FALSE},
        {' ',           "solid rock",                    FALSE},
        {ARMOR,         "armor",                         FALSE},
        {AMULET,        "the Amulet of Yendor",          FALSE},
        {RING,          "ring",                          FALSE},
        {STICK,         "wand or staff",                 FALSE},
        {'\0'}
    };

    ch = displayMessage("what do you want identified?");
    if (ch == ESCAPE)
    {
        return;
    }
    if (isupper(ch))
        str = monsters[ch-'A'].m_name;
    else
    {
        str = "unknown character";
        for (hp = ident_list; hp->h_ch != '\0'; hp++)
            if (hp->h_ch == ch)
            {
                str = hp->h_desc;
                break;
            }
    }
    msg("'%s': %s", getKeyName(ch), str);
}

/*
 * d_level:
 *        He wants to go down a level
 */
void
d_level()
{
    if (levit_check())
        return;
    if (char_at(hero.x, hero.y) != STAIRS)
        msg("I see no way down");
    else
    {
        level++;
        seenstairs = FALSE;
        new_level();
        msg("You descend to level %d", level);
    }
}

/*
 * u_level:
 *        He wants to go up a level
 */
void
u_level()
{
    if (levit_check())
        return;
    if (char_at(hero.x, hero.y) == STAIRS)
    {
        if (amulet)
        {
            level--;
            if (level == 0)
                total_winner();
            new_level();
            msg("you feel a wrenching sensation in your gut");
        }
        else
        {
            msg("your way is magically blocked");
        }
    }
    else
    {
        msg("I see no way up");
    }
}

/*
 * levit_check:
 *        Check to see if she's levitating, and if she is, print an
 *        appropriate message.
 */
bool
levit_check()
{
    if (!on(player, ISLEVIT))
        return FALSE;
    msg("You can't.  You're floating off the ground!");
    return TRUE;
}

/*
 * call:
 *        Allow a user to call a potion, scroll, or ring something
 */
void
call()
{
    ItemThing *obj;
    struct obj_info *op = NULL;
    char **guess;
    const char* elsewise = NULL;
    register bool *know;

    obj = get_item("call", CALLABLE);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (obj == NULL)
        return;
    switch (obj->type)
    {
        case RING:
            op = &ring_info[obj->which];
            elsewise = r_stones[obj->which];
            goto norm;
        when POTION:
            op = &pot_info[obj->which];
            elsewise = p_colors[obj->which];
            goto norm;
        when SCROLL:
            op = &scr_info[obj->which];
            elsewise = s_names[obj->which];
            goto norm;
        when STICK:
            op = &ws_info[obj->which];
            elsewise = ws_made[obj->which];
norm:
            know = &op->oi_know;
            guess = &op->oi_guess;
            if (*guess != NULL)
                elsewise = *guess;
        when FOOD:
            msg("you can't call that anything");
            return;
        otherwise:
            guess = &obj->label;
            know = NULL;
            elsewise = obj->label;
    }
    if (know != NULL && *know)
    {
        msg("that has already been identified");
        return;
    }
    if (elsewise != NULL && elsewise == *guess)
    {
        if (!terse)
            addmsg("Was ");
        msg("called \"%s\"", elsewise);
    }

    if (elsewise == NULL)
        strcpy(prbuf, "");
    else
        strcpy(prbuf, elsewise);

    if (askForInput(terse ? "call it:" : "what do you want to call it?", prbuf, MAXSTR) == NORM)
    {
        if (*guess != NULL)
            free(*guess);
        *guess = (char*)malloc((unsigned int) strlen(prbuf) + 1);
        strcpy(*guess, prbuf);
    }
}

/*
 * current:
 *        Print the current weapon/armor
 */
void current(ItemThing *cur, const char *how, const char *where)
{
    after = FALSE;
    if (cur != NULL)
    {
        if (!terse)
            addmsg("you are %s (", how);
        inv_describe = FALSE;
        addmsg("%c) %s", cur->packch, inv_name(cur, TRUE));
        inv_describe = TRUE;
        if (where)
            addmsg(" %s", where);
        endmsg();
    }
    else
    {
        if (!terse)
            addmsg("you are ");
        addmsg("%s nothing", how);
        if (where)
            addmsg(" %s", where);
        endmsg();
    }
}
