#ifndef REGEXPERIENCE_CORE_AST_NODE_H
#define REGEXPERIENCE_CORE_AST_NODE_H

#include <glib-object.h>

#include "internal/state_machines/state_machine_convertible.h"

G_BEGIN_DECLS

#define SEMANTIC_ANALYSIS_TYPE_AST_NODE (ast_node_get_type ())

G_DECLARE_DERIVABLE_TYPE (AstNode, ast_node, SEMANTIC_ANALYSIS, AST_NODE, GObject)

struct _AstNodeClass
{
    GObjectClass parent_class;

    StateMachineConvertible * (*build_state_machine) (AstNode *self);
    gboolean (*is_valid) (AstNode *self, GError **error);

    gpointer padding[8];
};

StateMachineConvertible *ast_node_build_state_machine (AstNode *self);

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
