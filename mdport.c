/*
    mdport.c - Machine Dependent Code for Porting Unix/Curses games

    Copyright (C) 2005 Nicholas J. Kisseberth
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>

#include "extern.h"

void md_init()
{
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);
    SDL_SetVideoMode(900, 480, 32, SDL_SWSURFACE);
}

int
md_unlink(char *file)
{
    return(unlink(file));
}

int md_unlink_open_file(char *file, FILE *inf)
{
#ifdef WIN32
    fclose(inf);
#endif
    return unlink(file);
}

int
md_chmod(char *filename, int mode)
{
    return 0;
}

int
md_getuid()
{
    return 42;
}

int
md_getpid()
{
    return 42;
}

char *
md_getusername()
{
    return "ultimaker";
}

char *
md_gethomedir()
{
    return "";
}

char *
md_getrealname(int uid)
{
    static char uidstr[20];
#if !defined(_WIN32) && !defined(DJGPP)
    struct passwd *pp;

        if ((pp = getpwuid(uid)) == NULL)
    {
        sprintf(uidstr,"%d", uid);
        return(uidstr);
    }
        else
            return(pp->pw_name);
#else
   sprintf(uidstr,"%d", uid);
   return(uidstr);
#endif
}

extern char *xcrypt(char *key, char *salt);

char *
md_crypt(char *key, char *salt)
{
    return( xcrypt(key,salt) );
}

char *
md_getpass(char *prompt)
{
#ifndef HAVE_GETPASS
    static char password_buffer[9];
    char *p = password_buffer;
    int c, count = 0;
    int max_length = 9;

    fflush(stdout);
    /* If we can't prompt, abort */
    if (fputs(prompt, stderr) < 0)
    {
        *p = '\0';
        return NULL;
    }

    for(;;)
    {
        /* Get a character with no echo */
        c = md_readchar();

        /* Exit on interrupt (^c or ^break) */
        if (c == '\003' || c == 0x100)
            exit(1);

        /* Terminate on end of line or file (^j, ^m, ^d, ^z) */
        if (c == '\r' || c == '\n' || c == '\004' || c == '\032')
            break;

        /* Back up on backspace */
        if (c == '\b')
        {
            if (count)
                count--;
            else if (p > password_buffer)
                p--;
            continue;
        }

        /* Ignore DOS extended characters */
        if ((c & 0xff) != c)
            continue;

        /* Add to password if it isn't full */
        if (p < password_buffer + max_length - 1)
            *p++ = (char) c;
        else
            count++;
    }
   *p = '\0';

   fputc('\n', stderr);

   return password_buffer;
#else
   return( (char *) getpass(prompt) );
#endif
}

int
md_erasechar()
{
    return 0;
}

int
md_killchar()
{
    return 0;
}

int
md_dsuspchar()
{
    return 0;
}

int
md_setdsuspchar(int c)
{
    return(0);
}

int
md_suspchar()
{
    return(0);
}

int
md_setsuspchar(int c)
{
    return(0);
}

