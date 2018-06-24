#ifndef REGEXPERIENCE_GRAMMAR_H
#define REGEXPERIENCE_GRAMMAR_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_GRAMMAR (grammar_get_type ())
#define grammar_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_GRAMMAR, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Grammar, grammar, SYNTACTIC_ANALYSIS, GRAMMAR, GObject)

#define PROP_GRAMMAR_ALL_PRODUCTIONS     "all-productions"
#define PROP_GRAMMAR_ALL_TERMINALS       "all-terminals"
#define PROP_GRAMMAR_START_PRODUCTION    "start-production"
#define PROP_GRAMMAR_PARSING_TABLE       "parsing-table"

#define EPSILON                          "ε"

#define START                            "start"
#define EXPRESSION                       "expression"
#define EXPRESSION_PRIME                 "expression-prime"
#define ALTERNATION                      "alternation"
#define ALTERNATION_PRIME                "alternation-prime"
#define SIMPLE_EXPRESSION                "simple-expression"
#define SIMPLE_EXPRESSION_PRIME          "simple-expression-prime"
#define CONCATENATION                    "concatenation"
#define CONCATENATION_PRIME              "concatenation-prime"
#define BASIC_EXPRESSION                 "basic-expression"
#define BASIC_EXPRESSION_PRIME           "basic-expression-prime"
#define STAR_QUANTIFICATION              "star-quantification"
#define PLUS_QUANTIFICATION              "plus-quantification"
#define QUESTION_MARK_QUANTIFICATION     "question-mark-quantification"
#define ELEMENTARY_EXPRESSION            "elementary-expression"
#define ELEMENTARY_EXPRESSION_PRIME      "elementary-expression-prime"
#define GROUP                            "group"
#define BRACKET_EXPRESSION               "bracket-expression"
#define BRACKET_EXPRESSION_ITEMS         "bracket-expression-items"
#define BRACKET_EXPRESSION_ITEMS_PRIME   "bracket-expression-items-prime"
#define BRACKET_EXPRESSION_ITEM          "bracket-expression-item"
#define BRACKET_EXPRESSION_ITEM_PRIME    "bracket-expression-item-prime"
#define LOWER_CASE_LETTER_RANGE          "upper-case-letter-range"
#define UPPER_CASE_LETTER_RANGE          "lower-case-letter-range"
#define DIGIT_RANGE                      "digit-range"
#define UPPER_CASE_LETTER                "upper-case-letter"
#define LOWER_CASE_LETTER                "lower-case-letter"
#define DIGIT                            "digit"
#define SPECIAL_CHARACTER                "special-character"
#define REGULAR_METACHARACTER            "regular-metacharacter"
#define BRACKET_EXPRESSION_METACHARACTER "bracket-expression-metacharacter"
#define ANY_CHARACTER                    "any-character"
#define METACHARACTER_ESCAPE             "metacharacter-escape"

G_END_DECLS

#endif /* REGEXPERIENCE_GRAMMAR_H */
