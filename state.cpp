/*
    state.c - Portable Rogue Save State Code

    Copyright (C) 1999, 2000, 2005 Nicholas J. Kisseberth
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
#include "rogue.h"
#include "areas.h"

/************************************************************************/
/* Save State Code                                                      */
/************************************************************************/

#define RSID_STATS        0xABCD0001
#define RSID_THING        0xABCD0002
#define RSID_THING_NULL   0xDEAD0002
#define RSID_OBJECT       0xABCD0003
#define RSID_MAGICITEMS   0xABCD0004
#define RSID_KNOWS        0xABCD0005
#define RSID_GUESSES      0xABCD0006
#define RSID_OBJECTLIST   0xABCD0007
#define RSID_BAGOBJECT    0xABCD0008
#define RSID_MONSTERLIST  0xABCD0009
#define RSID_MONSTERSTATS 0xABCD000A
#define RSID_MONSTERS     0xABCD000B
#define RSID_TRAP         0xABCD000C
#define RSID_WINDOW       0xABCD000D
#define RSID_DAEMONS      0xABCD000E
#define RSID_IWEAPS       0xABCD000F
#define RSID_IARMOR       0xABCD0010
#define RSID_SPELLS       0xABCD0011
#define RSID_ILIST        0xABCD0012
#define RSID_HLIST        0xABCD0013
#define RSID_DEATHTYPE    0xABCD0014
#define RSID_CTYPES       0XABCD0015
#define RSID_COORDLIST    0XABCD0016
#define RSID_ROOMS        0XABCD0017

#define READSTAT (format_error || read_error )
#define WRITESTAT (write_error)

static int read_error   = FALSE;
static int write_error  = FALSE;
static int format_error = FALSE;
static int endian = 0x01020304;
#define  big_endian ( *((char *)&endian) == 0x01 )

int
rs_write(FILE *savef, const void *ptr, size_t size)
{
    if (write_error)
        return(WRITESTAT);

    if (encwrite(ptr, size, savef) != size)
        write_error = 1;

    return(WRITESTAT);
}

int
rs_read(FILE *inf, void *ptr, size_t size)
{
    if (read_error || format_error)
        return(READSTAT);

    if (encread(ptr, size, inf) != size)
        read_error = 1;
    
    return(READSTAT);
}

int
rs_write_int(FILE *savef, int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (write_error)
        return(WRITESTAT);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_read_int(FILE *inf, int *i)
{
    unsigned char bytes[4];
    int input = 0;
    unsigned char *buf = (unsigned char *)&input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((int *) buf);

    return(READSTAT);
}

int
rs_write_char(FILE *savef, char c)
{
    if (write_error)
        return(WRITESTAT);

    rs_write(savef, &c, 1);

    return(WRITESTAT);
}

int
rs_read_char(FILE *inf, char *c)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, c, 1);

    return(READSTAT);
}

int
rs_write_chars(FILE *savef, const char *c, int count)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);
    rs_write(savef, c, count);

    return(WRITESTAT);
}

int
rs_read_chars(FILE *inf, char *i, int count)
{
    int value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &value);
    
    if (value != count)
        format_error = TRUE;

    rs_read(inf, i, count);
    
    return(READSTAT);
}

int
rs_write_ints(FILE *savef, int *c, int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
        if( rs_write_int(savef,c[n]) != 0)
            break;

    return(WRITESTAT);
}

