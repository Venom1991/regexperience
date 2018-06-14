#ifndef REGEXPERIENCE_RANGE_H
#define REGEXPERIENCE_RANGE_H

#include <glib-object.h>

#include "binary_operator.h"

G_BEGIN_DECLS

#define AST_NODES_TYPE_RANGE (range_get_type ())
#define range_new(...) (g_object_new (AST_NODES_TYPE_RANGE, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Range, range, AST_NODES, RANGE, BinaryOperator)

G_END_DECLS

#endif /* REGEXPERIENCE_RANGE_H */
