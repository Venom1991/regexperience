#ifndef REGEXPERIENCE_TOKEN_H
#define REGEXPERIENCE_TOKEN_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_TOKEN (token_get_type ())
#define token_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_TOKEN, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Token, token, SYNTACTIC_ANALYSIS, TOKEN, GObject)

#define PROP_TOKEN_CATEGORY "category"
#define PROP_TOKEN_LEXEME   "lexeme"

typedef enum _TokenCategory
{
    TOKEN_CATEGORY_UNDEFINED,
    TOKEN_CATEGORY_ORDINARY_CHARACTER,
    TOKEN_CATEGORY_END_OF_INPUT_MARKER,
    TOKEN_CATEGORY_ALTERNATION_OPERATOR,
    TOKEN_CATEGORY_STAR_QUANTIFICATION_OPERATOR,
    TOKEN_CATEGORY_PLUS_QUANTIFICATION_OPERATOR,
    TOKEN_CATEGORY_QUESTION_MARK_QUANTIFICATION_OPERATOR,
    TOKEN_CATEGORY_METACHARACTER_ESCAPE,
    TOKEN_CATEGORY_OPEN_PARENTHESIS,
    TOKEN_CATEGORY_CLOSE_PARENTHESIS,
    TOKEN_CATEGORY_OPEN_BRACKET,
    TOKEN_CATEGORY_CLOSE_BRACKET,
    TOKEN_CATEGORY_BRACKET_EXPRESSION_NEGATION_OPERATOR,
    TOKEN_CATEGORY_RANGE_OPERATOR,
    TOKEN_CATEGORY_N_CATEGORIES
} TokenCategory;

G_END_DECLS

#endif /* REGEXPERIENCE_TOKEN_H */