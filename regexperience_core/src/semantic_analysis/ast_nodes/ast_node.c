#include "internal/semantic_analysis/ast_nodes/ast_node.h"

G_DEFINE_ABSTRACT_TYPE (AstNode, ast_node, G_TYPE_OBJECT)

static gboolean ast_node_default_is_valid (AstNode  *self,
                                           GError  **error);

static void
ast_node_class_init (AstNodeClass *klass)
{
  klass->build_state_machine = NULL;
  klass->is_valid = ast_node_default_is_valid;
}

static void
ast_node_init (AstNode *self)
{
  /* NOP */
}

static gboolean
ast_node_default_is_valid (AstNode  *self,
                           GError  **error)
{
  return TRUE;
}

StateMachineConvertible *
ast_node_build_state_machine (AstNode *self)
{
  AstNodeClass *klass;

  g_return_val_if_fail (SEMANTIC_ANALYSIS_IS_AST_NODE (self), NULL);

  klass = SEMANTIC_ANALYSIS_AST_NODE_GET_CLASS (self);

  g_return_val_if_fail (klass->build_state_machine != NULL, NULL);

  return klass->build_state_machine (self);
}

gboolean
ast_node_is_valid (AstNode  *self,
                   GError  **error)
{
  AstNodeClass *klass;

  g_return_val_if_fail (SEMANTIC_ANALYSIS_IS_AST_NODE (self), FALSE);

  klass = SEMANTIC_ANALYSIS_AST_NODE_GET_CLASS (self);

  g_assert (klass->is_valid != NULL);

  return klass->is_valid (self, error);
}