int
rs_read_ints(FILE *inf, int *i, int count)
{
    int n, value;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        if (rs_read_int(inf, &i[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_boolean(FILE *savef, int c)
{
    unsigned char buf = (c == 0) ? 0 : 1;
    
    if (write_error)
        return(WRITESTAT);

    rs_write(savef, &buf, 1);

    return(WRITESTAT);
}

int
rs_read_boolean(FILE *inf, bool *i)
{
    unsigned char buf = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &buf, 1);

    *i = (buf != 0);
    
    return(READSTAT);
}

int
rs_write_booleans(FILE *savef, bool *c, int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
        if (rs_write_boolean(savef, c[n]) != 0)
            break;

    return(WRITESTAT);
}

int
rs_read_booleans(FILE *inf, bool *i, int count)
{
    int n = 0, value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        if (rs_read_boolean(inf, &i[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_short(FILE *savef, short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (write_error)
        return(WRITESTAT);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_read_short(FILE *inf, short *i)
{
    unsigned char bytes[2];
    short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 2);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((short *) buf);

    return(READSTAT);
} 

int
rs_write_shorts(FILE *savef, short *c, int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
        if (rs_write_short(savef, c[n]) != 0)
            break; 

    return(WRITESTAT);
}

int
rs_read_shorts(FILE *inf, short *i, int count)
{
    int n = 0, value = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < value; n++)
        if (rs_read_short(inf, &i[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_ushort(FILE *savef, unsigned short c)
{
    unsigned char bytes[2];
    unsigned char *buf = (unsigned char *) &c;

    if (write_error)
        return(WRITESTAT);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }

    rs_write(savef, buf, 2);

    return(WRITESTAT);
}

int
rs_read_ushort(FILE *inf, unsigned short *i)
{
    unsigned char bytes[2];
    unsigned short  input;
    unsigned char *buf = (unsigned char *)&input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 2);

    if (big_endian)
    {
        bytes[1] = buf[0];
        bytes[0] = buf[1];
        buf = bytes;
    }
    
    *i = *((unsigned short *) buf);

    return(READSTAT);
} 

int
rs_write_uint(FILE *savef, unsigned int c)
{
    unsigned char bytes[4];
    unsigned char *buf = (unsigned char *) &c;

    if (write_error)
        return(WRITESTAT);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    rs_write(savef, buf, 4);

    return(WRITESTAT);
}

int
rs_read_uint(FILE *inf, unsigned int *i)
{
    unsigned char bytes[4];
    int  input;
    unsigned char *buf = (unsigned char *)&input;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read(inf, &input, 4);

    if (big_endian)
    {
        bytes[3] = buf[0];
        bytes[2] = buf[1];
        bytes[1] = buf[2];
        bytes[0] = buf[3];
        buf = bytes;
    }
    
    *i = *((unsigned int *) buf);

    return(READSTAT);
}

int
rs_write_marker(FILE *savef, int id)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, id);

    return(WRITESTAT);
}

int 
rs_read_marker(FILE *inf, int id)
{
    int nid;

    if (read_error || format_error)
        return(READSTAT);

    if (rs_read_int(inf, &nid) == 0)
        if (id != nid)
            format_error = 1;
    
    return(READSTAT);
}



/******************************************************************************/

int
rs_write_string(FILE *savef, const char *s)
{
    int len = 0;

    if (write_error)
        return(WRITESTAT);

    len = (s == NULL) ? 0 : (int) strlen(s) + 1;

    rs_write_int(savef, len);
    rs_write_chars(savef, s, len);
            
    return(WRITESTAT);
}

int
rs_read_string(FILE *inf, char *s, int max)
{
    int len = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &len);

    if (len > max)
        format_error = TRUE;

    rs_read_chars(inf, s, len);
    
    return(READSTAT);
}

int
rs_read_new_string(FILE *inf, char **s)
{
    int len=0;
    char *buf=0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &len);

    if (len == 0)
        buf = NULL;
    else
    { 
        buf = (char*)malloc(len);

        if (buf == NULL)            
            read_error = TRUE;
    }

    rs_read_chars(inf, buf, len);

    *s = buf;

    return(READSTAT);
}

int
rs_write_strings(FILE *savef, char *s[], int count)
{
    int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
        if (rs_write_string(savef, s[n]) != 0)
            break;
    
    return(WRITESTAT);
}

int
rs_read_strings(FILE *inf, char **s, int count, int max)
{
    int n     = 0;
    int value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        if (rs_read_string(inf, s[n], max) != 0)
            break;
    
    return(READSTAT);
}

int
rs_read_new_strings(FILE *inf, char **s, int count)
{
    int n     = 0;
    int value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        if (rs_read_new_string(inf, &s[n]) != 0)
            break;
    
    return(READSTAT);
}

int
rs_write_string_index(FILE *savef, const char *master[], int max, const char *str)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < max; i++)
        if (str == master[i])
            return( rs_write_int(savef, i) );

    return( rs_write_int(savef,-1) );
}

int rs_read_string_index(FILE *inf, const char *master[], int maxindex, const char **str)
{
    int i;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &i);

    if (i > maxindex)
        format_error = TRUE;
    else if (i >= 0)
        *str = master[i];
    else
        *str = NULL;

    return(READSTAT);
}

int
rs_write_str_t(FILE *savef, str_t st)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_uint(savef, st);

    return( WRITESTAT );
}

int
rs_read_str_t(FILE *inf, str_t *st)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read_uint(inf, st);

    return(READSTAT);
}

int
rs_write_coord(FILE *savef, coord c)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, c.x);
    rs_write_int(savef, c.y);
    
    return(WRITESTAT);
}

int
rs_read_coord(FILE *inf, coord *c)
{
    coord in;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&in.x);
    rs_read_int(inf,&in.y);

    if (READSTAT == 0) 
    {
        c->x = in.x;
        c->y = in.y;
    }

    return(READSTAT);
}

int
rs_write_window(FILE *savef)
{
    int row,col,height,width;

    if (write_error)
        return(WRITESTAT);

    width  = NUMCOLS;
    height = NUMLINES;

    rs_write_marker(savef,RSID_WINDOW);
    rs_write_int(savef,height);
    rs_write_int(savef,width);

    for(row=0;row<height;row++)
        for(col=0;col<width;col++)
            if (rs_write_int(savef, getMapDisplay(col, row)) != 0)
                return(WRITESTAT);

    return(WRITESTAT);
}

