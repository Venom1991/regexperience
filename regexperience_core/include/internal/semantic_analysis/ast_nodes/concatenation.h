#ifndef REGEXPERIENCE_CORE_CONCATENATION_H
#define REGEXPERIENCE_CORE_CONCATENATION_H

#include <glib-object.h>

#include "binary_operator.h"

G_BEGIN_DECLS

#define SEMANTIC_ANALYSIS_TYPE_CONCATENATION (concatenation_get_type ())
#define concatenation_new(...) (g_object_new (SEMANTIC_ANALYSIS_TYPE_CONCATENATION, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Concatenation, concatenation, SEMANTIC_ANALYSIS, CONCATENATION, BinaryOperator)

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_CONCATENATION_H */