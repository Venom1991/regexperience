#include "internal/semantic_analysis/ast_nodes/ast_node.h"

G_DEFINE_ABSTRACT_TYPE (AstNode, ast_node, G_TYPE_OBJECT)

static gboolean ast_node_default_is_valid (AstNode  *self,
                                           GError  **error);

static void
ast_node_class_init (AstNodeClass *klass)
{
  klass->build_fsm = NULL;
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

FsmConvertible *
ast_node_build_fsm (AstNode *self)
{
  AstNodeClass *klass;

  g_return_val_if_fail (AST_NODES_IS_AST_NODE (self), NULL);

  klass = AST_NODES_AST_NODE_GET_CLASS (self);

  g_return_val_if_fail (klass->build_fsm != NULL, NULL);

  return klass->build_fsm (self);
}

gboolean
ast_node_is_valid (AstNode  *self,
                   GError  **error)
{
  AstNodeClass *klass;

  g_return_val_if_fail (AST_NODES_IS_AST_NODE (self), FALSE);

  klass = AST_NODES_AST_NODE_GET_CLASS (self);

  g_assert (klass->is_valid != NULL);

  return klass->is_valid (self, error);
}