int
rs_read_window(FILE *inf)
{
    int row,col,maxlines,maxcols,value,width,height;
    
    if (read_error || format_error)
        return(READSTAT);

    width  = NUMCOLS;
    height = NUMLINES;

    rs_read_marker(inf, RSID_WINDOW);

    rs_read_int(inf, &maxlines);
    rs_read_int(inf, &maxcols);

    for(row = 0; row < maxlines; row++)
        for(col = 0; col < maxcols; col++)
        {
            if (rs_read_int(inf, &value) != 0)
                return(READSTAT);

            if ((row < height) && (col < width))
                setMapDisplay(col, row, value);
        }
        
    return(READSTAT);
}

/******************************************************************************/

ItemThing* get_list_item(std::list<ItemThing*>& l, int i)
{
    int count = 0;
    for(ItemThing* obj : l)
    {
        if (count == i)
            return obj;
        count++;
    }
    return(NULL);
}

int find_list_ptr(std::list<ItemThing*>& l, ItemThing *ptr)
{
    int count = 0;
    for(ItemThing* obj : l)
    {
        if (obj == ptr)
            return(count);
        count++;
    }
    
    return(-1);
}

/******************************************************************************/

int
rs_write_stats(FILE *savef, struct stats *s)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_STATS);
    rs_write_str_t(savef, s->s_str);
    rs_write_int(savef, s->s_exp);
    rs_write_int(savef, s->s_lvl);
    rs_write_int(savef, s->s_arm);
    rs_write_int(savef, s->s_hpt);
    rs_write_chars(savef, s->s_dmg, sizeof(s->s_dmg));
    rs_write_int(savef,s->s_maxhp);

    return(WRITESTAT);
}

int
rs_read_stats(FILE *inf, struct stats *s)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_STATS);
    rs_read_str_t(inf,&s->s_str);
    rs_read_int(inf,&s->s_exp);
    rs_read_int(inf,&s->s_lvl);
    rs_read_int(inf,&s->s_arm);
    rs_read_int(inf,&s->s_hpt);
    rs_read_chars(inf,s->s_dmg,sizeof(s->s_dmg));
    rs_read_int(inf,&s->s_maxhp);

    return(READSTAT);
}

int
rs_write_stone_index(FILE *savef, STONE master[], int max, const char *str)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < max; i++)
        if (str == master[i].st_name)
        {
            rs_write_int(savef,i);
            return(WRITESTAT);
        }

    rs_write_int(savef,-1);

    return(WRITESTAT);
}

int
rs_read_stone_index(FILE *inf, STONE master[], int maxindex, const char **str)
{
    int i = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&i);

    if (i > maxindex)
        format_error = TRUE;
    else if (i >= 0)
        *str = master[i].st_name;
    else
        *str = NULL;

    return(READSTAT);
}

int
rs_write_scrolls(FILE *savef)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < MAXSCROLLS; i++)
        rs_write_string(savef, s_names[i]);

    return(READSTAT);
}

int
rs_read_scrolls(FILE *inf)
{
    int i;

    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < MAXSCROLLS; i++)
        rs_read_new_string(inf, &s_names[i]);

    return(READSTAT);
}

int
rs_write_potions(FILE *savef)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < MAXPOTIONS; i++)
        rs_write_string_index(savef, rainbow, cNCOLORS, p_colors[i]);

    return(WRITESTAT);
}

int
rs_read_potions(FILE *inf)
{
    int i;

    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < MAXPOTIONS; i++)
        rs_read_string_index(inf, rainbow, cNCOLORS, &p_colors[i]);

    return(READSTAT);
}

int
rs_write_rings(FILE *savef)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < MAXRINGS; i++)
        rs_write_stone_index(savef, stones, cNSTONES, r_stones[i]);

    return(WRITESTAT);
}

int
rs_read_rings(FILE *inf)
{
    int i;

    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < MAXRINGS; i++)
        rs_read_stone_index(inf, stones, cNSTONES, &r_stones[i]);

    return(READSTAT);
}

int
rs_write_sticks(FILE *savef)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    for (i = 0; i < MAXSTICKS; i++)
    {
        if (strcmp(ws_type[i],"staff") == 0)
        {
            rs_write_int(savef,0);
            rs_write_string_index(savef, wood, cNWOOD, ws_made[i]);
        }
        else
        {
            rs_write_int(savef,1);
            rs_write_string_index(savef, metal, cNMETAL, ws_made[i]);
        }
    }
 
    return(WRITESTAT);
}
        
int
rs_read_sticks(FILE *inf)
{
    int i = 0, list = 0;

    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < MAXSTICKS; i++)
    { 
        rs_read_int(inf,&list);

        if (list == 0)
        {
            rs_read_string_index(inf, wood, cNWOOD, &ws_made[i]);
            ws_type[i] = "staff";
        }
        else 
        {
            rs_read_string_index(inf, metal, cNMETAL, &ws_made[i]);
            ws_type[i] = "wand";
        }
    }

    return(READSTAT);
}

