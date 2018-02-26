#include "internal/semantic_analysis/ast_nodes/bracket_expression.h"

struct _BracketExpression
{
    UnaryOperator parent_instance;
};

G_DEFINE_TYPE (BracketExpression, bracket_expression, SEMANTIC_ANALYSIS_TYPE_UNARY_OPERATOR)

static StateMachineConvertible *bracket_expression_build_state_machine (AstNode *self);

static void
bracket_expression_class_init (BracketExpressionClass *klass)
{
  AstNodeClass *ast_node_class = SEMANTIC_ANALYSIS_AST_NODE_CLASS (klass);

  ast_node_class->build_state_machine = bracket_expression_build_state_machine;
}

static void
bracket_expression_init (BracketExpression *self)
{
  /* NOP */
}

static StateMachineConvertible *
bracket_expression_build_state_machine (AstNode *self)
{
  g_return_val_if_fail (SEMANTIC_ANALYSIS_IS_BRACKET_EXPRESSION (self), NULL);

  g_autoptr (AstNode) operand = NULL;

  g_object_get (self,
                PROP_UNARY_OPERATOR_OPERAND, &operand,
                NULL);

  return ast_node_build_state_machine (operand);
}
