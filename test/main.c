/*
  Filename: testing/main.c

  Author: Paul Schulz <paul@mawsonlakes.org>
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <time.h>

#include <math.h>  // cos()

// #include "xtank.h"
#include "xtanklib.h"
#include <dlfcn.h>
#include "program.h"

char code_name[256];

/**
** Compiles and/or dynamically loads the module with the given name.
** The module name should be a complete path to a .c or .o file.
** If the module is a .c file, it is compiled into a .o file in the
** same directory.  The .o file is then linked into xtank by forking
** an ld and frobbing with the symbol table.  The address of the prog_desc
** is put into pdesc.
**
** Return code  Meaning
** -----------  -------
**      0       module compiled and/or linked properly
**      1       module name is improperly formatted
**      2       .c file compiled incorrectly  (errors in /tmp/xtank.errors)
**      3       .o file linked incorrectly    (errors in /tmp/xtank.errors)
**      4       output file cannot be read
**      5       name list for the symbol table could not be generated
**      6       module does not have a program description structure
*/
int compile_module(char *module_name,
                   char *output_name,
                   char *error_name,
                   char **symbol,
                   char **code
    ) {

    char command[1024];

    char *pathname = ".";
    char programsdir[] = ".";
    char headersdir[] = "../src/include";

    void *dl_handle;
	char symbol_name[256];

    // Assume module file is 'module_name'.c
    sprintf(command,
            "cd %s/%s; cc %s -c -O -I%s -o %s.o %s.c >%s 2>&1",
            pathname,
            programsdir,
            "-DUNIX", // ALLDEFINES,
            headersdir,
            module_name,
            module_name,
            error_name);

    sprintf(command,
            "cd %s/%s; cc -shared -fPIC -Wall -ansi -pedantic -DUNIX -I%s -o minimal.so minimal.c >%s 2>&1",
            pathname,
            programsdir,
            headersdir,
            error_name);

    fprintf(stdout, "[DEBUG] Compile command: %s\n", command);

    if (system(command)) {
        fprintf(stdout, "*** Error running command: %s\n", command);
        return 2;
    }

    // Load module
	fprintf(stderr, "[DEBUG] dlopen called\n");

    void *handle;
    void (*func_print_name)(const char*);

    handle = dlopen("./minimal.so", RTLD_NOW);
    if (!handle) {
        /* fail to load the library */
        fprintf(stderr, "Error: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    char *tsymbol;
    void (*func_func)(void);

    tsymbol = (char *) dlsym(handle,"minimal_prog");
    *(void**)(&func_func) = ((Prog_desc *)tsymbol)->func;

    func_func();
}

//////////////////////////////////////////////////////////////////////////////
int
main (int argc, char **argv) {
    char program_name[246] = "minimal";
    char output_name[246];

    char **symbol;
    char **code;

    printf("Testing\n");

    strcpy(output_name, program_name);
    strcat(output_name, ".so");

    printf("Compiling program '%s'\n", program_name);
    compile_module(program_name,
                   output_name,
                   "error.log",
                   symbol,
                   code
        );

}
