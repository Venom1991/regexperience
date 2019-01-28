# regexperience
A simple DFA-based regular expression engine written in C with help from GLib and GObject.  
GCC or Clang are required to compile the source code due to the usage of GLib's automatic cleanup macros ([g_autoptr](https://developer.gnome.org/glib/stable/glib-Miscellaneous-Macros.html#g-autoptr), for example).

### Supported features:

* quantifiers: *, +, ?
* alternation: |
* groups: ( )
* bracket expressions: [ ]
    * value ranges: -
* dot metacharacter: .
* anchors: ^, $

Metacharacters are escaped the usual way - using the backslash character.

### Example usage:

```c
/* 1. Instantiate */
Regexperience *regex = regexperience_new ();
GPtrArray *matches = NULL;
GError *error = NULL;
gboolean is_match = FALSE;

/* 2. Compile */
regexperience_compile (regex, "(ab|CD|[4-8])*", &error);
/* ... check if an error happened ... */

/* 3. Match */
is_match = regexperience_match (regex, "0", &matches, &error);   /* FALSE */
is_match = regexperience_match (regex, "ab", &matches, &error);  /* TRUE */
is_match = regexperience_match (regex, "CD", &matches, &error);  /* TRUE */
is_match = regexperience_match (regex, "468", &matches, &error); /* TRUE */
is_match = regexperience_match (regex, "", &matches, &error);    /* TRUE */
/* ... so on and so forth ... */

/* Same instance can be reused to compile another expression */
regexperience_compile (regex, ".+", &error);

is_match = regexperience_match (regex, "", &matches, &error);       /* FALSE */
is_match = regexperience_match (regex, "foobar", &matches, &error); /* TRUE */

/* 4. Do something with the found matches */
if (is_match)
  {
    g_printf ("Matches:\n");

    for (guint i = 0; i < matches->len; ++i)
      {
        Match *match = g_ptr_array_index (matches, i);
        g_autoptr (GString) value = NULL;
        guint range_begin = 0;
        guint range_end = 0;

        g_object_get (match,
                      PROP_MATCH_VALUE, &value,
                      PROP_MATCH_RANGE_BEGIN, &range_begin,
                      PROP_MATCH_RANGE_END, &range_end,
                      NULL);

        g_printf ("\tValue: %s, Range: %u - %u\n", value->str, range_begin, range_end);
      }
  }

/* 4. Release resources (either explicitly or by using the aforementioned automatic cleanup macros) */
g_error_free (error);
g_ptr_array_unref (matches);
g_object_unref (regex);
```
