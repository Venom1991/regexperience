#include <locale.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gprintf.h>

#include <regexperience.h>

gint main (gint argc, gchar *argv[])
{
  const gchar *empty_string = "";
  const gchar *locale = empty_string;
  const gchar *regular_expression = empty_string;
  const gchar *input = empty_string;

  setlocale (LC_ALL, locale);

  if (argc > 1)
    regular_expression = argv[1];

  if (argc > 2)
    input = argv[2];

  g_autoptr (Regexperience) regexperience = regexperience_new ();
/*  g_autoptr (GError) error = NULL;

  regexperience_compile (regexperience, regular_expression, &error);

  if (error != NULL)
    {
      g_fprintf (stderr, "Unable to compile the regular expression - reason: \"%s\"\n", error->message);

      exit (EXIT_FAILURE);
    }

  gboolean match = regexperience_match (regexperience, input, &error);

  if (error != NULL)
    {
      g_fprintf (stderr, "Unable to match the input - reason: \"%s\"\n", error->message);

      exit (EXIT_FAILURE);
    }

  g_printf ("Input matched - %s\n", match ? "yes" : "no");*/

  exit (EXIT_SUCCESS);
}
