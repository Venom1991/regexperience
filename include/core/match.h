#ifndef REGEXPERIENCE_MATCH_H
#define REGEXPERIENCE_MATCH_H

#include <glib-object.h>

G_BEGIN_DECLS

#define CORE_TYPE_MATCH (match_get_type ())
#define match_new(...) (g_object_new (CORE_TYPE_MATCH, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Match, match, CORE, MATCH, GObject)

#define PROP_MATCH_VALUE       "value"
#define PROP_MATCH_RANGE_BEGIN "range-begin"
#define PROP_MATCH_RANGE_END   "range-end"

G_END_DECLS

#endif /* REGEXPERIENCE_MATCH_H */
