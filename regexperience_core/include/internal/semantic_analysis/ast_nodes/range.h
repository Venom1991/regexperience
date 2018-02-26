#ifndef REGEXPERIENCE_CORE_RANGE_H
#define REGEXPERIENCE_CORE_RANGE_H

#include <glib-object.h>

#include "binary_operator.h"

G_BEGIN_DECLS

#define SEMANTIC_ANALYSIS_TYPE_RANGE (range_get_type ())
#define range_new(...) (g_object_new (SEMANTIC_ANALYSIS_TYPE_RANGE, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Range, range, SEMANTIC_ANALYSIS, RANGE, BinaryOperator)

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_RANGE_H */
