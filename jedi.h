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
void refreshMapWithMore();
void refreshMap();
void animationDelay();
void displayLargeMap();
void setMapViewTarget(int x, int y);
void setStatusLine(const char* buffer);
int displayMessage(const char* buffer);
int askForInput(const char* message, char* input_buffer, unsigned int input_buffer_size);

const char* getKeyName(int key);

void startDisplayOfStringList();
int displayStringListItem(const char* fmt, ...);
int finishDisplayOfStringList();

int getRandomNumber(int max);

#endif//JEDI_H