int
rs_write_daemons(FILE *savef, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
        
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_DAEMONS);
    rs_write_int(savef, count);
        
    for(i = 0; i < count; i++)
    {
        if (d_list[i].d_func == rollwand)
            func = 1;
        else if (d_list[i].d_func == doctor)
            func = 2;
        else if (d_list[i].d_func == stomach)
            func = 3;
        else if (d_list[i].d_func == runners)
            func = 4;
        else if (d_list[i].d_func == swander)
            func = 5;
        else if (d_list[i].d_func == nohaste)
            func = 6;
        else if (d_list[i].d_func == unconfuse)
            func = 7;
        else if (d_list[i].d_func == unsee)
            func = 8;
        else if (d_list[i].d_func == sight)
            func = 9;
        else if (d_list[i].d_func == NULL)
            func = 0;
        else
            func = -1;

        rs_write_int(savef, d_list[i].d_type);
        rs_write_int(savef, func);
        rs_write_int(savef, d_list[i].d_arg);
        rs_write_int(savef, d_list[i].d_time);
    }
    
    return(WRITESTAT);
}       

int
rs_read_daemons(FILE *inf, struct delayed_action *d_list, int count)
{
    int i = 0;
    int func = 0;
    int value = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_DAEMONS);
    rs_read_int(inf, &value);

    if (value > count)
        format_error = TRUE;

    for(i=0; i < count; i++)
    {
        func = 0;
        rs_read_int(inf, &d_list[i].d_type);
        rs_read_int(inf, &func);
        rs_read_int(inf, &d_list[i].d_arg);
        rs_read_int(inf, &d_list[i].d_time);
                    
        switch(func)
        {
            case 1: d_list[i].d_func = rollwand;
                    break;
            case 2: d_list[i].d_func = doctor;
                    break;
            case 3: d_list[i].d_func = stomach;
                    break;
            case 4: d_list[i].d_func = runners;
                    break;
            case 5: d_list[i].d_func = swander;
                    break;
            case 6: d_list[i].d_func = nohaste;
                    break;
            case 7: d_list[i].d_func = unconfuse;
                    break;
            case 8: d_list[i].d_func = unsee;
                    break;
            case 9: d_list[i].d_func = sight;
                    break;
            default:d_list[i].d_func = NULL;
                    break;
        }
    }

    if (d_list[i].d_func == NULL)
    {
        d_list[i].d_type = 0;
        d_list[i].d_arg = 0;
        d_list[i].d_time = 0;
    }
    
    return(READSTAT);
}       
        
static int rs_write_obj_info(FILE *savef, struct obj_info *i, int count)
{
    int n;
    
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_MAGICITEMS);
    rs_write_int(savef, count);

    for(n = 0; n < count; n++)
    {
        /* mi_name is constant, defined at compile time in all cases */
        rs_write_int(savef,i[n].oi_prob);
        rs_write_int(savef,i[n].oi_worth);
        rs_write_string(savef,i[n].oi_guess);
        rs_write_boolean(savef,i[n].oi_know);
    }
    
    return(WRITESTAT);
}

static int rs_read_obj_info(FILE *inf, struct obj_info *mi, int count)
{
    int n;
    int value;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_MAGICITEMS);

    rs_read_int(inf, &value);

    if (value > count)
        format_error = TRUE;

    for(n = 0; n < value; n++)
    {
        /* mi_name is const, defined at compile time in all cases */
        rs_read_int(inf,&mi[n].oi_prob);
        rs_read_int(inf,&mi[n].oi_worth);
        rs_read_new_string(inf,&mi[n].oi_guess);
        rs_read_boolean(inf,&mi[n].oi_know);
    }
    
    return(READSTAT);
}

static int rs_write_area(FILE *savef, Area* area)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_coord(savef, area->position);
    rs_write_coord(savef, area->size);
    rs_write_int(savef, area->flags);
    
    return(WRITESTAT);
}

static int rs_read_area(FILE *inf, Area *area)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read_coord(inf,&area->position);
    rs_read_coord(inf,&area->size);
    rs_read_int(inf,&area->flags);

    return(READSTAT);
}

static int rs_write_areas(FILE *savef)
{
    unsigned int n = 0;

    if (write_error)
        return(WRITESTAT);

    rs_write_int(savef, areas.size());
    
    for(n = 0; n < areas.size(); n++)
        rs_write_area(savef, &areas[n]);
    
    return(WRITESTAT);
}

static int rs_read_areas(FILE *inf)
{
    int value = 0, n = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf,&value);

    if (value < 0)
        format_error = TRUE;
    
    areas.resize(value);
    for(n = 0; n < value; n++)
        rs_read_area(inf, &areas[n]);

    return(READSTAT);
}

static int rs_write_monsters(FILE *savef, struct monster *m, int count)
{
    int n;
    
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_MONSTERS);
    rs_write_int(savef, count);

    for(n=0;n<count;n++)
        rs_write_stats(savef, &m[n].m_stats);
    
    return(WRITESTAT);
}

static int rs_read_monsters(FILE *inf, struct monster *m, int count)
{
    int value = 0, n = 0;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_MONSTERS);

    rs_read_int(inf, &value);

    if (value != count)
        format_error = TRUE;

    for(n = 0; n < count; n++)
        rs_read_stats(inf, &m[n].m_stats);
    
    return(READSTAT);
}

