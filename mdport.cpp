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
#include <unistd.h>
#ifdef USE_SDL
#include <SDL/SDL.h>
#else
#include <stdio.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "extern.h"

void md_init()
{
#ifdef USE_SDL
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);
    SDL_SetVideoMode(900, 480, 32, SDL_SWSURFACE);
#endif
}

int
md_unlink(const char *file)
{
    return(unlink(file));
}

int md_unlink_open_file(const char *file, FILE *inf)
{
#ifdef __WIN32__
    fclose(inf);
#endif
    return unlink(file);
}

int
md_chmod(const char *filename, int mode)
{
    return 0;
}

int md_readchar()
{
#ifdef USE_SDL

#define HANDLE_KEY(k, n, s, c, a) \
    if (event.key.keysym.sym == (k)) { \
        if ((a) && (event.key.keysym.mod & KMOD_ALT)) return (a); \
        if ((c) && (event.key.keysym.mod & KMOD_CTRL)) return (c); \
        if ((s) && (event.key.keysym.mod & KMOD_SHIFT)) return (s); \
        if (n) return (n); \
    }
#define HANDLE_KEY_CHAR(k, n) \
    HANDLE_KEY(k, n, (n)-'a'+'A', CTRL(n), 0)

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
                    HANDLE_KEY_CHAR(event.key.keysym.sym, event.key.keysym.sym);
                }
                HANDLE_KEY(SDLK_ESCAPE, 27, 0, 0, 0);
                HANDLE_KEY(SDLK_1, '1', '!', 0, 0);
                HANDLE_KEY(SDLK_2, '2', '@', 0, 0);
                HANDLE_KEY(SDLK_3, '3', '#', 0, 0);
                HANDLE_KEY(SDLK_4, '4', '$', 0, 0);
                HANDLE_KEY(SDLK_5, '5', '%', 0, 0);
                HANDLE_KEY(SDLK_6, '6', '^', 0, 0);
                HANDLE_KEY(SDLK_7, '7', '&', 0, 0);
                HANDLE_KEY(SDLK_8, '8', '*', 0, 0);
                HANDLE_KEY(SDLK_9, '9', '(', 0, 0);
                HANDLE_KEY(SDLK_0, '0', ')', 0, 0);
                HANDLE_KEY(SDLK_MINUS, '-', '_', 0, 0);
                HANDLE_KEY(SDLK_EQUALS, '=', '+', 0, 0);
                HANDLE_KEY(SDLK_BACKSPACE, 8, 0, 0, 0);
                HANDLE_KEY(SDLK_TAB, 0, 0, 0, 0);
                HANDLE_KEY(SDLK_LEFTBRACKET, '[', '{', 0, 0);
                HANDLE_KEY(SDLK_RIGHTBRACKET, ']', '}', 0, 0);
                HANDLE_KEY(SDLK_RETURN, 13, 0, 0, 0);
                HANDLE_KEY(SDLK_SEMICOLON, ';', ':', 0, 0);
                HANDLE_KEY(SDLK_QUOTE, '\'', '"', 0, 0);
                HANDLE_KEY(SDLK_BACKQUOTE, '`', '~', 0, 0);
                HANDLE_KEY(SDLK_BACKSLASH, '\\', '|', 0, 0);
                HANDLE_KEY(SDLK_COMMA, ',', '<', 0, 0);
                HANDLE_KEY(SDLK_PERIOD, '.', '>', 0, 0);
                HANDLE_KEY(SDLK_SLASH, '/', '?', 0, 0);
                HANDLE_KEY(SDLK_KP_MULTIPLY, '*', 0, 0, 0);
                HANDLE_KEY(SDLK_SPACE, ' ', 0, 0, 0);
                
                HANDLE_KEY(SDLK_LEFT, K_LEFT, K_SHIFT_LEFT, CTRL(K_SHIFT_LEFT), 0);
                HANDLE_KEY(SDLK_RIGHT, K_RIGHT, K_SHIFT_RIGHT, CTRL(K_SHIFT_RIGHT), 0);
                HANDLE_KEY(SDLK_UP, K_UP, K_SHIFT_UP, CTRL(K_SHIFT_UP), 0);
                HANDLE_KEY(SDLK_DOWN, K_DOWN, K_SHIFT_DOWN, CTRL(K_SHIFT_DOWN), 0);

                HANDLE_KEY(SDLK_KP7, K_UP_LEFT, K_SHIFT_UP_LEFT, CTRL(K_SHIFT_UP_LEFT), 0);
                HANDLE_KEY(SDLK_KP8, K_UP, K_SHIFT_UP, CTRL(K_SHIFT_UP), 0);
                HANDLE_KEY(SDLK_KP9, K_UP_RIGHT, K_SHIFT_UP_RIGHT, CTRL(K_SHIFT_UP_RIGHT), 0);
                HANDLE_KEY(SDLK_KP_MINUS, '-', 0, 0, 0);
                HANDLE_KEY(SDLK_KP4, K_LEFT, K_SHIFT_LEFT, CTRL(K_SHIFT_LEFT), 0);
                HANDLE_KEY(SDLK_KP5, '.', 0, 0, 0);
                HANDLE_KEY(SDLK_KP6, K_RIGHT, K_SHIFT_RIGHT, CTRL(K_SHIFT_RIGHT), 0);
                HANDLE_KEY(SDLK_KP_PLUS, '+', 0, 0, 0);
                HANDLE_KEY(SDLK_KP1, K_DOWN_LEFT, K_SHIFT_DOWN_LEFT, CTRL(K_SHIFT_DOWN_LEFT), 0);
                HANDLE_KEY(SDLK_KP2, K_DOWN, K_SHIFT_DOWN, CTRL(K_SHIFT_DOWN), 0);
                HANDLE_KEY(SDLK_KP3, K_DOWN_RIGHT, K_SHIFT_DOWN_RIGHT, CTRL(K_SHIFT_DOWN_RIGHT), 0);
                HANDLE_KEY(SDLK_KP0, 0, 0, 0, 0);
                HANDLE_KEY(SDLK_KP_PERIOD, '.', 0, 0, 0);
                
                //printf("Unknown key: %s %d\n", SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.sym);
                break;
            }
        }
    }
