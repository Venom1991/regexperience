#ifndef REGEXPERIENCE_CORE_BRACKET_EXPRESSION_H
#define REGEXPERIENCE_CORE_BRACKET_EXPRESSION_H

#include <glib-object.h>

#include "unary_operator.h"

G_BEGIN_DECLS

#define AST_NODES_TYPE_BRACKET_EXPRESSION (bracket_expression_get_type ())
#define bracket_expression_new(...) (g_object_new (AST_NODES_TYPE_BRACKET_EXPRESSION, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (BracketExpression, bracket_expression, AST_NODES, BRACKET_EXPRESSION, UnaryOperator)

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_BRACKET_EXPRESSION_H */