static int rs_write_object(FILE *savef, ItemThing *o)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_OBJECT);
    rs_write_int(savef, o->type); 
    rs_write_coord(savef, o->pos); 
    rs_write_int(savef, o->launch);
    rs_write_char(savef, o->packch);
    rs_write_chars(savef, o->damage, sizeof(o->damage));
    rs_write_chars(savef, o->hurldmg, sizeof(o->hurldmg));
    rs_write_int(savef, o->count);
    rs_write_int(savef, o->which);
    rs_write_int(savef, o->hplus);
    rs_write_int(savef, o->dplus);
    rs_write_int(savef, o->arm);
    rs_write_int(savef, o->flags);
    rs_write_int(savef, o->group);
    rs_write_string(savef, o->label);
    return(WRITESTAT);
}

static int rs_read_object(FILE *inf, ItemThing *o)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_OBJECT);
    rs_read_int(inf, &o->type);
    rs_read_coord(inf, &o->pos);
    rs_read_int(inf, &o->launch);
    rs_read_char(inf, &o->packch);
    rs_read_chars(inf, o->damage, sizeof(o->damage));
    rs_read_chars(inf, o->hurldmg, sizeof(o->hurldmg));
    rs_read_int(inf, &o->count);
    rs_read_int(inf, &o->which);
    rs_read_int(inf, &o->hplus);
    rs_read_int(inf, &o->dplus);
    rs_read_int(inf, &o->arm);
    rs_read_int(inf, &o->flags);
    rs_read_int(inf, &o->group);
    rs_read_new_string(inf, &o->label);
    
    return(READSTAT);
}

static int rs_write_object_list(FILE *savef, std::list<ItemThing*>& list)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_OBJECTLIST);
    rs_write_int(savef, list.size());

    for(ItemThing* l : list)
        rs_write_object(savef, l);
    
    return(WRITESTAT);
}

static int rs_read_object_list(FILE *inf, std::list<ItemThing*>& list)
{
    int i, cnt;
    ItemThing *l = NULL;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_OBJECTLIST);
    rs_read_int(inf, &cnt);

    for (i = 0; i < cnt; i++) 
    {
        l = new ItemThing();
        list.push_back(l);

        rs_read_object(inf,l);
    }

    return(READSTAT);
}

static int rs_write_object_reference(FILE *savef, std::list<ItemThing*>& list, ItemThing *item)
{
    int i;
    
    if (write_error)
        return(WRITESTAT);

    i = find_list_ptr(list, item);

    rs_write_int(savef, i);

    return(WRITESTAT);
}

static int rs_read_object_reference(FILE *inf, std::list<ItemThing*>& list, ItemThing **item)
{
    int i;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &i);

    *item = get_list_item(list, i);
            
    return(READSTAT);
}

static int find_monster_index_for_coord(coord *c)
{
    int i = 0;

    for(MonsterThing* tp : mlist)
    {
        if (c == &tp->pos)
            return(i);

        i++;
    }

    return(-1);
}

static int find_object_coord(coord *c)
{
    int i = 0;

    for(ItemThing* obj : lvl_obj)
    {
        if (c == &obj->pos)
            return(i);

        i++;
    }

    return(-1);
}

static int rs_write_thing(FILE *savef, MonsterThing *t)
{
    int i = -1;
    
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_THING);

    if (t == NULL)
    {
        rs_write_int(savef, 0);
        return(WRITESTAT);
    }
    
    rs_write_int(savef, 1);
    rs_write_coord(savef, t->pos);
    rs_write_boolean(savef, t->turn);
    rs_write_char(savef, t->type);
    rs_write_char(savef, t->disguise);

    /* 
        t_dest can be:
        0,0: NULL
        0,1: location of hero
        1,i: location of a thing (monster)
        2,i: location of an object
        3,i: location of gold in a room

        We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */

    if (t->dest == &hero)
    {
        rs_write_int(savef,0);
        rs_write_int(savef,1);
    }
    else if (t->dest != NULL)
    {
        i = find_monster_index_for_coord(t->dest);
            
        if (i >=0 )
        {
            rs_write_int(savef,1);
            rs_write_int(savef,i);
        }
        else
        {
            i = find_object_coord(t->dest);
            
            if (i >= 0)
            {
                rs_write_int(savef,2);
                rs_write_int(savef,i);
            }
            else
            {
                rs_write_int(savef, 0);
                rs_write_int(savef, 1); /* chase the hero anyway */
            }
        }
    }
    else
    {
        rs_write_int(savef,0);
        rs_write_int(savef,0);
    }
    
    rs_write_short(savef, t->flags);
    rs_write_stats(savef, &t->stats);
    rs_write_object_list(savef, t->pack);
    
    return(WRITESTAT);
}

