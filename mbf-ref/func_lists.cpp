#include "func_lists.h"

mbf6** funclists;
int* funcsize;

bool cmp6(mbf6* fl, int i, int j) {
    return ((~fl[i]) | fl[j]) == F6_MAX;
}

void uplevel(int init_level) {
    int i, j;
    int lsize = funcsize[init_level];
    int newsize = 0;
    for (i = 0; i < lsize; i++) {
        for (j = 0; j < lsize; j++) {
            if (cmp6(funclists[init_level], i, j))
                newsize++;
        }
    }
    funcsize[init_level + 1] = newsize;
    funclists[init_level + 1] = new mbf6[newsize];

    newsize = 0;
    int shift = 1 << init_level;
    for (i = 0; i < lsize; i++) {
        for (j = 0; j < lsize; j++) {
            if (cmp6(funclists[init_level], i, j)) {
                funclists[init_level + 1][newsize] = ((funclists[init_level][i]) << shift) | funclists[init_level][j];
                newsize++;
            }
        }
    }
}

void func_lists_init() {
    funclists = new mbf6*[7];
    funclists[0] = new mbf6[2];
    funclists[0][0] = 0;
    funclists[0][1] = 1;
    funcsize = new int[7];
    funcsize[0] = 2;
    uplevel(0);
    uplevel(1);
    uplevel(2);
    uplevel(3);
    uplevel(4);
    uplevel(5);
}
