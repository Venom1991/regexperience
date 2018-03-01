#ifndef REGEXPERIENCE_CORE_AST_NODE_H
#define REGEXPERIENCE_CORE_AST_NODE_H

#include <glib-object.h>

#include "internal/state_machines/fsm_convertible.h"

G_BEGIN_DECLS

#define AST_NODES_TYPE_AST_NODE (ast_node_get_type ())

G_DECLARE_DERIVABLE_TYPE (AstNode, ast_node, AST_NODES, AST_NODE, GObject)

struct _AstNodeClass
{
    GObjectClass parent_class;

    FsmConvertible * (*build_fsm) (AstNode *self);
    gboolean (*is_valid) (AstNode *self, GError **error);

    gpointer padding[8];
};

FsmConvertible *ast_node_build_fsm (AstNode *self);

gboolean ast_node_is_valid (AstNode  *self,
                            GError  **error);

typedef enum _OperatorType
{
    OPERATOR_TYPE_UNDEFINED,
    OPERATOR_TYPE_ALTERNATION,
    OPERATOR_TYPE_CONCATENATION,
    OPERATOR_TYPE_STAR_QUANTIFICATION,
    OPERATOR_TYPE_PLUS_QUANTIFICATION,
    OPERATOR_TYPE_QUESTION_MARK_QUANTIFICATION,
    OPERATOR_TYPE_BRACKET_EXPRESSION,
    OPERATOR_TYPE_RANGE
} OperatorType;

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_AST_NODE_H */