int
rs_read_thing(FILE *inf, MonsterThing *t)
{
    int listid = 0, index = -1;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_THING);

    rs_read_int(inf, &index);

    if (index == 0)
        return(READSTAT);

    rs_read_coord(inf,&t->pos);
    rs_read_boolean(inf,&t->turn);
    rs_read_char(inf,&t->type);
    rs_read_char(inf,&t->disguise);
            
    /* 
        t_dest can be (listid,index):
        0,0: NULL
        0,1: location of hero
        1,i: location of a thing (monster)
        2,i: location of an object
        3,i: location of gold in a room

        We need to remember what we are chasing rather than 
        the current location of what we are chasing.
    */
            
    rs_read_int(inf, &listid);
    rs_read_int(inf, &index);
    t->reserved = -1;

    if (listid == 0) /* hero or NULL */
    {
        if (index == 1)
            t->dest = &hero;
        else
            t->dest = NULL;
    }
    else if (listid == 1) /* monster/thing */
    {
        t->dest     = NULL;
        t->reserved = index;
    }
    else if (listid == 2) /* object */
    {
        ItemThing *obj;

        obj = get_list_item(lvl_obj, index);

        if (obj != NULL)
        {
            t->dest = &obj->pos;
        }
    }
    else
        t->dest = NULL;
            
    rs_read_short(inf,&t->flags);
    rs_read_stats(inf,&t->stats);
    rs_read_object_list(inf, t->pack);
    
    return(READSTAT);
}

void
rs_fix_monster(MonsterThing *t)
{
    MonsterThing *item;

    if (t->reserved < 0)
        return;

    auto it = mlist.begin();
    for(int n=0; n<t->reserved; n++)
        it++;
    item = *it;

    if (item != NULL)
    {
        t->dest = &item->pos;
    }
}

int
rs_write_monster_list(FILE *savef)
{
    int cnt = 0;
    
    if (write_error)
        return(WRITESTAT);

    rs_write_marker(savef, RSID_MONSTERLIST);

    cnt = mlist.size();

    rs_write_int(savef, cnt);

    if (cnt < 1)
        return(WRITESTAT);

    for(MonsterThing* l : mlist)
        rs_write_thing(savef, l);
    
    return(WRITESTAT);
}

int rs_read_monster_list(FILE *inf)
{
    int i, cnt;
    MonsterThing *l = NULL;

    if (read_error || format_error)
        return(READSTAT);

    rs_read_marker(inf, RSID_MONSTERLIST);

    rs_read_int(inf, &cnt);

    for (i = 0; i < cnt; i++) 
    {
        l = new MonsterThing();
        mlist.push_back(l);
        rs_read_thing(inf,l);
    }
        
    return(READSTAT);
}

void rs_fix_monster_list()
{
    for(MonsterThing *item : mlist)
        rs_fix_monster(item);
}

static int rs_write_monster_reference(FILE *savef, MonsterThing *item)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    i = 0;
    for(MonsterThing* mt : mlist)
    {
        if (item == mt)
        {
            rs_write_int(savef, i);
            return(WRITESTAT);
        }
        i++;
    }
    rs_write_int(savef,-1);

    return(WRITESTAT);
}

static int rs_read_monster_reference(FILE *inf, MonsterThing **item)
{
    int i;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &i);

    if (i == -1)
    {
        *item = NULL;
    }else{
        auto it = mlist.begin();
        while(i > 0)
        {
            it++;
            i--;
        }
        *item = *it;
    }

    return(READSTAT);
}

static int rs_write_item_reference(FILE *savef, ItemThing* item)
{
    int i;

    if (write_error)
        return(WRITESTAT);

    i = 0;
    for(ItemThing* it : lvl_obj)
    {
        if (item == it)
        {
            rs_write_int(savef, i);
            return(WRITESTAT);
        }
        i++;
    }
    rs_write_int(savef,-1);

    return(WRITESTAT);
}

static int rs_read_item_reference(FILE *inf, ItemThing** item)
{
    int i;
    
    if (read_error || format_error)
        return(READSTAT);

    rs_read_int(inf, &i);

    if (i == -1)
    {
        *item = NULL;
    }else{
        auto it = lvl_obj.begin();
        while(i > 0)
        {
            it++;
            i--;
        }
        *item = *it;
    }

    return(READSTAT);
}

static int rs_write_places(FILE *savef, PLACE *places, int count)
{
    int i = 0;
    
    if (write_error)
        return(WRITESTAT);

    for(i = 0; i < count; i++) 
    {
        rs_write_char(savef, places[i].p_ch);
        rs_write_int(savef, places[i].p_flags);
        rs_write_monster_reference(savef, places[i].p_monst);
        rs_write_item_reference(savef, places[i].p_item);
    }

    return(WRITESTAT);
}

static int rs_read_places(FILE *inf, PLACE *places, int count)
{
    int i = 0;
    
    if (read_error || format_error)
        return(READSTAT);

    for(i = 0; i < count; i++) 
    {
        rs_read_char(inf,&places[i].p_ch);
        rs_read_int(inf,&places[i].p_flags);
        rs_read_monster_reference(inf, &places[i].p_monst);
        rs_read_item_reference(inf, &places[i].p_item);
    }

    return(READSTAT);
}

