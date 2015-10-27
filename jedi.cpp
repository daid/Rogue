#include <random>
#include <time.h>
#include <string.h>
#ifdef USE_SDL
#include <SDL/SDL.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "jedi.h"
#include "extern.h"
#include "rogue.h"

extern uint8_t font_data[];

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
uint8_t jedi_screen_buffer[DISPLAY_WIDTH*DISPLAY_HEIGHT];

static std::mt19937_64 random_engine;

static char status_line[40];
static int map_view_x;
static int map_view_y;
static int map_buffer[NUMCOLS*NUMLINES];

static void drawChar(int x, int y, int c)
{
    bool inverted = (c & DISPLAY_INVERT);
    c &=~DISPLAY_INVERT;
    
    for(int _y=0; _y<6; _y++)
    {
        uint8_t row = font_data[c*6+_y];
        if (inverted)
            row = ~row;
        for(int _x=0; _x<4; _x++)
        {
            jedi_screen_buffer[x+_x + (y+_y) * DISPLAY_WIDTH] = (row & (0b1000 >> _x)) ? 255 : 0;
        }
    }
}

static void fillBox(int x, int y, int w, int h, uint8_t color)
{
    for(int _x=0; _x<w; _x++)
    {
        for(int _y=0; _y<h; _y++)
        {
            jedi_screen_buffer[x+_x + (y+_y) * DISPLAY_WIDTH] = color;
        }
    }
}

static void drawMore(int y)
{
    int x = 90;
    for(const char* c="  MORE  "; *c; c++)
    {
        drawChar(x, y, *c | DISPLAY_INVERT);
        x += 4;
    }
}

static void refreshDisplay()
{
#ifdef USE_SDL
    SDL_Surface* screen = SDL_GetVideoSurface();
    for(int x=0; x<DISPLAY_WIDTH; x++)
    {
        for(int y=0; y<DISPLAY_HEIGHT; y++)
        {
            int color = jedi_screen_buffer[x + y * DISPLAY_WIDTH] > 128 ? 0x8080AF : 0x202020;
            SDL_Rect rect = {Sint16(x * 5), Sint16(y * 5), 4, 4};
            SDL_FillRect(screen, &rect, color);
        }
    }
    SDL_Flip(screen);
#else
    static int surface_handle = -1;
    static uint8_t* surface_buffer;
    if (surface_handle == -1)
    {
        surface_handle = open("/dev/fb0", O_RDWR);
        surface_buffer = (uint8_t*)mmap(NULL, DISPLAY_WIDTH * DISPLAY_HEIGHT, PROT_READ | PROT_WRITE, MAP_SHARED, surface_handle, 0);
    }

    uint8_t* src = jedi_screen_buffer;
    uint8_t* dst = surface_buffer;
    for(int y=0; y<DISPLAY_HEIGHT; y++)
    {
        for(int x=0; x<DISPLAY_WIDTH/8; x++)
        {
            uint8_t b = 0;
            if (*src++ > 128) b |= 0x01;
            if (*src++ > 128) b |= 0x02;
            if (*src++ > 128) b |= 0x04;
            if (*src++ > 128) b |= 0x08;
            if (*src++ > 128) b |= 0x10;
            if (*src++ > 128) b |= 0x20;
            if (*src++ > 128) b |= 0x40;
            if (*src++ > 128) b |= 0x80;
            *dst++ = b;
        }
    }
#endif
}

static void drawMap()
{
    for(int x=0; x<DISPLAY_WIDTH / 4; x++)
    {
        for(int y=0; y<DISPLAY_HEIGHT / 6; y++)
        {
            int n = 0;
            if ((x + map_view_x) >= 0 && (x + map_view_x) < NUMCOLS && (y + map_view_y) >= 0 && (y + map_view_y) < NUMLINES)
                n = map_buffer[(x + map_view_x) + (y + map_view_y) * NUMCOLS];

            drawChar(x*4, y*6, n);
        }
    }
    fillBox(0, DISPLAY_HEIGHT-6, DISPLAY_WIDTH, 6, 255);
    for(char* c=status_line; *c; c++)
    {
        drawChar((c-status_line)*4, DISPLAY_HEIGHT-6, (*c) | DISPLAY_INVERT);
    }
}

void initJedi()
{
    clearMapDisplay();
    memset(jedi_screen_buffer, 0, DISPLAY_WIDTH*DISPLAY_HEIGHT);
    
    random_engine.seed(time(NULL));
}

