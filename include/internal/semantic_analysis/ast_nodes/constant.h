#ifndef REGEXPERIENCE_CONSTANT_H
#define REGEXPERIENCE_CONSTANT_H

#include "ast_node.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define AST_NODES_TYPE_CONSTANT (constant_get_type ())
#define constant_new(...) (g_object_new (AST_NODES_TYPE_CONSTANT, ##__VA_ARGS__, NULL))

G_DECLARE_DERIVABLE_TYPE (Constant, constant, AST_NODES, CONSTANT, AstNode)

struct _ConstantClass
{
  AstNodeClass parent_class;
};

#define PROP_CONSTANT_VALUE    "value"
#define PROP_CONSTANT_POSITION "position"

G_END_DECLS

#endif /* REGEXPERIENCE_CONSTANT_H */
