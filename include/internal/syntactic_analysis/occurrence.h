#ifndef REGEXPERIENCE_OCCURRENCE_H
#define REGEXPERIENCE_OCCURRENCE_H

#include "derivation_item.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_OCCURRENCE (occurrence_get_type ())
#define occurrence_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_OCCURRENCE, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Occurrence, occurrence, SYNTACTIC_ANALYSIS, OCCURRENCE, DerivationItem)

#define PROP_OCCURRENCE_POSITION "position"

G_END_DECLS

#endif /* REGEXPERIENCE_OCCURRENCE_H */
