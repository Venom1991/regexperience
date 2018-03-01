#ifndef REGEXPERIENCE_GRAMMAR_H
#define REGEXPERIENCE_GRAMMAR_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_GRAMMAR (grammar_get_type ())
#define grammar_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_GRAMMAR, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Grammar, grammar, SYNTACTIC_ANALYSIS, GRAMMAR, GObject)

#define PROP_GRAMMAR_PRODUCTIONS         "productions"

#define START                            "start"
#define EXPRESSION                       "expression"
#define ALTERNATION                      "alternation"
#define SIMPLE_EXPRESSION                "simple-expression"
#define CONCATENATION                    "concatenation"
#define BASIC_EXPRESSION                 "basic-expression"
#define STAR_QUANTIFICATION              "star-quantification"
#define PLUS_QUANTIFICATION              "plus-quantification"
#define QUESTION_MARK_QUANTIFICATION     "question-mark-quantification"
#define ELEMENTARY_EXPRESSION            "elementary-expression"
#define GROUP                            "group"
#define BRACKET_EXPRESSION               "bracket-expression"
#define UPPER_CASE_LETTER                "upper-case-letter"
#define LOWER_CASE_LETTER                "lower-case-letter"
#define DIGIT                            "digit"
#define SPECIAL_CHARACTER                "special-character"
#define REGULAR_METACHARACTER            "regular-metacharacter"
#define BRACKET_EXPRESSION_METACHARACTER "bracket-expression-metacharacter"
#define METACHARACTER_ESCAPE             "metacharacter-escape"
#define BRACKET_EXPRESSION_ITEMS         "bracket-expression-items"
#define BRACKET_EXPRESSION_ITEM          "bracket-expression-item"
#define RANGE                            "range"

G_END_DECLS

#endif /* REGEXPERIENCE_GRAMMAR_H */
