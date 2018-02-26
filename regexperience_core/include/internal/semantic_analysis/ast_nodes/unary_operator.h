#ifndef REGEXPERIENCE_CORE_UNARY_OPERATOR_H
#define REGEXPERIENCE_CORE_UNARY_OPERATOR_H

#include <glib-object.h>

#include "ast_node.h"

G_BEGIN_DECLS

#define SEMANTIC_ANALYSIS_TYPE_UNARY_OPERATOR (unary_operator_get_type ())

G_DECLARE_DERIVABLE_TYPE (UnaryOperator, unary_operator, SEMANTIC_ANALYSIS, UNARY_OPERATOR, AstNode)

struct _UnaryOperatorClass
{
    AstNodeClass parent_class;
};

#define PROP_UNARY_OPERATOR_OPERAND "operand"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_UNARY_OPERATOR_H */
