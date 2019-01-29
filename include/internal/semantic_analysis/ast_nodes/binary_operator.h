#ifndef REGEXPERIENCE_BINARY_OPERATOR_H
#define REGEXPERIENCE_BINARY_OPERATOR_H

#include "ast_node.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define AST_NODES_TYPE_BINARY_OPERATOR (binary_operator_get_type ())

G_DECLARE_DERIVABLE_TYPE (BinaryOperator, binary_operator, AST_NODES, BINARY_OPERATOR, AstNode)

struct _BinaryOperatorClass
{
  AstNodeClass parent_class;

  FsmConvertible * (*build_acceptor) (AstNode        *self,
                                      FsmConvertible *left_operand_acceptor,
                                      FsmConvertible *right_operand_acceptor);

  gpointer     padding[8];
};

#define PROP_BINARY_OPERATOR_LEFT_OPERAND  "left-operand"
#define PROP_BINARY_OPERATOR_RIGHT_OPERAND "right-operand"

G_END_DECLS

#endif /* REGEXPERIENCE_BINARY_OPERATOR_H */
