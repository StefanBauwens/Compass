// Created by Stefan Bauwens 2026
// Mini Window/Popup Library for Pebble
#ifndef MINIWINDOW_H

#define MINIWINDOW_H

#include <pebble.h>

void create_mini_window(Layer * window_layer, const char * title, const char * body);
void destroy_mini_window();
void show_mini_window();
void hide_mini_window();

#endif // MINIWINDOW_H