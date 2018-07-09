#ifndef REGEXPERIENCE_AST_NODE_H
#define REGEXPERIENCE_AST_NODE_H

#include <glib-object.h>

#include "internal/state_machines/fsm_convertible.h"

G_BEGIN_DECLS

#define AST_NODES_TYPE_AST_NODE (ast_node_get_type ())

G_DECLARE_DERIVABLE_TYPE (AstNode, ast_node, AST_NODES, AST_NODE, GObject)

typedef enum
{
  OPERATOR_TYPE_UNDEFINED,
  OPERATOR_TYPE_ALTERNATION,
  OPERATOR_TYPE_CONCATENATION,
  OPERATOR_TYPE_STAR_QUANTIFICATION,
  OPERATOR_TYPE_PLUS_QUANTIFICATION,
  OPERATOR_TYPE_QUESTION_MARK_QUANTIFICATION,
  OPERATOR_TYPE_RANGE
} OperatorType;

struct _AstNodeClass
{
  GObjectClass parent_class;

  FsmConvertible * (*build_acceptor) (AstNode  *self);
  gboolean         (*is_valid)       (AstNode  *self,
                                      GError  **error);

  gpointer     padding[8];
};

FsmConvertible *ast_node_build_acceptor (AstNode  *self);

gboolean        ast_node_is_valid       (AstNode  *self,
                                         GError  **error);

G_END_DECLS

#endif /* REGEXPERIENCE_AST_NODE_H */