int
rs_save_file(FILE *savef)
{
    if (write_error)
        return(WRITESTAT);

    rs_write_boolean(savef, after);                 /* 1  */    /* extern.c */
    rs_write_boolean(savef, again);                 /* 2  */
    rs_write_boolean(savef, seenstairs);            /* 4  */
    rs_write_boolean(savef, amulet);                /* 5  */
    rs_write_boolean(savef, door_stop);             /* 6  */
    rs_write_boolean(savef, fight_flush);           /* 7  */
    rs_write_boolean(savef, firstmove);             /* 8  */
    rs_write_boolean(savef, has_hit);               /* 10 */
    rs_write_boolean(savef, inv_describe);          /* 12 */
    rs_write_boolean(savef, jump);                  /* 13 */
    rs_write_boolean(savef, kamikaze);              /* 14 */
    rs_write_boolean(savef, lower_msg);             /* 15 */
    rs_write_boolean(savef, move_on);               /* 16 */
    rs_write_boolean(savef, passgo);                /* 18 */
    rs_write_boolean(savef, playing);               /* 19 */
    rs_write_boolean(savef, running);               /* 21 */
    rs_write_boolean(savef, save_msg);              /* 22 */
    rs_write_boolean(savef, stat_msg);              /* 24 */
    rs_write_boolean(savef, terse);                 /* 25 */
    rs_write_boolean(savef, to_death);              /* 26 */
    rs_write_boolean(savef, tombstone);             /* 27 */
    rs_write_booleans(savef, pack_used, 26);        /* 29 */
    rs_write_int(savef, dir_ch);
    rs_write_chars(savef, file_name, MAXSTR);
    rs_write_chars(savef, huh, MAXSTR);
    rs_write_potions(savef);
    rs_write_chars(savef,prbuf,2*MAXSTR);
    rs_write_rings(savef);
    rs_write_int(savef, runch);
    rs_write_scrolls(savef);
    rs_write_char(savef, take);
    rs_write_sticks(savef);
    rs_write_int(savef,orig_dsusp);
    rs_write_chars(savef, fruit, MAXSTR);
    rs_write_int(savef,l_last_comm);
    rs_write_int(savef,l_last_dir);
    rs_write_int(savef,last_comm);
    rs_write_int(savef,last_dir);
    rs_write_int(savef,n_objs);
    rs_write_int(savef, hungry_state);
    rs_write_int(savef, inpack);
    rs_write_int(savef, inv_type);
    rs_write_int(savef, level);
    rs_write_int(savef, max_level);
    rs_write_int(savef, no_food);
    rs_write_ints(savef,a_class,MAXARMORS);
    rs_write_int(savef, count);
    rs_write_int(savef, food_left);
    rs_write_int(savef, lastscore);
    rs_write_int(savef, no_command);
    rs_write_int(savef, no_move);
    rs_write_int(savef, purse);
    rs_write_int(savef, quiet);
    rs_write_int(savef, vf_hit);
    rs_write_int(savef, dnum);
    rs_write_ints(savef, e_levels, 21);
    rs_write_coord(savef, delta);
    rs_write_coord(savef, oldpos);
    rs_write_coord(savef, stairs);

    rs_write_thing(savef, &player);                     
    rs_write_object_reference(savef, player.pack, cur_armor);
    rs_write_object_reference(savef, player.pack, cur_ring[0]);
    rs_write_object_reference(savef, player.pack, cur_ring[1]); 
    rs_write_object_reference(savef, player.pack, cur_weapon); 
    rs_write_object_reference(savef, player.pack, l_last_pick); 
    rs_write_object_reference(savef, player.pack, last_pick); 
    
    rs_write_object_list(savef, lvl_obj);               
    rs_write_monster_list(savef);

    rs_write_places(savef,places,MAXLINES*MAXCOLS);

    rs_write_stats(savef,&max_stats); 
    rs_write_areas(savef);

    rs_write_monsters(savef,monsters,26);               
    rs_write_obj_info(savef, things,   NUMITEMTYPES);   
    rs_write_obj_info(savef, arm_info,  MAXARMORS);  
    rs_write_obj_info(savef, pot_info,  MAXPOTIONS);  
    rs_write_obj_info(savef, ring_info,  MAXRINGS);    
    rs_write_obj_info(savef, scr_info,  MAXSCROLLS);  
    rs_write_obj_info(savef, weap_info,  MAXWEAPONS+1);  
    rs_write_obj_info(savef, ws_info, MAXSTICKS);      
    
    
    rs_write_daemons(savef, &d_list[0], 20);            /* 5.4-daemon.c */
    rs_write_int(savef,between);                        /* 5.4-daemons.c*/
    rs_write_coord(savef, nh);                          /* 5.4-move.c    */
    rs_write_int(savef, group);                         /* 5.4-weapons.c */

    rs_write_window(savef);

    return(WRITESTAT);
}

