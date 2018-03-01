#ifndef REGEXPERIENCE_CORE_RULE_H
#define REGEXPERIENCE_CORE_RULE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_RULE (rule_get_type ())
#define rule_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_RULE, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Rule, rule, SYNTACTIC_ANALYSIS, RULE, GObject)

#define PROP_RULE_SYMBOLS "symbols"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_RULE_H */
