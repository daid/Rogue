#ifndef JEDI_H
#define JEDI_H

#include <stdint.h>

extern uint8_t jedi_screen_buffer[128*64];

//Interface function for the graphical jedi interface
void initJedi();

#define DISPLAY_INVERT 0x100

void clearMapDisplay();
void setMapDisplay(int x, int y, int display_char);
int getMapDisplay(int x, int y);
void refreshMap();
void displayLargeMap();
void setMapViewTarget(int x, int y);
void setStatusLine(const char* buffer);
int displayMessage(const char* buffer);
int askForInput(const char* message, char* input_buffer, int input_buffer_size);

const char* getKeyName(int key);

void startDisplayOfStringList();
void displayStringListItem(const char* fmt, ...);
void finishDisplayOfStringList();

#endif//JEDI_H
