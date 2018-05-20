#ifndef REGEXPERIENCE_PRODUCTION_H
#define REGEXPERIENCE_PRODUCTION_H

#include <glib-object.h>

#include "internal/syntactic_analysis/derivation_item.h"

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_PRODUCTION (production_get_type ())
#define production_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_PRODUCTION, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Production, production, SYNTACTIC_ANALYSIS, PRODUCTION, GObject)

void       production_mark_occurrence    (Production     *self,
                                          DerivationItem *occurrence);

GPtrArray *production_compute_first_set  (Production     *self);

GPtrArray *production_compute_follow_set (Production     *self);

#define PROP_PRODUCTION_CAPTION     "caption"
#define PROP_PRODUCTION_RULES       "rules"

G_END_DECLS

#endif /* REGEXPERIENCE_PRODUCTION_H */
