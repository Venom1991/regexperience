#ifndef REGEXPERIENCE_QUANTIFICATION_C_H
#define REGEXPERIENCE_QUANTIFICATION_C_H

#include "unary_operator.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define AST_NODES_TYPE_QUANTIFICATION (quantification_get_type ())
#define quantification_new(...) (g_object_new (AST_NODES_TYPE_QUANTIFICATION, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Quantification, quantification, AST_NODES, QUANTIFICATION, UnaryOperator)

typedef enum
{
  QUANTIFICATION_BOUND_TYPE_UNDEFINED,
  QUANTIFICATION_BOUND_TYPE_ZERO,
  QUANTIFICATION_BOUND_TYPE_ONE,
  QUANTIFICATION_BOUND_TYPE_INFINITY
} QuantificationBoundType;

#define PROP_QUANTIFICATION_LOWER_BOUND "lower-bound"
#define PROP_QUANTIFICATION_UPPER_BOUND "upper-bound"

G_END_DECLS

#endif /* REGEXPERIENCE_QUANTIFICATION_C_H */
