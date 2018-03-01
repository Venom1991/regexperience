#ifndef REGEXPERIENCE_PARSER_H
#define REGEXPERIENCE_PARSER_H

#include <glib-object.h>

#include "grammar.h"
#include "production.h"
#include "internal/syntactic_analysis/token.h"

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_PARSER (parser_get_type ())
#define parser_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_PARSER, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Parser, parser, SYNTACTIC_ANALYSIS, PARSER, GObject)

GNode *parser_build_concrete_syntax_tree (Parser     *self,
                                          GPtrArray  *tokens,
                                          GError    **error);

G_END_DECLS

#endif /* REGEXPERIENCE_PARSER_H */
