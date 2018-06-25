#ifndef REGEXPERIENCE_ANALYZER_H
#define REGEXPERIENCE_ANALYZER_H

#include <glib-object.h>

#include "ast_nodes/ast_node.h"

G_BEGIN_DECLS

#define SEMANTIC_ANALYSIS_TYPE_ANALYZER (analyzer_get_type ())
#define analyzer_new(...) (g_object_new (SEMANTIC_ANALYSIS_TYPE_ANALYZER, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Analyzer, analyzer, SEMANTIC_ANALYSIS, ANALYZER, GObject)

AstNode *analyzer_build_abstract_syntax_tree (Analyzer  *self,
                                              GNode     *concrete_syntax_tree,
                                              GError   **error);

G_END_DECLS

#endif /* REGEXPERIENCE_ANALYZER_H */