#else
    static int handle = -1;
    static int mods = 0;
    const int LEFT_SHIFT = 0x01;
    const int RIGHT_SHIFT = 0x02;
    const int ANY_SHIFT = LEFT_SHIFT | RIGHT_SHIFT;
    const int LEFT_CTRL = 0x04;
    const int RIGHT_CTRL = 0x08;
    const int ANY_CTRL = LEFT_CTRL | RIGHT_CTRL;
    const int LEFT_ALT = 0x10;
    const int RIGHT_ALT = 0x20;
    const int ANT_ALT = LEFT_ALT | RIGHT_ALT;

#define HANDLE_MOD_KEY(k, m) \
    if (event.code == (k)) { if (event.value) { mods |= (m); } else { mods &=~(m); } continue; }
#define HANDLE_KEY(k, n, s, c, a) \
    if (event.value && event.code == (k)) { \
        if ((a) && (mods & ANY_ALT)) return (a); \
        if ((c) && (mods & ANY_CTRL)) return (c); \
        if ((s) && (mods & ANY_SHIFT)) return (s); \
        if (n) return (n); \
    }
#define HANDLE_KEY_CHAR(k, n) \
    HANDLE_KEY(k, n, (n)-'a'+'A', CTRL(n), 0)

    if (handle == -1)
    {
        handle = open("/dev/input4", O_RDWR);
    }

    struct input_event event;
    while(read(handle, &event, sizeof(event)) == sizeof(event))
    {
        if (event.type == EV_KEY)
        {
            HANDLE_MOD_KEY(KEY_LEFTSHIFT, LEFT_SHIFT);
            HANDLE_MOD_KEY(KEY_RIGHTSHIFT, RIGHT_SHIFT);
            HANDLE_MOD_KEY(KEY_LEFTCTRL, LEFT_CTRL);
            HANDLE_MOD_KEY(KEY_RIGHTCTRL, RIGHT_CTRL);
            HANDLE_MOD_KEY(KEY_LEFTALT, LEFT_ALT);
            HANDLE_MOD_KEY(KEY_RIGHTALT, RIGHT_ALT);

            HANDLE_KEY(KEY_ESC, ESCAPE, 27, 0, 0);
            HANDLE_KEY(KEY_1, '1', '!', 0, 0);
            HANDLE_KEY(KEY_2, '2', '@', 0, 0);
            HANDLE_KEY(KEY_3, '3', '#', 0, 0);
            HANDLE_KEY(KEY_4, '4', '$', 0, 0);
            HANDLE_KEY(KEY_5, '5', '%', 0, 0);
            HANDLE_KEY(KEY_6, '6', '^', 0, 0);
            HANDLE_KEY(KEY_7, '7', '&', 0, 0);
            HANDLE_KEY(KEY_8, '8', '*', 0, 0);
            HANDLE_KEY(KEY_9, '9', '(', 0, 0);
            HANDLE_KEY(KEY_0, '0', ')', 0, 0);
            HANDLE_KEY(KEY_MINUS, '-', '_', 0, 0);
            HANDLE_KEY(KEY_EQUAL, '=', '+', 0, 0);
            HANDLE_KEY(KEY_BACKSPACE, 8, 0, 0, 0);
            HANDLE_KEY(KEY_TAB, 0, 0, 0, 0);
            HANDLE_KEY_CHAR(KEY_Q, 'q');
            HANDLE_KEY_CHAR(KEY_W, 'w');
            HANDLE_KEY_CHAR(KEY_E, 'e');
            HANDLE_KEY_CHAR(KEY_R, 'r');
            HANDLE_KEY_CHAR(KEY_T, 't');
            HANDLE_KEY_CHAR(KEY_Y, 'y');
            HANDLE_KEY_CHAR(KEY_U, 'u');
            HANDLE_KEY_CHAR(KEY_I, 'i');
            HANDLE_KEY_CHAR(KEY_O, 'o');
            HANDLE_KEY_CHAR(KEY_P, 'p');
            HANDLE_KEY(KEY_LEFTBRACE, '[', '{', 0, 0);
            HANDLE_KEY(KEY_RIGHTBRACE, ']', '}', 0, 0);
            HANDLE_KEY(KEY_ENTER, 13, 0, 0, 0);
            HANDLE_KEY_CHAR(KEY_A, 'a');
            HANDLE_KEY_CHAR(KEY_S, 's');
            HANDLE_KEY_CHAR(KEY_D, 'd');
            HANDLE_KEY_CHAR(KEY_F, 'f');
            HANDLE_KEY_CHAR(KEY_G, 'g');
            HANDLE_KEY_CHAR(KEY_H, 'h');
            HANDLE_KEY_CHAR(KEY_J, 'j');
            HANDLE_KEY_CHAR(KEY_K, 'k');
            HANDLE_KEY_CHAR(KEY_L, 'l');
            HANDLE_KEY(KEY_SEMICOLON, ';', ':', 0, 0);
            HANDLE_KEY(KEY_APOSTROPHE, '\'', '"', 0, 0);
            HANDLE_KEY(KEY_GRAVE, '`', '~', 0, 0);
            HANDLE_KEY(KEY_BACKSLASH, '\\', '|', 0, 0);
            HANDLE_KEY_CHAR(KEY_Z, 'z');
            HANDLE_KEY_CHAR(KEY_X, 'x');
            HANDLE_KEY_CHAR(KEY_C, 'c');
            HANDLE_KEY_CHAR(KEY_V, 'v');
            HANDLE_KEY_CHAR(KEY_B, 'b');
            HANDLE_KEY_CHAR(KEY_N, 'n');
            HANDLE_KEY_CHAR(KEY_M, 'm');
            HANDLE_KEY(KEY_COMMA, ',', '<', 0, 0);
            HANDLE_KEY(KEY_DOT, '.', '>', 0, 0);
            HANDLE_KEY(KEY_SLASH, '/', '?', 0, 0);
            HANDLE_KEY(KEY_KPASTERISK, '*', 0, 0, 0);
            HANDLE_KEY(KEY_SPACE, ' ', 0, 0, 0);
            HANDLE_KEY(KEY_CAPSLOCK, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F1, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F2, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F3, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F4, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F5, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F6, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F7, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F8, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F9, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F10, 0, 0, 0, 0);
            HANDLE_KEY(KEY_NUMLOCK, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SCROLLLOCK, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KP7, K_UP_LEFT, K_SHIFT_UP_LEFT, CTRL(K_SHIFT_UP_LEFT), 0);
            HANDLE_KEY(KEY_KP8, K_UP, K_SHIFT_UP, CTRL(K_SHIFT_UP), 0);
            HANDLE_KEY(KEY_KP9, K_UP_RIGHT, K_SHIFT_UP_RIGHT, CTRL(K_SHIFT_UP_RIGHT), 0);
            HANDLE_KEY(KEY_KPMINUS, '-', 0, 0, 0);
            HANDLE_KEY(KEY_KP4, K_LEFT, K_SHIFT_LEFT, CTRL(K_SHIFT_LEFT), 0);
            HANDLE_KEY(KEY_KP5, '.', 0, 0, 0);
            HANDLE_KEY(KEY_KP6, K_RIGHT, K_SHIFT_RIGHT, CTRL(K_SHIFT_RIGHT), 0);
            HANDLE_KEY(KEY_KPPLUS, '+', 0, 0, 0);
            HANDLE_KEY(KEY_KP1, K_DOWN_LEFT, K_SHIFT_DOWN_LEFT, CTRL(K_SHIFT_DOWN_LEFT), 0);
            HANDLE_KEY(KEY_KP2, K_DOWN, K_SHIFT_DOWN, CTRL(K_SHIFT_DOWN), 0);
            HANDLE_KEY(KEY_KP3, K_DOWN_RIGHT, K_SHIFT_DOWN_RIGHT, CTRL(K_SHIFT_DOWN_RIGHT), 0);
            HANDLE_KEY(KEY_KP0, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KPDOT, '.', 0, 0, 0);

            HANDLE_KEY(KEY_ZENKAKUHANKAKU, 0, 0, 0, 0);
            HANDLE_KEY(KEY_102ND, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F11, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F12, 0, 0, 0, 0);
            HANDLE_KEY(KEY_RO, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KATAKANA, 0, 0, 0, 0);
            HANDLE_KEY(KEY_HIRAGANA, 0, 0, 0, 0);
            HANDLE_KEY(KEY_HENKAN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KATAKANAHIRAGANA, 0, 0, 0, 0);
            HANDLE_KEY(KEY_MUHENKAN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KPJPCOMMA, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KPENTER, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KPSLASH, '/', 0, 0, 0);
            HANDLE_KEY(KEY_SYSRQ, 0, 0, 0, 0);
            HANDLE_KEY(KEY_LINEFEED, 0, 0, 0, 0);
            HANDLE_KEY(KEY_HOME, 0, 0, 0, 0);
            HANDLE_KEY(KEY_UP, KEY_UP, K_SHIFT_UP, CTRL(K_SHIFT_UP), 0);
            HANDLE_KEY(KEY_PAGEUP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_LEFT, K_LEFT, K_SHIFT_LEFT, CTRL(K_SHIFT_LEFT), 0);
            HANDLE_KEY(KEY_RIGHT, K_RIGHT, K_SHIFT_RIGHT, CTRL(K_SHIFT_RIGHT), 0);
            HANDLE_KEY(KEY_END, 0, 0, 0, 0);
            HANDLE_KEY(KEY_DOWN, K_DOWN, K_SHIFT_DOWN, CTRL(K_SHIFT_DOWN), 0);
            HANDLE_KEY(KEY_PAGEDOWN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_INSERT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_DELETE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_MACRO, 0, 0, 0, 0);
            HANDLE_KEY(KEY_MUTE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_VOLUMEDOWN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_VOLUMEUP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_POWER, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KPEQUAL, '=', 0, 0, 0);
            HANDLE_KEY(KEY_KPPLUSMINUS, '-', 0, 0, 0);
            HANDLE_KEY(KEY_PAUSE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SCALE, 0, 0, 0, 0);

            HANDLE_KEY(KEY_KPCOMMA, ',', 0, 0, 0);
            HANDLE_KEY(KEY_HANGEUL, 0, 0, 0, 0);
            HANDLE_KEY(KEY_HANGUEL, 0, 0, 0, 0);
            HANDLE_KEY(KEY_HANJA, 0, 0, 0, 0);
            HANDLE_KEY(KEY_YEN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_LEFTMETA, 0, 0, 0, 0);
            HANDLE_KEY(KEY_RIGHTMETA, 0, 0, 0, 0);
            HANDLE_KEY(KEY_COMPOSE, 0, 0, 0, 0);

            HANDLE_KEY(KEY_STOP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_AGAIN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PROPS, 0, 0, 0, 0);
            HANDLE_KEY(KEY_UNDO, 0, 0, 0, 0);
            HANDLE_KEY(KEY_FRONT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_COPY, 0, 0, 0, 0);
            HANDLE_KEY(KEY_OPEN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PASTE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_FIND, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CUT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_HELP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_MENU, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CALC, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SETUP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SLEEP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_WAKEUP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_FILE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SENDFILE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_DELETEFILE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_XFER, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PROG1, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PROG2, 0, 0, 0, 0);
            HANDLE_KEY(KEY_WWW, 0, 0, 0, 0);
            HANDLE_KEY(KEY_MSDOS, 0, 0, 0, 0);
            HANDLE_KEY(KEY_COFFEE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SCREENLOCK, 0, 0, 0, 0);
            HANDLE_KEY(KEY_ROTATE_DISPLAY, 0, 0, 0, 0);
            HANDLE_KEY(KEY_DIRECTION, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CYCLEWINDOWS, 0, 0, 0, 0);
            HANDLE_KEY(KEY_MAIL, 0, 0, 0, 0);
            HANDLE_KEY(KEY_BOOKMARKS, 0, 0, 0, 0);
            HANDLE_KEY(KEY_COMPUTER, 0, 0, 0, 0);
            HANDLE_KEY(KEY_BACK, 0, 0, 0, 0);
            HANDLE_KEY(KEY_FORWARD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CLOSECD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_EJECTCD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_EJECTCLOSECD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_NEXTSONG, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PLAYPAUSE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PREVIOUSSONG, 0, 0, 0, 0);
            HANDLE_KEY(KEY_STOPCD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_RECORD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_REWIND, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PHONE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_ISO, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CONFIG, 0, 0, 0, 0);
            HANDLE_KEY(KEY_HOMEPAGE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_REFRESH, 0, 0, 0, 0);
            HANDLE_KEY(KEY_EXIT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_MOVE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_EDIT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SCROLLUP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SCROLLDOWN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KPLEFTPAREN, '(', 0, 0, 0);
            HANDLE_KEY(KEY_KPRIGHTPAREN, ')', 0, 0, 0);
            HANDLE_KEY(KEY_NEW, 0, 0, 0, 0);
            HANDLE_KEY(KEY_REDO, 0, 0, 0, 0);

            HANDLE_KEY(KEY_F13, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F14, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F15, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F16, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F17, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F18, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F19, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F20, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F21, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F22, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F23, 0, 0, 0, 0);
            HANDLE_KEY(KEY_F24, 0, 0, 0, 0);

            HANDLE_KEY(KEY_PLAYCD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PAUSECD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PROG3, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PROG4, 0, 0, 0, 0);
            HANDLE_KEY(KEY_DASHBOARD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SUSPEND, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CLOSE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PLAY, 0, 0, 0, 0);
            HANDLE_KEY(KEY_FASTFORWARD, 0, 0, 0, 0);
            HANDLE_KEY(KEY_BASSBOOST, 0, 0, 0, 0);
            HANDLE_KEY(KEY_PRINT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_HP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CAMERA, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SOUND, 0, 0, 0, 0);
            HANDLE_KEY(KEY_QUESTION, 0, 0, 0, 0);
            HANDLE_KEY(KEY_EMAIL, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CHAT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SEARCH, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CONNECT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_FINANCE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SPORT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SHOP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_ALTERASE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_CANCEL, 0, 0, 0, 0);
            HANDLE_KEY(KEY_BRIGHTNESSDOWN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_BRIGHTNESSUP, 0, 0, 0, 0);
            HANDLE_KEY(KEY_MEDIA, 0, 0, 0, 0);

            HANDLE_KEY(KEY_SWITCHVIDEOMODE, 0, 0, 0, 0);

            HANDLE_KEY(KEY_KBDILLUMTOGGLE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KBDILLUMDOWN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_KBDILLUMUP, 0, 0, 0, 0);

            HANDLE_KEY(KEY_SEND, 0, 0, 0, 0);
            HANDLE_KEY(KEY_REPLY, 0, 0, 0, 0);
            HANDLE_KEY(KEY_FORWARDMAIL, 0, 0, 0, 0);
            HANDLE_KEY(KEY_SAVE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_DOCUMENTS, 0, 0, 0, 0);

            HANDLE_KEY(KEY_BATTERY, 0, 0, 0, 0);

            HANDLE_KEY(KEY_BLUETOOTH, 0, 0, 0, 0);
            HANDLE_KEY(KEY_WLAN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_UWB, 0, 0, 0, 0);

            HANDLE_KEY(KEY_UNKNOWN, 0, 0, 0, 0);

            HANDLE_KEY(KEY_VIDEO_NEXT, 0, 0, 0, 0);
            HANDLE_KEY(KEY_VIDEO_PREV, 0, 0, 0, 0);
            HANDLE_KEY(KEY_BRIGHTNESS_CYCLE, 0, 0, 0, 0);
            HANDLE_KEY(KEY_BRIGHTNESS_AUTO, 0, 0, 0, 0);

            HANDLE_KEY(KEY_BRIGHTNESS_ZERO, 0, 0, 0, 0);
            HANDLE_KEY(KEY_DISPLAY_OFF, 0, 0, 0, 0);

            HANDLE_KEY(KEY_WWAN, 0, 0, 0, 0);
            HANDLE_KEY(KEY_WIMAX, 0, 0, 0, 0);
            HANDLE_KEY(KEY_RFKILL, 0, 0, 0, 0);

            HANDLE_KEY(KEY_MICMUTE, 0, 0, 0, 0);
        }
    }
    printf("Failed to read input device?\n");
    return 0;
#endif
}

void md_flush_input()
{
#ifdef USE_SDL
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
    }
#endif
}
