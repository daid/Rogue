/*
 * Various input/output functions
 *
 * @(#)io.c        4.32 (Berkeley) 02/05/99
 */

#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "rogue.h"

/*
 * msg:
 *        Display a message at the top of the screen.
 */
#define MAXMSG        (NUMCOLS - sizeof "--More--")

static char msgbuf[2*MAXMSG+1];
static int newpos = 0;

/* VARARGS1 */
int msg(const char *fmt, ...)
{
    va_list args;

    /*
     * if the string is "", just clear the line
     */
    if (*fmt == '\0')
    {
        return ~ESCAPE;
    }
    /*
     * otherwise add to the message and flush it out
     */
    va_start(args, fmt);
    doadd(fmt, args);
    va_end(args);
    return endmsg();
}

/*
 * addmsg:
 *        Add things to the current message
 */
/* VARARGS1 */
void addmsg(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    doadd(fmt, args);
    va_end(args);
}

/*
 * endmsg:
 *        Display a new msg (giving him a chance to see the previous one
 *        if it is up there with the --More--)
 */
int
endmsg()
{
    int ch = 0;

    if (save_msg)
        strcpy(huh, msgbuf);
    /*
     * All messages should start with uppercase, except ones that
     * start with a pack addressing character
     */
    if (islower(msgbuf[0]) && !lower_msg && msgbuf[1] != ')')
        msgbuf[0] = (char) toupper(msgbuf[0]);
    
    if (strlen(msgbuf) > 0)
        ch = displayMessage(msgbuf);

    newpos = 0;
    msgbuf[0] = '\0';
    return ch;
}

/*
 * doadd:
 *        Perform an add onto the message buffer
 */
void
doadd(const char *fmt, va_list args)
{
    static char buf[MAXSTR];

    /*
     * Do the printf into buf
     */
    vsprintf(buf, fmt, args);
    if (strlen(buf) + newpos >= MAXMSG)
        endmsg(); 
    strcat(msgbuf, buf);
    newpos = (int) strlen(msgbuf);
}

/*
 * step_ok:
 *        Returns true if it is ok to step on ch
 */
int
step_ok(int ch)
{
    switch (ch)
    {
        case ' ':
        case '|':
        case '-':
        case WALL_H:
        case WALL_V:
        case WALL_TL:
        case WALL_TR:
        case WALL_BL:
        case WALL_BR:
            return FALSE;
        default:
            return (!isalpha(ch));
    }
}

/*
 * status:
 *        Display the important stats line.  Keep the cursor where it was.
 */
void status()
{
    int temp;
    static int hpwidth = 0;
    static int s_hungry = 0;
    static int s_lvl = 0;
    static int s_pur = -1;
    static int s_hp = 0;
    static int s_arm = 0;
    static str_t s_str = 0;
    static int s_exp = 0;
    static const char *state_name[] =
    {
        "", "Hungry", "Weak", "Faint"
    };

    /*
     * If nothing has changed since the last status, don't
     * bother.
     */
    temp = (cur_armor != NULL ? cur_armor->o_arm : pstats.s_arm);
    if (s_hp == pstats.s_hpt && s_exp == pstats.s_exp && s_pur == purse
        && s_arm == temp && s_str == pstats.s_str && s_lvl == level
        && s_hungry == hungry_state
        && !stat_msg
        )
            return;

    s_arm = temp;

    if (s_hp != max_hp)
    {
        temp = max_hp;
        s_hp = max_hp;
        for (hpwidth = 0; temp; hpwidth++)
            temp /= 10;
    }

    /*
     * Save current status
     */
    s_lvl = level;
    s_pur = purse;
    s_hp = pstats.s_hpt;
    s_str = pstats.s_str;
    s_exp = pstats.s_exp; 
    s_hungry = hungry_state;

    if (stat_msg)
    {
        msg("Level: %d Gold: %-5d Hp: %*d(%*d) Str: %2d(%d) Arm: %-2d Exp: %d/%ld %s",
            level, purse, hpwidth, pstats.s_hpt, hpwidth, max_hp, pstats.s_str,
            max_stats.s_str, 10 - s_arm, pstats.s_lvl, pstats.s_exp,
            state_name[hungry_state]);
    }
    else
    {
        //12345678901234567890123456789012
        //Lvl:xx Gold:xxxxx Hp: xx/xx Str: xx/xx, Ac: xx, XP: xx,xx [hungry]
        //Hp: xxx/xxx Ac: xx XP: 8000000,20
        //Hp: xxx/xxx Ac: xx XP: 800k,20
        //Hp: xxx/xxx Ac: xx [HUNGRY]
        char status_line[40];
        char* c = status_line + sprintf(status_line, "Hp: %3d/%3d Ac: %2d ", pstats.s_hpt, max_hp, 10 - s_arm);
        if (hungry_state == 0)
        {
            if (pstats.s_exp < 2000)
                sprintf(c, "Xp: %3d/%2d", pstats.s_exp, pstats.s_lvl);
            else
                sprintf(c, "Xp: %3dk/%2d", pstats.s_exp / 1000, pstats.s_lvl);
        }
        else
        {
            sprintf(c, "[%s]", state_name[hungry_state]);
        }
        setStatusLine(status_line);
    }
}
