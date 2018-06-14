#ifndef REGEXPERIENCE_LEXEME_H
#define REGEXPERIENCE_LEXEME_H

#include <glib-object.h>

G_BEGIN_DECLS

#define LEXICAL_ANALYSIS_TYPE_LEXEME (lexeme_get_type ())
#define lexeme_new(...) (g_object_new (LEXICAL_ANALYSIS_TYPE_LEXEME, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Lexeme, lexeme, LEXICAL_ANALYSIS, LEXEME, GObject)

#define PROP_LEXEME_CONTENT        "content"
#define PROP_LEXEME_START_POSITION "start-position"
#define PROP_LEXEME_END_POSITION   "end-position"

G_END_DECLS

#endif /* REGEXPERIENCE_LEXEME_H */
