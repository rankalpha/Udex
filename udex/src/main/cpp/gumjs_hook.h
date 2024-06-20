//
// Created by weimo on 2023/4/1.
//

#ifndef GUMJS_HOOK_H
#define GUMJS_HOOK_H

#include "frida-gumjs.h"

static void on_message( const gchar *message, GBytes *data, gpointer user_data);

int gumjsHook(const char *scriptpath);

char *readfile(const char *filepath);

int hookFunc(const char *scriptpath);

#endif //GUMJS_HOOK_H