/*
    Cursor/Keypad Support

    Sadly Cursor/Keypad support is less straightforward than it should be.
    
    The various terminal emulators/consoles choose to differentiate the 
    cursor and keypad keys (with modifiers) in different ways (if at all!). 
    Furthermore they use different code set sequences for each key only
    a subset of which the various curses libraries recognize. Partly due
    to incomplete termcap/terminfo entries and partly due to inherent 
    limitations of those terminal capability databases.

    I give curses first crack at decoding the sequences. If it fails to decode
    it we check for common ESC-prefixed sequences.

    All cursor/keypad results are translated into standard rogue movement 
    commands.

    Unmodified keys are translated to walk commands: hjklyubn
    Modified (shift,control,alt) are translated to run commands: HJKLYUBN

    Console and supported (differentiated) keys
    Interix:  Cursor Keys, Keypad, Ctl-Keypad
    Cygwin:   Cursor Keys, Keypad, Alt-Cursor Keys
    MSYS:     Cursor Keys, Keypad, Ctl-Cursor Keys, Ctl-Keypad
    Win32:    Cursor Keys, Keypad, Ctl/Shift/Alt-Cursor Keys, Ctl/Alt-Keypad
    DJGPP:    Cursor Keys, Keypad, Ctl/Shift/Alt-Cursor Keys, Ctl/Alt-Keypad

    Interix Console (raw, ncurses)
    ==============================
    normal        shift                ctrl            alt
    ESC [D,        ESC F^,                ESC [D,            ESC [D            /# Left            #/
    ESC [C,        ESC F$,                ESC [C,            ESC [C            /# Right            #/
    ESC [A,        ESC F-,                local win,  ESC [A            /# Up            #/
    ESC [B,        ESC F+,                local win,  ESC [B            /# Down            #/
    ESC [H,        ESC [H,                ESC [H,            ESC [H            /# Home            #/
    ESC [S,        local win,        ESC [S,            ESC [S            /# Page Up            #/
    ESC [T,        local win,        ESC [T,            ESC [T            /# Page Down    #/
    ESC [U,        ESC [U,                ESC [U,            ESC [U            /# End            #/
    ESC [D,        ESC F^,                ESC [D,            O                    /# Keypad Left  #/
    ESC [C,        ESC F$,                ESC [C,            O                    /# Keypad Right #/
    ESC [A,        ESC [A,                ESC [-1,    O                    /# Keypad Up    #/
    ESC [B,        ESC [B,                ESC [-2,    O                    /# Keypad Down  #/
    ESC [H,        ESC [H,                ESC [-263,  O                    /# Keypad Home  #/
    ESC [S,        ESC [S,                ESC [-19,   O                    /# Keypad PgUp  #/
    ESC [T,        ESC [T,                ESC [-20,   O                    /# Keypad PgDn  #/
    ESC [U,        ESC [U,                ESC [-21,   O                    /# Keypad End   #/
    nothing,        nothing,        nothing,    O                    /# Kaypad 5     #/

    Interix Console (term=interix, ncurses)
    ==============================
    KEY_LEFT,        ESC F^,                KEY_LEFT,   KEY_LEFT            /# Left            #/
    KEY_RIGHT,        ESC F$,                KEY_RIGHT,  KEY_RIGHT            /# Right            #/
    KEY_UP,        0x146,                local win,  KEY_UP            /# Up            #/
    KEY_DOWN,        0x145,                local win,  KEY_DOWN            /# Down            #/
    ESC [H,        ESC [H,                ESC [H,            ESC [H            /# Home            #/
    KEY_PPAGE,        local win,        KEY_PPAGE,  KEY_PPAGE            /# Page Up            #/
    KEY_NPAGE,        local win,        KEY_NPAGE,  KEY_NPAGE            /# Page Down    #/
    KEY_LL,        KEY_LL,                KEY_LL,            KEY_LL            /# End            #/
    KEY_LEFT,        ESC F^,                ESC [-4,    O                    /# Keypad Left  #/
    KEY_RIGHT,        ESC F$,                ESC [-3,    O                    /# Keypad Right #/
    KEY_UP,        KEY_UP,                ESC [-1,    O                    /# Keypad Up    #/
    KEY_DOWN,        KEY_DOWN,        ESC [-2,    O                    /# Keypad Down  #/
    ESC [H,        ESC [H,                ESC [-263,  O                    /# Keypad Home  #/
    KEY_PPAGE,        KEY_PPAGE,        ESC [-19,   O                    /# Keypad PgUp  #/
    KEY_NPAGE,        KEY_NPAGE,        ESC [-20,   O                    /# Keypad PgDn  #/
    KEY_LL,        KEY_LL,                ESC [-21,   O                    /# Keypad End   #/
    nothing,        nothing,        nothing,    O                    /# Keypad 5     #/

    Cygwin Console (raw, ncurses)
    ==============================
    normal        shift                ctrl            alt
    ESC [D,        ESC [D,                ESC [D,            ESC ESC [D            /# Left            #/
    ESC [C,        ESC [C,                ESC [C,            ESC ESC [C            /# Rght            #/
    ESC [A,        ESC [A,                ESC [A,            ESC ESC [A            /# Up            #/
    ESC [B,        ESC [B,                ESC [B,            ESC ESC [B            /# Down            #/
    ESC [1~,        ESC [1~,        ESC [1~,    ESC ESC [1~            /# Home            #/
    ESC [5~,        ESC [5~,        ESC [5~,    ESC ESC [5~            /# Page Up            #/
    ESC [6~,        ESC [6~,        ESC [6~,    ESC ESC [6~            /# Page Down    #/
    ESC [4~,        ESC [4~,        ESC [4~,    ESC ESC [4~            /# End            #/
    ESC [D,        ESC [D,                ESC [D,            ESC ESC [D,O    /# Keypad Left  #/
    ESC [C,        ESC [C,                ESC [C,            ESC ESC [C,O    /# Keypad Right #/
    ESC [A,        ESC [A,                ESC [A,            ESC ESC [A,O    /# Keypad Up    #/
    ESC [B,        ESC [B,                ESC [B,            ESC ESC [B,O    /# Keypad Down  #/
    ESC [1~,        ESC [1~,        ESC [1~,    ESC ESC [1~,O   /# Keypad Home  #/
    ESC [5~,        ESC [5~,        ESC [5~,    ESC ESC [5~,O   /# Keypad PgUp  #/
    ESC [6~,        ESC [6~,        ESC [6~,    ESC ESC [6~,O   /# Keypad PgDn  #/
    ESC [4~,        ESC [4~,        ESC [4~,    ESC ESC [4~,O   /# Keypad End   #/
    ESC [-71,        nothing,        nothing,    O                    /# Keypad 5            #/

    Cygwin Console (term=cygwin, ncurses)
    ==============================
    KEY_LEFT,        KEY_LEFT,        KEY_LEFT,   ESC-260            /# Left            #/
    KEY_RIGHT,        KEY_RIGHT,        KEY_RIGHT,  ESC-261            /# Rght            #/
    KEY_UP,        KEY_UP,                KEY_UP,            ESC-259            /# Up            #/
    KEY_DOWN,        KEY_DOWN,        KEY_DOWN,   ESC-258            /# Down            #/
    KEY_HOME,        KEY_HOME,        KEY_HOME,   ESC-262            /# Home            #/
    KEY_PPAGE,        KEY_PPAGE,        KEY_PPAGE,  ESC-339            /# Page Up            #/
    KEY_NPAGE,        KEY_NPAGE,        KEY_NPAGE,  ESC-338            /# Page Down    #/
    KEY_END,        KEY_END,        KEY_END,    ESC-360            /# End            #/
    KEY_LEFT,        KEY_LEFT,        KEY_LEFT,   ESC-260,O            /# Keypad Left  #/
    KEY_RIGHT,        KEY_RIGHT,        KEY_RIGHT,  ESC-261,O            /# Keypad Right #/
    KEY_UP,        KEY_UP,                KEY_UP,            ESC-259,O       /# Keypad Up    #/
    KEY_DOWN,        KEY_DOWN,        KEY_DOWN,   ESC-258,O       /# Keypad Down  #/
    KEY_HOME,        KEY_HOME,        KEY_HOME,   ESC-262,O       /# Keypad Home  #/
    KEY_PPAGE,        KEY_PPAGE,        KEY_PPAGE,  ESC-339,O            /# Keypad PgUp  #/
    KEY_NPAGE,        KEY_NPAGE,        KEY_NPAGE,  ESC-338,O            /# Keypad PgDn  #/
    KEY_END,        KEY_END,        KEY_END,    ESC-360,O       /# Keypad End   #/
    ESC [G,        nothing,        nothing,    O                    /# Keypad 5            #/

    MSYS Console (raw, ncurses)
    ==============================
    normal        shift                ctrl            alt
    ESC OD,        ESC [d,                ESC Od            nothing            /# Left            #/
    ESC OE,        ESC [e,                ESC Oe,            nothing            /# Right            #/
    ESC OA,        ESC [a,                ESC Oa,            nothing            /# Up            #/
    ESC OB,        ESC [b,                ESC Ob,            nothing            /# Down            #/
    ESC [7~,        ESC [7$,        ESC [7^,    nothing            /# Home            #/
    ESC [5~,        local window,   ESC [5^,    nothing            /# Page Up      #/
    ESC [6~,        local window,   ESC [6^,    nothing            /# Page Down    #/
    ESC [8~,        ESC [8$,        ESC [8^,    nothing            /# End            #/
    ESC OD,        ESC [d,                ESC Od            O                    /# Keypad Left  #/
    ESC OE,        ESC [c,                ESC Oc,            O                    /# Keypad Right #/
    ESC OA,        ESC [a,                ESC Oa,            O                    /# Keypad Up    #/
    ESC OB,        ESC [b,                ESC Ob,            O                    /# Keypad Down  #/
    ESC [7~,        ESC [7$,        ESC [7^,    O                    /# Keypad Home  #/
    ESC [5~,        local window,   ESC [5^,    O                    /# Keypad PgUp  #/
    ESC [6~,        local window,   ESC [6^,    O                    /# Keypad PgDn  #/
    ESC [8~,        ESC [8$,        ESC [8^,    O                    /# Keypad End   #/
    11,                11,                11,            O                    /# Keypad 5     #/

    MSYS Console (term=rxvt, ncurses)
    ==============================
    normal        shift                ctrl            alt
    KEY_LEFT,        KEY_SLEFT,        514            nothing            /# Left            #/
    KEY_RIGHT,        KEY_SRIGHT,        516,            nothing            /# Right            #/
    KEY_UP,        518,                519,            nothing            /# Up            #/
    KEY_DOWN,        511,                512,            nothing            /# Down            #/
    KEY_HOME,        KEY_SHOME,        ESC [7^,    nothing            /# Home            #/
    KEY_PPAGE,        local window,   ESC [5^,    nothing            /# Page Up      #/
    KEY_NPAGE,        local window,   ESC [6^,    nothing            /# Page Down    #/
    KEY_END,        KEY_SEND,        KEY_EOL,    nothing            /# End            #/
    KEY_LEFT,        KEY_SLEFT,        514            O                    /# Keypad Left  #/
    KEY_RIGHT,        KEY_SRIGHT,        516,            O                    /# Keypad Right #/
    KEY_UP,        518,                519,            O                    /# Keypad Up    #/
    KEY_DOWN,        511,                512,            O                    /# Keypad Down  #/
    KEY_HOME,        KEY_SHOME,        ESC [7^,    O                    /# Keypad Home  #/
    KEY_PPAGE,        local window,   ESC [5^,    O                    /# Keypad PgUp  #/
    KEY_NPAGE,        local window,   ESC [6^,    O                    /# Keypad PgDn  #/
    KEY_END,        KEY_SEND,        KEY_EOL,    O                    /# Keypad End   #/
    11,                11,                11,            O                    /# Keypad 5     #/

    Win32 Console (raw, pdcurses)
    DJGPP Console (raw, pdcurses)
    ==============================
    normal        shift                ctrl            alt
    260,        391,                443,            493                    /# Left            #/
    261,        400,                444,            492                    /# Right            #/
    259,        547,                480,            490                    /# Up            #/
    258,        548,                481,            491                    /# Down            #/
    262,        388,                447,            524                        /# Home            #/
    339,        396,                445,            526                        /# Page Up            #/
    338,        394,                446,            520                    /# Page Down    #/
    358,        384,                448,            518                     /# End            #/
    452,        52('4'),        511,            521                    /# Keypad Left  #/
    454,        54('6'),        513,            523                    /# Keypad Right #/
    450,        56('8'),        515,            525                    /# Keypad Up    #/
    456,        50('2'),        509,            519                    /# Keypad Down  #/
    449,        55('7'),        514,            524                    /# Keypad Home  #/
    451,        57('9'),        516,            526                    /# Keypad PgUp  #/
    457,        51('3'),        510,            520                    /# Keypad PgDn  #/
    455,        49('1'),        508,            518                    /# Keypad End   #/
    453,        53('5'),        512,            522                    /# Keypad 5     #/

    Win32 Console (pdcurses, MSVC/MingW32)
    DJGPP Console (pdcurses)
    ==============================
    normal        shift                ctrl            alt
    KEY_LEFT,        KEY_SLEFT,        CTL_LEFT,   ALT_LEFT            /# Left            #/
    KEY_RIGHT,        KEY_SRIGHT,        CTL_RIGHT,  ALT_RIGHT            /# Right            #/
    KEY_UP,        KEY_SUP,        CTL_UP,            ALT_UP            /# Up            #/
    KEY_DOWN,        KEY_SDOWN,        CTL_DOWN,   ALT_DOWN            /# Down            #/
    KEY_HOME,        KEY_SHOME,        CTL_HOME,   ALT_HOME            /# Home            #/
    KEY_PPAGE,        KEY_SPREVIOUS,  CTL_PGUP,   ALT_PGUP            /# Page Up      #/
    KEY_NPAGE,        KEY_SNEXTE,        CTL_PGDN,   ALT_PGDN            /# Page Down    #/
    KEY_END,        KEY_SEND,        CTL_END,    ALT_END            /# End            #/
    KEY_B1,        52('4'),        CTL_PAD4,   ALT_PAD4            /# Keypad Left  #/
    KEY_B3,        54('6'),        CTL_PAD6,   ALT_PAD6            /# Keypad Right #/
    KEY_A2,        56('8'),        CTL_PAD8,   ALT_PAD8            /# Keypad Up    #/
    KEY_C2,        50('2'),        CTL_PAD2,   ALT_PAD2            /# Keypad Down  #/
    KEY_A1,        55('7'),        CTL_PAD7,   ALT_PAD7            /# Keypad Home  #/
    KEY_A3,        57('9'),        CTL_PAD9,   ALT_PAD9            /# Keypad PgUp  #/
    KEY_C3,        51('3'),        CTL_PAD3,   ALT_PAD3            /# Keypad PgDn  #/
    KEY_C1,        49('1'),        CTL_PAD1,   ALT_PAD1            /# Keypad End   #/
    KEY_B2,        53('5'),        CTL_PAD5,   ALT_PAD5            /# Keypad 5     #/

    Windows Telnet (raw)
    ==============================
    normal        shift                ctrl            alt
    ESC [D,        ESC [D,                ESC [D,            ESC [D            /# Left            #/
    ESC [C,        ESC [C,                ESC [C,            ESC [C            /# Right            #/
    ESC [A,        ESC [A,                ESC [A,            ESC [A            /# Up            #/
    ESC [B,        ESC [B,                ESC [B,            ESC [B            /# Down            #/
    ESC [1~,        ESC [1~,        ESC [1~,    ESC [1~            /# Home            #/
    ESC [5~,        ESC [5~,        ESC [5~,    ESC [5~            /# Page Up            #/
    ESC [6~,        ESC [6~,        ESC [6~,    ESC [6~            /# Page Down    #/
    ESC [4~,        ESC [4~,        ESC [4~,    ESC [4~            /# End            #/
    ESC [D,        ESC [D,                ESC [D,            ESC [D            /# Keypad Left  #/
    ESC [C,        ESC [C,                ESC [C,            ESC [C            /# Keypad Right #/
    ESC [A,        ESC [A,                ESC [A,            ESC [A            /# Keypad Up    #/
    ESC [B,        ESC [B,                ESC [B,            ESC [B            /# Keypad Down  #/
    ESC [1~,        ESC [1~,        ESC [1~,    ESC [1~            /# Keypad Home  #/
    ESC [5~,        ESC [5~,        ESC [5~,    ESC [5~            /# Keypad PgUp  #/
    ESC [6~,        ESC [6~,        ESC [6~,    ESC [6~            /# Keypad PgDn  #/
    ESC [4~,        ESC [4~,        ESC [4~,    ESC [4~            /# Keypad End   #/
    nothing,        nothing,        nothing,    nothing            /# Keypad 5     #/

    Windows Telnet (term=xterm)
    ==============================
    normal        shift                ctrl            alt
    KEY_LEFT,        KEY_LEFT,        KEY_LEFT,   KEY_LEFT            /# Left            #/
    KEY_RIGHT,        KEY_RIGHT,        KEY_RIGHT,  KEY_RIGHT            /# Right            #/
    KEY_UP,        KEY_UP,                KEY_UP,            KEY_UP            /# Up            #/
    KEY_DOWN,        KEY_DOWN,        KEY_DOWN,   KEY_DOWN            /# Down            #/
    ESC [1~,        ESC [1~,        ESC [1~,    ESC [1~            /# Home            #/
    KEY_PPAGE,        KEY_PPAGE,        KEY_PPAGE,  KEY_PPAGE            /# Page Up            #/
    KEY_NPAGE,        KEY_NPAGE,        KEY_NPAGE,  KEY_NPAGE            /# Page Down    #/
    ESC [4~,        ESC [4~,        ESC [4~,    ESC [4~            /# End            #/
    KEY_LEFT,        KEY_LEFT,        KEY_LEFT,   O                    /# Keypad Left  #/
    KEY_RIGHT,        KEY_RIGHT,        KEY_RIGHT,  O                    /# Keypad Right #/
    KEY_UP,        KEY_UP,                KEY_UP,            O                    /# Keypad Up    #/
    KEY_DOWN,        KEY_DOWN,        KEY_DOWN,   O                    /# Keypad Down  #/
    ESC [1~,        ESC [1~,        ESC [1~,    ESC [1~            /# Keypad Home  #/
    KEY_PPAGE,        KEY_PPAGE,        KEY_PPAGE,  KEY_PPAGE            /# Keypad PgUp  #/
    KEY_NPAGE,        KEY_NPAGE,        KEY_NPAGE,  KEY_NPAGE            /# Keypad PgDn  #/
    ESC [4~,        ESC [4~,        ESC [4~,    O                    /# Keypad End   #/
    ESC [-71,        nothing,        nothing,    O                    /# Keypad 5            #/

    PuTTY
    ==============================
    normal        shift                ctrl            alt
    ESC [D,        ESC [D,                ESC OD,            ESC [D            /# Left            #/
    ESC [C,        ESC [C,                ESC OC,            ESC [C            /# Right            #/
    ESC [A,        ESC [A,                ESC OA,            ESC [A            /# Up            #/
    ESC [B,        ESC [B,                ESC OB,            ESC [B            /# Down            #/
    ESC [1~,        ESC [1~,        local win,  ESC [1~            /# Home            #/
    ESC [5~,        local win,        local win,  ESC [5~            /# Page Up            #/
    ESC [6~,        local win,        local win,  ESC [6~            /# Page Down    #/
    ESC [4~,        ESC [4~,        local win,  ESC [4~            /# End            #/
    ESC [D,        ESC [D,                ESC [D,            O                    /# Keypad Left  #/
    ESC [C,        ESC [C,                ESC [C,            O                    /# Keypad Right #/
    ESC [A,        ESC [A,                ESC [A,            O                    /# Keypad Up    #/
    ESC [B,        ESC [B,                ESC [B,            O                    /# Keypad Down  #/
    ESC [1~,        ESC [1~,        ESC [1~,    O                    /# Keypad Home  #/
    ESC [5~,        ESC [5~,        ESC [5~,    O                    /# Keypad PgUp  #/
    ESC [6~,        ESC [6~,        ESC [6~,    O                    /# Keypad PgDn  #/
    ESC [4~,        ESC [4~,        ESC [4~,    O                    /# Keypad End   #/
    nothing,        nothing,        nothing,    O                    /# Keypad 5            #/

    PuTTY
    ==============================
    normal        shift                ctrl            alt
    KEY_LEFT,        KEY_LEFT,        ESC OD,            ESC KEY_LEFT    /# Left            #/
    KEY_RIGHT        KEY_RIGHT,        ESC OC,            ESC KEY_RIGHT   /# Right            #/
    KEY_UP,        KEY_UP,                ESC OA,            ESC KEY_UP            /# Up            #/
    KEY_DOWN,        KEY_DOWN,        ESC OB,            ESC KEY_DOWN    /# Down            #/
    ESC [1~,        ESC [1~,        local win,  ESC ESC [1~            /# Home            #/
    KEY_PPAGE        local win,        local win,  ESC KEY_PPAGE   /# Page Up            #/
    KEY_NPAGE        local win,        local win,  ESC KEY_NPAGE   /# Page Down    #/
    ESC [4~,        ESC [4~,        local win,  ESC ESC [4~            /# End            #/
    ESC Ot,        ESC Ot,                ESC Ot,            O                    /# Keypad Left  #/
    ESC Ov,        ESC Ov,                ESC Ov,            O                    /# Keypad Right #/
    ESC Ox,        ESC Ox,                ESC Ox,            O                    /# Keypad Up    #/
    ESC Or,        ESC Or,                ESC Or,            O                    /# Keypad Down  #/
    ESC Ow,        ESC Ow,                ESC Ow,     O                    /# Keypad Home  #/
    ESC Oy,        ESC Oy,                ESC Oy,     O                    /# Keypad PgUp  #/
    ESC Os,        ESC Os,                ESC Os,     O                    /# Keypad PgDn  #/
    ESC Oq,        ESC Oq,                ESC Oq,     O                    /# Keypad End   #/
    ESC Ou,        ESC Ou,                ESC Ou,            O                    /# Keypad 5            #/
*/

