#pragma once

#include "mbf_types.h"

extern mbf6** funclists;
extern int* funcsize;

bool cmp6(mbf6* fl, int i, int j);
void uplevel(int init_level);
void func_lists_init();
