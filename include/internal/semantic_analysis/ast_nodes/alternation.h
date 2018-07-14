#ifndef REGEXPERIENCE_ALTERNATION_H
#define REGEXPERIENCE_ALTERNATION_H

#include "binary_operator.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define AST_NODES_TYPE_ALTERNATION (alternation_get_type ())
#define alternation_new(...) (g_object_new (AST_NODES_TYPE_ALTERNATION, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Alternation, alternation, AST_NODES, ALTERNATION, BinaryOperator)

G_END_DECLS

#endif /* REGEXPERIENCE_ALTERNATION_H */
