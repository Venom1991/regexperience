#ifndef REGEXPERIENCE_CORE_PARSING_TABLE_KEY_H
#define REGEXPERIENCE_CORE_PARSING_TABLE_KEY_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_PARSING_TABLE_KEY (parsing_table_key_get_type ())
#define parsing_table_key_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_PARSING_TABLE_KEY, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (ParsingTableKey, parsing_table_key, SYNTACTIC_ANALYSIS, PARSING_TABLE_KEY, GObject)

guint    parsing_table_key_hash     (ParsingTableKey *self);

gboolean parsing_table_key_is_equal (ParsingTableKey *a,
                                     ParsingTableKey *b);

#define PROP_PARSING_TABLE_KEY_PRODUCTION "production"
#define PROP_PARSING_TABLE_KEY_TERMINAL   "terminal"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_PARSING_TABLE_KEY_H */
