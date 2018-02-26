#ifndef REGEXPERIENCE_CORE_CONSTANT_H
#define REGEXPERIENCE_CORE_CONSTANT_H

#include <glib-object.h>

#include "ast_node.h"

G_BEGIN_DECLS

#define SEMANTIC_ANALYSIS_TYPE_CONSTANT (constant_get_type ())
#define constant_new(...) (g_object_new (SEMANTIC_ANALYSIS_TYPE_CONSTANT, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Constant, constant, SEMANTIC_ANALYSIS, CONSTANT, AstNode)

#define PROP_CONSTANT_VALUE    "value"
#define PROP_CONSTANT_POSITION "position"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_CONSTANT_H */