int
rs_restore_file(FILE *inf)
{
    if (read_error || format_error)
        return(READSTAT);

    rs_read_boolean(inf, &after);               /* 1  */    /* extern.c */
    rs_read_boolean(inf, &again);               /* 2  */
    rs_read_boolean(inf, &seenstairs);          /* 4  */
    rs_read_boolean(inf, &amulet);              /* 5  */
    rs_read_boolean(inf, &door_stop);           /* 6  */
    rs_read_boolean(inf, &fight_flush);         /* 7  */
    rs_read_boolean(inf, &firstmove);           /* 8  */
    rs_read_boolean(inf, &has_hit);             /* 10 */
    rs_read_boolean(inf, &inv_describe);        /* 12 */
    rs_read_boolean(inf, &jump);                /* 13 */
    rs_read_boolean(inf, &kamikaze);            /* 14 */
    rs_read_boolean(inf, &lower_msg);           /* 15 */
    rs_read_boolean(inf, &move_on);             /* 16 */
    rs_read_boolean(inf, &passgo);              /* 18 */
    rs_read_boolean(inf, &playing);             /* 19 */
    rs_read_boolean(inf, &running);             /* 21 */
    rs_read_boolean(inf, &save_msg);            /* 22 */
    rs_read_boolean(inf, &stat_msg);            /* 24 */
    rs_read_boolean(inf, &terse);               /* 25 */
    rs_read_boolean(inf, &to_death);            /* 26 */
    rs_read_boolean(inf, &tombstone);           /* 27 */
    rs_read_booleans(inf, pack_used, 26);       /* 29 */
    rs_read_int(inf, &dir_ch);
    rs_read_chars(inf, file_name, MAXSTR);
    rs_read_chars(inf, huh, MAXSTR);
    rs_read_potions(inf);
    rs_read_chars(inf, prbuf, 2*MAXSTR);
    rs_read_rings(inf);
    rs_read_int(inf, &runch);
    rs_read_scrolls(inf);
    rs_read_char(inf, &take);
    rs_read_sticks(inf);
    rs_read_int(inf,&orig_dsusp);
    rs_read_chars(inf, fruit, MAXSTR);
    rs_read_int(inf, &l_last_comm);
    rs_read_int(inf, &l_last_dir);
    rs_read_int(inf, &last_comm);
    rs_read_int(inf, &last_dir);
    rs_read_int(inf, &n_objs);
    rs_read_int(inf, &hungry_state);
    rs_read_int(inf, &inpack);
    rs_read_int(inf, &inv_type);
    rs_read_int(inf, &level);
    rs_read_int(inf, &max_level);
    rs_read_int(inf, &no_food);
    rs_read_ints(inf,a_class,MAXARMORS);
    rs_read_int(inf, &count);
    rs_read_int(inf, &food_left);
    rs_read_int(inf, &lastscore);
    rs_read_int(inf, &no_command);
    rs_read_int(inf, &no_move);
    rs_read_int(inf, &purse);
    rs_read_int(inf, &quiet);
    rs_read_int(inf, &vf_hit);
    rs_read_int(inf, &dnum);
    rs_read_ints(inf,e_levels,21);
    rs_read_coord(inf, &delta);
    rs_read_coord(inf, &oldpos);
    rs_read_coord(inf, &stairs);

    rs_read_thing(inf, &player); 
    rs_read_object_reference(inf, player.pack, &cur_armor);
    rs_read_object_reference(inf, player.pack, &cur_ring[0]);
    rs_read_object_reference(inf, player.pack, &cur_ring[1]);
    rs_read_object_reference(inf, player.pack, &cur_weapon);
    rs_read_object_reference(inf, player.pack, &l_last_pick);
    rs_read_object_reference(inf, player.pack, &last_pick);

    rs_read_object_list(inf, lvl_obj);
    rs_read_monster_list(inf);                  
    rs_fix_monster(&player);
    rs_fix_monster_list();

    rs_read_places(inf,places,MAXLINES*MAXCOLS);

    rs_read_stats(inf, &max_stats);
    rs_read_areas(inf);

    rs_read_monsters(inf,monsters,26);                  
    rs_read_obj_info(inf, things,   NUMITEMTYPES);         
    rs_read_obj_info(inf, arm_info,   MAXARMORS);         
    rs_read_obj_info(inf, pot_info,  MAXPOTIONS);       
    rs_read_obj_info(inf, ring_info,  MAXRINGS);         
    rs_read_obj_info(inf, scr_info,  MAXSCROLLS);       
    rs_read_obj_info(inf, weap_info, MAXWEAPONS+1);       
    rs_read_obj_info(inf, ws_info, MAXSTICKS);       

    rs_read_daemons(inf, d_list, 20);                   /* 5.4-daemon.c     */
    rs_read_int(inf,&between);                          /* 5.4-daemons.c    */
    rs_read_coord(inf, &nh);                            /* 5.4-move.c       */
    rs_read_int(inf,&group);                            /* 5.4-weapons.c    */
    
    rs_read_window(inf);

    return(READSTAT);
}
