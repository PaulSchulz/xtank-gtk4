/* minimal.c */

#include <stdio.h>
#include <program.h>

static void minimal_main();

Prog_desc minimal_prog = {
    "minimal",
    "Vanguard",
    "Does nothing.",
    "Robert Potter",
    0,
    0,
    minimal_main
};

static void minimal_main()
{
    while (1) {
        fprintf(stderr,".");
        sync();
        sleep(1);
    }
}
