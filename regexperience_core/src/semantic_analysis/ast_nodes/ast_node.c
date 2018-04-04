#include "internal/semantic_analysis/ast_nodes/ast_node.h"

static gboolean ast_node_default_is_valid (AstNode  *self,
                                           GError  **error);

G_DEFINE_ABSTRACT_TYPE (AstNode, ast_node, G_TYPE_OBJECT)

static void
ast_node_class_init (AstNodeClass *klass)
{
  klass->build_acceptor = NULL;
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
ast_node_build_acceptor (AstNode *self)
{
  g_return_val_if_fail (AST_NODES_IS_AST_NODE (self), NULL);

  AstNodeClass *klass = AST_NODES_AST_NODE_GET_CLASS (self);

  g_return_val_if_fail (klass->build_acceptor != NULL, NULL);

  return klass->build_acceptor (self);
}

gboolean
ast_node_is_valid (AstNode  *self,
                   GError  **error)
{
  g_return_val_if_fail (AST_NODES_IS_AST_NODE (self), FALSE);

  AstNodeClass *klass = AST_NODES_AST_NODE_GET_CLASS (self);

  g_assert (klass->is_valid != NULL);

  return klass->is_valid (self, error);
}
