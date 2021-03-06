#ifndef REGEXPERIENCE_LEXER_H
#define REGEXPERIENCE_LEXER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define LEXICAL_ANALYSIS_TYPE_LEXER (lexer_get_type ())
#define lexer_new(...) (g_object_new (LEXICAL_ANALYSIS_TYPE_LEXER, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Lexer, lexer, LEXICAL_ANALYSIS, LEXER, GObject)

GPtrArray *lexer_tokenize (Lexer        *self,
                           const gchar  *expression,
                           GError      **error);

G_END_DECLS

#endif /* REGEXPERIENCE_LEXER_H */