int getRandomNumber(int max)
{
    return std::uniform_int_distribution<>(0, max)(random_engine);
}

void clearMapDisplay()
{
    memset(map_buffer, 0, sizeof(int) * NUMCOLS*NUMLINES);
}

void setMapDisplay(int x, int y, int display_char)
{
    map_buffer[x + y * NUMCOLS] = display_char;
}

int getMapDisplay(int x, int y)
{
    return map_buffer[x + y * NUMCOLS];
}

void refreshMapWithMore()
{
    drawMap();
    drawMore(DISPLAY_HEIGHT - 12);
    refreshDisplay();
    md_readchar();
    refreshMap();
}

void refreshMap()
{
    drawMap();
    refreshDisplay();
}

void animationDelay()
{
#ifdef USE_SDL
    SDL_Delay(30);
#else
    usleep(30 * 1000);
#endif
}

void displayLargeMap()
{
    memset(jedi_screen_buffer, 0, DISPLAY_WIDTH*DISPLAY_HEIGHT);
    for(int y=0; y<NUMLINES; y++)
    {
        for(int x=0; x<NUMCOLS; x++)
        {
            int c = map_buffer[x + y * NUMCOLS];
            if (c && c != ' ' && c != '.')
            {
                jedi_screen_buffer[x + (y * 2 + 1) * DISPLAY_WIDTH] = 255;
                if (IS_WALL(c))
                    jedi_screen_buffer[x + (y * 2) * DISPLAY_WIDTH] = 255;
            }
        }
    }
    refreshDisplay();
    md_readchar();
    refreshMap();
}

void setMapViewTarget(int x, int y)
{
    map_view_x = x - DISPLAY_WIDTH / 2 / 4;
    map_view_y = y - DISPLAY_HEIGHT / 2 / 6;
}

void setStatusLine(const char* buffer)
{
    strcpy(status_line, buffer);
}

void lineWrapString(char* buffer, int max_width, const char* newline_prefix, int* actual_width, int* line_count)
{
    bool first_word = true;
    int word_start_pos = 0;
    int line_width = 0;
    if (line_count)
        *line_count = 1;
    for(unsigned int pos=0; pos<strlen(buffer); pos++)
    {
        if (buffer[pos] == ' ')
        {
            word_start_pos = pos + 1;
            first_word = false;
        }
        if (buffer[pos] == '\n')
        {
            word_start_pos = pos + 1;
            first_word = true;
            line_width = -1;
            if (line_count)
                (*line_count)++;
        }
        line_width ++;
        if (line_width > max_width)
        {
            if (first_word)
            {
                if (line_count)
                    (*line_count)++;
                memmove(&buffer[pos+1+strlen(newline_prefix)], &buffer[pos], strlen(&buffer[pos])+1);
                buffer[pos] = '\n';
                memcpy(&buffer[pos+1], newline_prefix, strlen(newline_prefix));
                first_word = true;
                line_width = 0;
            }else{
                if (line_count)
                    (*line_count)++;
                memmove(&buffer[word_start_pos+strlen(newline_prefix)], &buffer[word_start_pos], strlen(&buffer[word_start_pos])+1);
                buffer[word_start_pos - 1] = '\n';
                memcpy(&buffer[word_start_pos], newline_prefix, strlen(newline_prefix));
                first_word = true;
                line_width = (pos - word_start_pos) + 1;
            }
        }
    }

    if (actual_width)
    {
        *actual_width = 0;
        line_width = 0;
        for(unsigned int pos=0; pos<strlen(buffer); pos++)
        {
            if (buffer[pos] == '\n')
            {
                if (line_width > *actual_width)
                    *actual_width = line_width;
                line_width = 0;
            }else{
                line_width ++;
            }
        }
        if (line_width > *actual_width)
            *actual_width = line_width;
    }
}

