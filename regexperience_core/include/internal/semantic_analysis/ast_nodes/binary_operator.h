#ifndef REGEXPERIENCE_CORE_BINARY_OPERATOR_H
#define REGEXPERIENCE_CORE_BINARY_OPERATOR_H

#include <glib-object.h>

#include "ast_node.h"

G_BEGIN_DECLS

#define SEMANTIC_ANALYSIS_TYPE_BINARY_OPERATOR (binary_operator_get_type ())

G_DECLARE_DERIVABLE_TYPE (BinaryOperator, binary_operator, SEMANTIC_ANALYSIS, BINARY_OPERATOR, AstNode)

struct _BinaryOperatorClass
{
    AstNodeClass parent_class;
};

#define PROP_BINARY_OPERATOR_LEFT_OPERAND  "left-operand"
#define PROP_BINARY_OPERATOR_RIGHT_OPERAND "right-operand"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_BINARY_OPERATOR_H */
