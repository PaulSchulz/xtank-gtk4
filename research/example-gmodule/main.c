/*
Modified from www.freedesktop.org.
   https://www.freedesktop.org/software/gstreamer-sdk/data/docs/2012.5/glib/glib-Dynamic-Loading-of-Modules.html
 */

#include <glib.h>
#include <gmodule.h>
#include <stdio.h>

/* Error Handling
   See: https://developer.gimp.org/api/2.0/glib/glib-Error-Reporting.html
*/
#define SAY_ERROR_OPEN 1
#define SAY_ERROR_LINK 2
#define SAY_ERROR_USE  3

GQuark
g_say_error_quark (void)
{
    return g_quark_from_static_string ("say-error-quark");
}

/* the function signature for 'say_hello' */
typedef void (* SayHelloFunc) (const char *message);

gboolean
just_say_hello (const char *filename, GError **error)
{
  SayHelloFunc  say_hello;
  GModule      *module;

  GQuark error_quark;

  printf("*** Open Module: %s\n", filename);
  module = g_module_open (filename, G_MODULE_BIND_LAZY);
  if (!module) {
      printf("*E* Unable to open module\n");
      g_set_error (error, g_say_error_quark(), SAY_ERROR_OPEN,
                   "%s", g_module_error ());
      return FALSE;
  }

  printf("*** Link symbol: %s\n", "say_hello");
  if (!g_module_symbol (module, "say_hello", (gpointer *)&say_hello))
  {
      printf("*E* Unable to link symbol");
      g_set_error (error, g_say_error_quark(), SAY_ERROR_LINK,
                   "%s: %s", filename, g_module_error ());
      if (!g_module_close (module))
          g_warning ("%s: %s", filename, g_module_error ());
      return FALSE;
  }

  if (say_hello == NULL)
    {
        g_set_error (error, g_say_error_quark(), SAY_ERROR_USE,
                     "%s", "symbol say_hello is NULL");
        if (!g_module_close (module))
            g_warning ("%s: %s", filename, g_module_error ());
        return FALSE;
    }

  /* call our function in the module */
  say_hello ("Hello world!");

  if (!g_module_close (module))
    g_warning ("%s: %s", filename, g_module_error ());
  return TRUE;
}

void main (int argc, char **argv){

    GError **error;
    just_say_hello("./libmodule.so", error);

    printf("*** Finished ***\n");
}