int displayMessage(const char* msg_buffer)
{
    char buffer[strlen(msg_buffer) + 32];
    strcpy(buffer, msg_buffer);
    
    int max_line_width = (DISPLAY_WIDTH * 2 / 3) / 4;
    int width, line_count;
    lineWrapString(buffer, max_line_width, "", &width, &line_count);
    
    drawMap();
    int x = (DISPLAY_WIDTH - width * 4) / 2;
    int y = (DISPLAY_HEIGHT - line_count * 6) / 2;
    int sx = x;
    fillBox(x - 2, y - 2, width * 4 + 4, line_count * 6 + 5, 255);
    fillBox(x - 1, y - 1, width * 4 + 2, line_count * 6 + 2, 0);
    for(const char* c=buffer; *c; c++)
    {
        if (*c == '\n')
        {
            x = sx;
            y += 6;
        }else{
            drawChar(x, y, *c);
            x += 4;
        }
    }
    
    refreshDisplay();
    int ch = md_readchar();
    refreshMap();
    return ch;
}

int askForInput(const char* message, char* input_buffer, int input_buffer_size)
{
    memset(input_buffer, 0, input_buffer_size);
    
    int w = strlen(message) * 4;
    int x = (DISPLAY_WIDTH - w) / 2;
    int y = (DISPLAY_HEIGHT - 12) / 2;
    fillBox(x - 2, y - 2, w + 4, 2 * 6 + 5, 255);
    fillBox(x - 1, y - 1, w + 2, 2 * 6 + 2, 0);
    for(const char* c=message; *c; c++)
    {
        drawChar(x, y, *c);
        x += 4;
    }
    y += 6;
    while(true)
    {
        x = (DISPLAY_WIDTH - w) / 2;
        for(const char* c=input_buffer; *c; c++)
        {
            drawChar(x, y, *c);
            x += 4;
        }
        drawChar(x, y, '_');
        refreshDisplay();

        int ch = md_readchar();
        if (ch == 27)
            return QUIT;
        if (ch == 13)
            return NORM;
        if (ch == 8 && strlen(input_buffer) > 0)
        {
            input_buffer[strlen(input_buffer) - 1] = '\0';
            drawChar(x, y, ' ');
        }
        if (ch >= ' ' && ch <= '~')
            input_buffer[strlen(input_buffer)] = ch;
    }
}

const char* getKeyName(int key)
{
    static char buffer[16];
    strcpy(buffer, "[?]");
    if ((key >= ' ' && key <= '~'))
        sprintf(buffer, "%c", key);
    if (key == 0)
        return "";
    if (key == 27)
        return "ESC";
    if (key == K_UP_LEFT)
        return "UpLeft";
    if (key == K_UP)
        return "Up";
    if (key == K_UP_RIGHT)
        return "UpRight";
    if (key == K_LEFT)
        return "Left";
    if (key == K_RIGHT)
        return "Right";
    if (key == K_DOWN_LEFT)
        return "DownLeft";
    if (key == K_DOWN)
        return "Down";
    if (key == K_DOWN_RIGHT)
        return "DownRight";
    return buffer;
}

int display_string_list_line_count;
char display_string_list_buffer[1024];

static int drawDisplayStringList()
{
    drawMap();
    int w = 30 * 4;
    int x = 5;
    int sx = x;
    int y = (DISPLAY_HEIGHT-display_string_list_line_count*6) / 2;
    fillBox(x - 2, y - 2, w + 4, display_string_list_line_count * 6 + 5, 255);
    fillBox(x - 1, y - 1, w + 2, display_string_list_line_count * 6 + 2, 0);
    for(char* c=display_string_list_buffer; *c; c++)
    {
        if (*c == '\n')
        {
            x = sx;
            y += 6;
        }else{
            drawChar(x, y, *c);
            x+=4;
        }
    }
    return y;
}

void startDisplayOfStringList()
{
    strcpy(display_string_list_buffer, "");
    display_string_list_line_count = 0;
}

int displayStringListItem(const char* fmt, ...)
{
    char buffer[128];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);
    
    int width, line_count;
    lineWrapString(buffer, 30, "   ", &width, &line_count);
    
    int ch = 0;

    if (display_string_list_line_count + line_count > 7)
    {
        drawMore(drawDisplayStringList() + 5);
        refreshDisplay();
        ch = md_readchar();
        refreshMap();
        
        display_string_list_line_count = 0;
        strcpy(display_string_list_buffer, "");
    }
    
    if (display_string_list_line_count > 0)
        strcat(display_string_list_buffer, "\n");
    strcat(display_string_list_buffer, buffer);
    display_string_list_line_count += line_count;
    
    return ch;
}

int finishDisplayOfStringList()
{
    int ch = 0;
    if (display_string_list_line_count > 0)
    {
        drawDisplayStringList();
        
        refreshDisplay();
        ch = md_readchar();
        refreshMap();
    }
    return ch;
}

