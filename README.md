# regexperience
A simple DFA-based regular expression engine written in C with help from GLib and GObject.  
The regexperience_core subdirectory contains the library itself whereas the regexperience_cli subdirectory contains a bare bones CLI that depends on it.

### Supported features:

* quantifiers: *, +, ?
* alternation: |
* groups: ( )
* bracket expressions: [ ]
    * value ranges: -

Metacharacters are escaped the usual way - using the backslash character.

### Usage:

```c
/* 1. Instantiate */
Regexperience regex = regexperience_new ();
GError *error = NULL;

/* 2. Compile */
regexperience_compile (regex, "(ab|CD|[4-8])*", &error);
/* ... check if an error happened ... */

/* 3. Match */
gboolean match = FALSE;

match = regexperience_match (regex, "ab", &error);  /* TRUE */
match = regexperience_match (regex, "CD", &error);  /* TRUE */
match = regexperience_match (regex, "468", &error); /* TRUE */
match = regexperience_match (regex, "", &error);    /* TRUE */
match = regexperience_match (regex, "0", &error);   /* FALSE */
/* ... so on and so forth ... */
```
