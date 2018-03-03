#include "internal/semantic_analysis/ast_nodes/bracket_expression.h"

struct _BracketExpression
{
    UnaryOperator parent_instance;
};

static FsmConvertible *bracket_expression_build_acceptor (AstNode *self);

G_DEFINE_TYPE (BracketExpression, bracket_expression, AST_NODES_TYPE_UNARY_OPERATOR)

static void
bracket_expression_class_init (BracketExpressionClass *klass)
{
  AstNodeClass *ast_node_class = AST_NODES_AST_NODE_CLASS (klass);

  ast_node_class->build_acceptor = bracket_expression_build_acceptor;
}

static void
bracket_expression_init (BracketExpression *self)
{
  /* NOP */
}

static FsmConvertible *
bracket_expression_build_acceptor (AstNode *self)
{
  g_return_val_if_fail (AST_NODES_IS_BRACKET_EXPRESSION (self), NULL);

  g_autoptr (AstNode) operand = NULL;

  g_object_get (self,
                PROP_UNARY_OPERATOR_OPERAND, &operand,
                NULL);

  return ast_node_build_acceptor (operand);
}
