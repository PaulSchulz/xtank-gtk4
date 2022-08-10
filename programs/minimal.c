/* minimal.c */

#include <xtanklib.h>

int i;
static void main()
{
    while (1) {
        i++;
    }
}

Prog_desc minimal_prog = {
    "minimal",
    "Vanguard",
    "Does nothing.",
    "Robert Potter",
    0,
    0,
    main
};
