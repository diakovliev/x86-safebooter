#ifndef TEXT_DISPLAY_CONSOLE_HEADER
#define TEXT_DISPLAY_CONSOLE_HEADER

#include <loader_types.h>
#include <stdio.h>
#include "text_display_driver.h"
#include "keyboard_driver.h"

extern console_base_p display_get_console(display_p display, keyboard_driver_p keyboard);

#endif//TEXT_DISPLAY_CONSOLE_HEADER