#define M_NORMAL 0
#define M_ESC    1
#define M_KEYPAD 2
#define M_TRAIL  3

int md_readchar()
{
#if 0
    int ch = 0;
    int lastch = 0;
    int mode = M_NORMAL;
    int mode2 = M_NORMAL;

    for(;;)
    {
        ch = getch();

        if (ch == ERR)            /* timed out waiting for valid sequence */
        {                    /* flush input so far and start over    */
            mode = M_NORMAL;
            nocbreak();
            raw();
            ch = 27;
            break;
        }

        if (mode == M_TRAIL)
        {
            if (ch == '^')                /* msys console  : 7,5,6,8: modified*/
                ch = CTRL( toupper(lastch) );

            if (ch == '~')                /* cygwin console: 1,5,6,4: normal  */
                ch = tolower(lastch);   /* windows telnet: 1,5,6,4: normal  */
                                        /* msys console  : 7,5,6,8: normal  */

            if (mode2 == M_ESC)                /* cygwin console: 1,5,6,4: modified*/
                ch = CTRL( toupper(ch) );

            break;
        }

        if (mode == M_ESC) 
        {
            if (ch == 27)
            {
                mode2 = M_ESC;
                continue;
            }

            if ((ch == 'F') || (ch == 'O') || (ch == '['))
            {
                mode = M_KEYPAD;
                continue;
            }


            switch(ch)
            {
                /* Cygwin Console   */
                /* PuTTY            */
                case KEY_LEFT : ch = CTRL('H'); break;
                case KEY_RIGHT: ch = CTRL('L'); break;
                case KEY_UP   : ch = CTRL('K'); break;
                case KEY_DOWN : ch = CTRL('J'); break;
                case KEY_HOME : ch = CTRL('Y'); break;
                case KEY_PPAGE: ch = CTRL('U'); break;
                case KEY_NPAGE: ch = CTRL('N'); break;
                case KEY_END  : ch = CTRL('B'); break;

                default: break;
            }

            break;
        }

        if (mode == M_KEYPAD)
        {
            switch(ch)
            {
                /* ESC F - Interix Console codes */
                case   '^': ch = CTRL('H'); break;        /* Shift-Left            */
                case   '$': ch = CTRL('L'); break;        /* Shift-Right            */

                /* ESC [ - Interix Console codes */
                case   'H': ch = 'y'; break;                /* Home                    */
                case     1: ch = CTRL('K'); break;        /* Ctl-Keypad Up    */
                case     2: ch = CTRL('J'); break;        /* Ctl-Keypad Down  */
                case     3: ch = CTRL('L'); break;        /* Ctl-Keypad Right */
                case     4: ch = CTRL('H'); break;        /* Ctl-Keypad Left  */
                case   263: ch = CTRL('Y'); break;        /* Ctl-Keypad Home  */
                case    19: ch = CTRL('U'); break;        /* Ctl-Keypad PgUp  */
                case    20: ch = CTRL('N'); break;        /* Ctl-Keypad PgDn  */
                case    21: ch = CTRL('B'); break;        /* Ctl-Keypad End   */

                /* ESC [ - Cygwin Console codes */
                case   'G': ch = '.'; break;                /* Keypad 5            */
                case   '7': lastch = 'Y'; mode=M_TRAIL; break;        /* Ctl-Home */
                case   '5': lastch = 'U'; mode=M_TRAIL; break;        /* Ctl-PgUp */
                case   '6': lastch = 'N'; mode=M_TRAIL; break;        /* Ctl-PgDn */

                /* ESC [ - Win32 Telnet, PuTTY */
                case   '1': lastch = 'y'; mode=M_TRAIL; break;        /* Home            */
                case   '4': lastch = 'b'; mode=M_TRAIL; break;        /* End            */

                /* ESC O - PuTTY */
                case   'D': ch = CTRL('H'); break;
                case   'C': ch = CTRL('L'); break;
                case   'A': ch = CTRL('K'); break;
                case   'B': ch = CTRL('J'); break;
                case   't': ch = 'h'; break;
                case   'v': ch = 'l'; break;
                case   'x': ch = 'k'; break;
                case   'r': ch = 'j'; break;
                case   'w': ch = 'y'; break;
                case   'y': ch = 'u'; break;
                case   's': ch = 'n'; break;
                case   'q': ch = 'b'; break;
                case   'u': ch = '.'; break;
            }

            if (mode != M_KEYPAD)
                continue;
        }

        if (ch == 27)
        {
            halfdelay(1);
            mode = M_ESC;
            continue;
        }

        switch(ch)
        {
            case KEY_LEFT   : ch = 'h'; break;
            case KEY_DOWN   : ch = 'j'; break;
            case KEY_UP     : ch = 'k'; break;
            case KEY_RIGHT  : ch = 'l'; break;
            case KEY_HOME   : ch = 'y'; break;
            case KEY_PPAGE  : ch = 'u'; break;
            case KEY_END    : ch = 'b'; break;
#ifdef KEY_LL
            case KEY_LL            : ch = 'b'; break;
#endif
            case KEY_NPAGE  : ch = 'n'; break;

#ifdef KEY_B1
            case KEY_B1            : ch = 'h'; break;
            case KEY_C2     : ch = 'j'; break;
            case KEY_A2     : ch = 'k'; break;
            case KEY_B3            : ch = 'l'; break;
#endif
            case KEY_A1     : ch = 'y'; break;
            case KEY_A3     : ch = 'u'; break;
            case KEY_C1     : ch = 'b'; break;
            case KEY_C3     : ch = 'n'; break;
            /* next should be '.', but for problem with putty/linux */
            case KEY_B2            : ch = 'u'; break;

#ifdef KEY_SLEFT
            case KEY_SRIGHT  : ch = CTRL('L'); break;
            case KEY_SLEFT   : ch = CTRL('H'); break;
#ifdef KEY_SUP
            case KEY_SUP     : ch = CTRL('K'); break;
            case KEY_SDOWN   : ch = CTRL('J'); break;
#endif
            case KEY_SHOME   : ch = CTRL('Y'); break;
            case KEY_SPREVIOUS:ch = CTRL('U'); break;
            case KEY_SEND    : ch = CTRL('B'); break;
            case KEY_SNEXT   : ch = CTRL('N'); break;
#endif
            case 0x146       : ch = CTRL('K'); break;         /* Shift-Up        */
            case 0x145       : ch = CTRL('J'); break;         /* Shift-Down        */


#ifdef CTL_RIGHT
            case CTL_RIGHT   : ch = CTRL('L'); break;
            case CTL_LEFT    : ch = CTRL('H'); break;
            case CTL_UP      : ch = CTRL('K'); break;
            case CTL_DOWN    : ch = CTRL('J'); break;
            case CTL_HOME    : ch = CTRL('Y'); break;
            case CTL_PGUP    : ch = CTRL('U'); break;
            case CTL_END     : ch = CTRL('B'); break;
            case CTL_PGDN    : ch = CTRL('N'); break;
#endif
#ifdef KEY_EOL
            case KEY_EOL     : ch = CTRL('B'); break;
#endif

#ifndef CTL_PAD1
            /* MSYS rxvt console */
            case 511             : ch = CTRL('J'); break; /* Shift Dn */
            case 512         : ch = CTRL('J'); break; /* Ctl Down */
            case 514             : ch = CTRL('H'); break; /* Ctl Left */
            case 516             : ch = CTRL('L'); break; /* Ctl Right*/
            case 518             : ch = CTRL('K'); break; /* Shift Up */
            case 519             : ch = CTRL('K'); break; /* Ctl Up   */
#endif

#ifdef CTL_PAD1
            case CTL_PAD1   : ch = CTRL('B'); break;
            case CTL_PAD2   : ch = CTRL('J'); break;
            case CTL_PAD3   : ch = CTRL('N'); break;
            case CTL_PAD4   : ch = CTRL('H'); break;
            case CTL_PAD5   : ch = '.'; break;
            case CTL_PAD6   : ch = CTRL('L'); break;
            case CTL_PAD7   : ch = CTRL('Y'); break;
            case CTL_PAD8   : ch = CTRL('K'); break;
            case CTL_PAD9   : ch = CTRL('U'); break;
#endif

#ifdef ALT_RIGHT
            case ALT_RIGHT  : ch = CTRL('L'); break;
            case ALT_LEFT   : ch = CTRL('H'); break;
            case ALT_DOWN   : ch = CTRL('J'); break;
            case ALT_HOME   : ch = CTRL('Y'); break;
            case ALT_PGUP   : ch = CTRL('U'); break;
            case ALT_END    : ch = CTRL('B'); break;
            case ALT_PGDN   : ch = CTRL('N'); break;
#endif

#ifdef ALT_PAD1
            case ALT_PAD1   : ch = CTRL('B'); break;
            case ALT_PAD2   : ch = CTRL('J'); break;
            case ALT_PAD3   : ch = CTRL('N'); break;
            case ALT_PAD4   : ch = CTRL('H'); break;
            case ALT_PAD5   : ch = '.'; break;
            case ALT_PAD6   : ch = CTRL('L'); break;
            case ALT_PAD7   : ch = CTRL('Y'); break;
            case ALT_PAD8   : ch = CTRL('K'); break;
            case ALT_PAD9   : ch = CTRL('U'); break;
#endif
#ifdef KEY_BACKSPACE /* NCURSES in Keypad mode sends this for Ctrl-H */
            case KEY_BACKSPACE: ch = CTRL('H'); break;
#endif
        }

        break;
    }

    nocbreak();            /* disable halfdelay mode if on */
    raw();

    return(ch & 0x7F);
#endif

    while(true)
    {
        SDL_Event event;
        while (SDL_WaitEvent(&event)) 
        {
            switch(event.type)
            {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z)
                {
                    if (event.key.keysym.mod & KMOD_SHIFT)
                        event.key.keysym.sym = event.key.keysym.sym - 'a' + 'A';
                    if (event.key.keysym.mod & KMOD_CTRL)
                        return CTRL(event.key.keysym.sym);
                    return event.key.keysym.sym;
                }
                if (event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9)
                {
                    if (event.key.keysym.mod & KMOD_SHIFT)
                    {
                        if (event.key.keysym.sym == '1')
                            return '!';
                        if (event.key.keysym.sym == '2')
                            return '@';
                        if (event.key.keysym.sym == '3')
                            return '#';
                        if (event.key.keysym.sym == '4')
                            return '$';
                        if (event.key.keysym.sym == '5')
                            return '%';
                        if (event.key.keysym.sym == '6')
                            return '^';
                        if (event.key.keysym.sym == '7')
                            return '&';
                        if (event.key.keysym.sym == '8')
                            return '*';
                        if (event.key.keysym.sym == '9')
                            return '(';
                        if (event.key.keysym.sym == '0')
                            return ')';
                    }
                    return event.key.keysym.sym;
                }
                if (event.key.keysym.sym == SDLK_SPACE)
                    return ' ';
                if (event.key.keysym.sym == SDLK_LEFT)
                {
                    if (event.key.keysym.mod & KMOD_SHIFT)
                        return 'H';
                    return 'h';
                }
                if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    if (event.key.keysym.mod & KMOD_SHIFT)
                        return 'L';
                    return 'l';
                }
                if (event.key.keysym.sym == SDLK_UP)
                {
                    if (event.key.keysym.mod & KMOD_SHIFT)
                        return 'K';
                    return 'k';
                }
                if (event.key.keysym.sym == SDLK_DOWN)
                {
                    if (event.key.keysym.mod & KMOD_SHIFT)
                        return 'J';
                    return 'j';
                }
                if (event.key.keysym.sym == SDLK_SLASH)
                {
                    if (event.key.keysym.mod & KMOD_SHIFT)
                        return '?';
                    return '/';
                }
                if (event.key.keysym.sym == SDLK_PERIOD)
                {
                    if (event.key.keysym.mod & KMOD_SHIFT)
                        return '>';
                    return '.';
                }
                if (event.key.keysym.sym == SDLK_COMMA)
                {
                    if (event.key.keysym.mod & KMOD_SHIFT)
                        return '<';
                    return ',';
                }
                if (event.key.keysym.sym == SDLK_KP_MULTIPLY)
                    return '*';
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    return 27;
                if (event.key.keysym.sym == SDLK_RETURN)
                    return 13;
                if (event.key.keysym.sym == SDLK_BACKSPACE)
                    return 8;
                break;
            }
        }
    }
}

void md_flush_input()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
    }
}