uint8_t font_data[] = {
//[0]
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
//[1]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[2] (PASSAGE)
  0b0101, 
  0b1010, 
  0b0101, 
  0b1010, 
  0b0101, 
  0b1010, 
//[3] (ARMOR)
  0b1110, 
  0b1110, 
  0b1010, 
  0b1110, 
  0b1110, 
  0b0000, 
//[4] (POTION)
  0b0100, 
  0b0000, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b0000, 
//[5] (TRAP)
  0b0100, 
  0b1110, 
  0b1110, 
  0b1110, 
  0b0100, 
  0b0000, 
//[6] (RING)
  0b0000, 
  0b0000, 
  0b0100, 
  0b1010, 
  0b0100, 
  0b0000, 
//[7] (STAIRS)
  0b0000, 
  0b0001, 
  0b0011, 
  0b0111, 
  0b1111, 
  0b0000,
//[8] SCROLL
  0b0111, 
  0b0110, 
  0b0110, 
  0b0110, 
  0b1110, 
  0b0000, 
//[9] FOOD
  0b0000, 
  0b0000, 
  0b0100, 
  0b1010, 
  0b1110, 
  0b0000, 
//[10]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[11]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[12]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[13]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[14]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[15]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[16] WALL_V
  0b1010, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1010, 
//[17] WALL_H
  0b0000, 
  0b1111, 
  0b0000, 
  0b1111, 
  0b0000, 
  0b0000, 
//[18] WALL_TL
  0b0000, 
  0b1111, 
  0b1000, 
  0b1011, 
  0b1010, 
  0b1010, 
//[19] WALL_TR
  0b0000, 
  0b1110, 
  0b0010, 
  0b1010, 
  0b1010, 
  0b1010, 
//[20] WALL_BL
  0b1010, 
  0b1011, 
  0b1000, 
  0b1111, 
  0b0000, 
  0b0000, 
//[21] WALL_BR
  0b1010, 
  0b1010, 
  0b0010, 
  0b1110, 
  0b0000, 
  0b0000, 
//[22] DOOR
  0b1010, 
  0b1011, 
  0b0000, 
  0b1011, 
  0b1010, 
  0b1010, 
//[23]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[24]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[25]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[26]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[27]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[28]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[29]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[30]
  0b1110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0000, 
//[31]
  0b1110, 
  0b1110, 
  0b1110, 
  0b1110, 
  0b1110, 
  0b0000, 
// 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
//!
  0b0100, 
  0b0100, 
  0b0100, 
  0b0000, 
  0b0100, 
  0b0000, 
//"
  0b1010, 
  0b1010, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
//#
  0b1010, 
  0b1110, 
  0b1010, 
  0b1110, 
  0b1010, 
  0b0000, 
//$
  0b0110, 
  0b1100, 
  0b0110, 
  0b1100, 
  0b0100, 
  0b0000, 
//%
  0b1000, 
  0b0010, 
  0b0100, 
  0b1000, 
  0b0010, 
  0b0000, 
//&
  0b1100, 
  0b1100, 
  0b1110, 
  0b1010, 
  0b0110, 
  0b0000, 
//'
  0b0100, 
  0b0100, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
//(
  0b0010, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b0010, 
  0b0000, 
//)
  0b1000, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b1000, 
  0b0000, 
//*
  0b1010, 
  0b0100, 
  0b1110, 
  0b0100, 
  0b1010, 
  0b0000, 
//+
  0b0000, 
  0b0100, 
  0b1110, 
  0b0100, 
  0b0000, 
  0b0000, 
//,
  0b0000, 
  0b0000, 
  0b0000, 
  0b0100, 
  0b1000, 
  0b0000, 
//-
  0b0000, 
  0b0000, 
  0b1110, 
  0b0000, 
  0b0000, 
  0b0000, 
//.
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0100, 
  0b0000, 
///
  0b0010, 
  0b0010, 
  0b0100, 
  0b1000, 
  0b1000, 
  0b0000, 
//0
  0b0110, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1100, 
  0b0000, 
//1
  0b0100, 
  0b1100, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b0000, 
//2
  0b1100, 
  0b0010, 
  0b0100, 
  0b1000, 
  0b1110, 
  0b0000, 
//3
  0b1100, 
  0b0010, 
  0b0100, 
  0b0010, 
  0b1100, 
  0b0000, 
//4
  0b1010, 
  0b1010, 
  0b1110, 
  0b0010, 
  0b0010, 
  0b0000, 
//5
  0b1110, 
  0b1000, 
  0b1100, 
  0b0010, 
  0b1100, 
  0b0000, 
//6
  0b0110, 
  0b1000, 
  0b1110, 
  0b1010, 
  0b1110, 
  0b0000, 
//7
  0b1110, 
  0b0010, 
  0b0100, 
  0b1000, 
  0b1000, 
  0b0000, 
//8
  0b1110, 
  0b1010, 
  0b1110, 
  0b1010, 
  0b1110, 
  0b0000, 
//9
  0b1110, 
  0b1010, 
  0b1110, 
  0b0010, 
  0b1100, 
  0b0000, 
//:
  0b0000, 
  0b0100, 
  0b0000, 
  0b0100, 
  0b0000, 
  0b0000, 
//;
  0b0000, 
  0b0100, 
  0b0000, 
  0b0100, 
  0b1000, 
  0b0000, 
//<
  0b0010, 
  0b0100, 
  0b1000, 
  0b0100, 
  0b0010, 
  0b0000, 
//=
  0b0000, 
  0b1110, 
  0b0000, 
  0b1110, 
  0b0000, 
  0b0000, 
//>
  0b1000, 
  0b0100, 
  0b0010, 
  0b0100, 
  0b1000, 
  0b0000, 
//?
  0b1110, 
  0b0010, 
  0b0100, 
  0b0000, 
  0b0100, 
  0b0000, 
//@
  0b0100, 
  0b1010, 
  0b1110, 
  0b1000, 
  0b0110, 
  0b0000, 
//A
  0b0100, 
  0b1010, 
  0b1110, 
  0b1010, 
  0b1010, 
  0b0000, 
//B
  0b1100, 
  0b1010, 
  0b1100, 
  0b1010, 
  0b1100, 
  0b0000, 
//C
  0b0110, 
  0b1000, 
  0b1000, 
  0b1000, 
  0b0110, 
  0b0000, 
//D
  0b1100, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b1100, 
  0b0000, 
//E
  0b1110, 
  0b1000, 
  0b1110, 
  0b1000, 
  0b1110, 
  0b0000, 
//F
  0b1110, 
  0b1000, 
  0b1110, 
  0b1000, 
  0b1000, 
  0b0000, 
//G
  0b0110, 
  0b1000, 
  0b1110, 
  0b1010, 
  0b0110, 
  0b0000, 
//H
  0b1010, 
  0b1010, 
  0b1110, 
  0b1010, 
  0b1010, 
  0b0000, 
//I
  0b1110, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b1110, 
  0b0000, 
//J
  0b0010, 
  0b0010, 
  0b0010, 
  0b1010, 
  0b0100, 
  0b0000, 
//K
  0b1010, 
  0b1010, 
  0b1100, 
  0b1010, 
  0b1010, 
  0b0000, 
//L
  0b1000, 
  0b1000, 
  0b1000, 
  0b1000, 
  0b1110, 
  0b0000, 
//M
  0b1010, 
  0b1110, 
  0b1110, 
  0b1010, 
  0b1010, 
  0b0000, 
//N
  0b1010, 
  0b1110, 
  0b1110, 
  0b1110, 
  0b1010, 
  0b0000, 
//O
  0b0100, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b0100, 
  0b0000, 
//P
  0b1100, 
  0b1010, 
  0b1100, 
  0b1000, 
  0b1000, 
  0b0000, 
//Q
  0b0100, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0110, 
  0b0000, 
//R
  0b1100, 
  0b1010, 
  0b1110, 
  0b1100, 
  0b1010, 
  0b0000, 
//S
  0b0110, 
  0b1000, 
  0b0100, 
  0b0010, 
  0b1100, 
  0b0000, 
//T
  0b1110, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b0000, 
//U
  0b1010, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b0110, 
  0b0000, 
//V
  0b1010, 
  0b1010, 
  0b1010, 
  0b0100, 
  0b0100, 
  0b0000, 
//W
  0b1010, 
  0b1010, 
  0b1110, 
  0b1110, 
  0b1010, 
  0b0000, 
//X
  0b1010, 
  0b1010, 
  0b0100, 
  0b1010, 
  0b1010, 
  0b0000, 
//Y
  0b1010, 
  0b1010, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b0000, 
//Z
  0b1110, 
  0b0010, 
  0b0100, 
  0b1000, 
  0b1110, 
  0b0000, 
//[
  0b1110, 
  0b1000, 
  0b1000, 
  0b1000, 
  0b1110, 
  0b0000, 
//
  0b0000, 
  0b1000, 
  0b0100, 
  0b0010, 
  0b0000, 
  0b0000, 
//]
  0b1110, 
  0b0010, 
  0b0010, 
  0b0010, 
  0b1110, 
  0b0000, 
//^
  0b0100, 
  0b1010, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
//_
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b1110, 
  0b0000, 
//`
  0b1000, 
  0b0100, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
//a
  0b0000, 
  0b1100, 
  0b0110, 
  0b1010, 
  0b1110, 
  0b0000, 
//b
  0b1000, 
  0b1100, 
  0b1010, 
  0b1010, 
  0b1100, 
  0b0000, 
//c
  0b0000, 
  0b0110, 
  0b1000, 
  0b1000, 
  0b0110, 
  0b0000, 
//d
  0b0010, 
  0b0110, 
  0b1010, 
  0b1010, 
  0b0110, 
  0b0000, 
//e
  0b0000, 
  0b0110, 
  0b1010, 
  0b1100, 
  0b0110, 
  0b0000, 
//f
  0b0010, 
  0b0100, 
  0b1110, 
  0b0100, 
  0b0100, 
  0b0000, 
//g
  0b0000, 
  0b0110, 
  0b1010, 
  0b1110, 
  0b0010, 
  0b0100, 
//h
  0b1000, 
  0b1100, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b0000, 
//i
  0b0100, 
  0b0000, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b0000, 
//j
  0b0010, 
  0b0000, 
  0b0010, 
  0b0010, 
  0b1010, 
  0b0100, 
//k
  0b1000, 
  0b1010, 
  0b1100, 
  0b1100, 
  0b1010, 
  0b0000, 
//l
  0b1100, 
  0b0100, 
  0b0100, 
  0b0100, 
  0b1110, 
  0b0000, 
//m
  0b0000, 
  0b1110, 
  0b1110, 
  0b1110, 
  0b1010, 
  0b0000, 
//n
  0b0000, 
  0b1100, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b0000, 
//o
  0b0000, 
  0b0100, 
  0b1010, 
  0b1010, 
  0b0100, 
  0b0000, 
//p
  0b0000, 
  0b1100, 
  0b1010, 
  0b1010, 
  0b1100, 
  0b1000, 
//q
  0b0000, 
  0b0110, 
  0b1010, 
  0b1010, 
  0b0110, 
  0b0010, 
//r
  0b0000, 
  0b0110, 
  0b1000, 
  0b1000, 
  0b1000, 
  0b0000, 
//s
  0b0000, 
  0b0110, 
  0b1100, 
  0b0110, 
  0b1100, 
  0b0000, 
//t
  0b0100, 
  0b1110, 
  0b0100, 
  0b0100, 
  0b0110, 
  0b0000, 
//u
  0b0000, 
  0b1010, 
  0b1010, 
  0b1010, 
  0b0110, 
  0b0000, 
//v
  0b0000, 
  0b1010, 
  0b1010, 
  0b1110, 
  0b0100, 
  0b0000, 
//w
  0b0000, 
  0b1010, 
  0b1110, 
  0b1110, 
  0b1110, 
  0b0000, 
//x
  0b0000, 
  0b1010, 
  0b0100, 
  0b0100, 
  0b1010, 
  0b0000, 
//y
  0b0000, 
  0b1010, 
  0b1010, 
  0b0110, 
  0b0010, 
  0b0100, 
//z
  0b0000, 
  0b1110, 
  0b0110, 
  0b1100, 
  0b1110, 
  0b0000, 
//{
  0b0110, 
  0b0100, 
  0b1000, 
  0b0100, 
  0b0110, 
  0b0000, 
//|
  0b0100, 
  0b0100, 
  0b0000, 
  0b0100, 
  0b0100, 
  0b0000, 
//}
  0b1100, 
  0b0100, 
  0b0010, 
  0b0100, 
  0b1100, 
  0b0000, 
//~
  0b0110, 
  0b1100, 
  0b0000, 
  0b0000, 
  0b0000, 
  0b0000, 
};
