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

Metacharacters are escaped the usual way - using the backslash character.

### Example usage:

```c
/* 1. Instantiate */
Regexperience *regex = regexperience_new ();
GError *error = NULL;
gboolean match = FALSE;

/* 2. Compile */
regexperience_compile (regex, "(ab|CD|[4-8])*", &error);
/* ... check if an error happened ... */

/* 3. Match */
match = regexperience_match (regex, "ab", &error);  /* TRUE */
match = regexperience_match (regex, "CD", &error);  /* TRUE */
match = regexperience_match (regex, "468", &error); /* TRUE */
match = regexperience_match (regex, "", &error);    /* TRUE */
match = regexperience_match (regex, "0", &error);   /* FALSE */
/* ... so on and so forth ... */

/* Same instance can be reused to compile another expression */
regexperience_compile (regex, ".+", &error);

match = regexperience_match (regex, "foobar", &error); /* TRUE */
match = regexperience_match (regex, "", &error);       /* FALSE */

/* 4. Release resources (either explicitly or by using the aforementioned automatic cleanup macros) */
g_error_free (error);
g_object_unref (regex);
```
