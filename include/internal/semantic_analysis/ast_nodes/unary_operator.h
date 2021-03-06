#ifndef REGEXPERIENCE_UNARY_OPERATOR_H
#define REGEXPERIENCE_UNARY_OPERATOR_H

#include "ast_node.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define AST_NODES_TYPE_UNARY_OPERATOR (unary_operator_get_type ())

G_DECLARE_DERIVABLE_TYPE (UnaryOperator, unary_operator, AST_NODES, UNARY_OPERATOR, AstNode)

struct _UnaryOperatorClass
{
  AstNodeClass parent_class;

  FsmConvertible * (*build_acceptor) (AstNode        *self,
                                      FsmConvertible *operand_acceptor);

  gpointer     padding[8];
};

#define PROP_UNARY_OPERATOR_OPERAND "operand"

G_END_DECLS

#endif /* REGEXPERIENCE_UNARY_OPERATOR_H */
