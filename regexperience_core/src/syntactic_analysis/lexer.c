#include "internal/syntactic_analysis/lexer.h"
#include "internal/syntactic_analysis/lexeme.h"
#include "internal/common/macros.h"
#include "core/errors.h"

struct _Lexer
{
    GObject parent_instance;
};

G_DEFINE_TYPE (Lexer, lexer, G_TYPE_OBJECT)

typedef enum _LexingContext
{
    LEXING_CONTEXT_UNDEFINED,
    LEXING_CONTEXT_COMMON_TOKENS,
    LEXING_CONTEXT_SET_TOKENS
} LexingContext;

G_DEFINE_QUARK (syntactic-analysis-lexer-error-quark, syntactic_analysis_lexer_error)
#define SYNTACTIC_ANALYSIS_LEXER_ERROR (syntactic_analysis_lexer_error_quark())

static Token *lexer_create_token (Token *previous_token,
                                  gchar *content,
                                  guint *character_position);

static TokenCategory lexer_determine_token_category_type (TokenCategory  previous_token_category,
                                                          GString       *lexeme_content);

static Token *lexer_fetch_previous_token (GPtrArray *tokens);

static void lexer_report_error_if_needed (const gchar  *regular_expression,
                                          GError      **error);

static void
lexer_class_init (LexerClass *klass)
{
  /* NOP */
}

static void
lexer_init (Lexer *self)
{
  /* NOP */
}

GPtrArray *
lexer_tokenize (Lexer        *self,
                const gchar  *regular_expression,
                GError      **error)
{
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_LEXER (self), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  GPtrArray *tokens = g_ptr_array_new_with_free_func (g_object_unref);
  guint character_position = 1;

  lexer_report_error_if_needed (regular_expression, error);

  if (*error != NULL)
    return NULL;

  while (TRUE)
    {
      gchar current_character = *regular_expression++;

      if (current_character == '\0')
        break;

      g_autofree gchar *content = g_strdup_printf ("%c", current_character);
      Token *previous_token = lexer_fetch_previous_token (tokens);
      Token *current_token = lexer_create_token (previous_token,
                                                 content,
                                                 &character_position);

      g_ptr_array_add (tokens, current_token);
    }

  /* Appending the end of input marker. */
  Token *end_of_input_marker = lexer_create_token (NULL,
                                                   EOI,
                                                   &character_position);

  g_ptr_array_add (tokens, end_of_input_marker);

  return tokens;
}

static Token*
lexer_create_token (Token *previous_token,
                    gchar *content,
                    guint *character_position)
{
  g_autoptr (Lexeme) lexeme = NULL;
  g_autoptr (GString) lexeme_content = g_string_new (content);
  gsize lexeme_content_length = lexeme_content->len;
  TokenCategory current_token_category = TOKEN_CATEGORY_UNDEFINED;
  TokenCategory previous_token_category = TOKEN_CATEGORY_UNDEFINED;

  if (previous_token != NULL)
    g_object_get (previous_token,
                  PROP_TOKEN_CATEGORY, &previous_token_category,
                  NULL);

  current_token_category = lexer_determine_token_category_type (previous_token_category,
                                                                lexeme_content);

  lexeme = lexeme_new (PROP_LEXEME_CONTENT, lexeme_content,
                       PROP_LEXEME_START_POSITION, *character_position,
                       PROP_LEXEME_END_POSITION, *character_position + (lexeme_content_length - 1));

  *character_position += lexeme_content_length;

  return token_new (PROP_TOKEN_CATEGORY, current_token_category,
                    PROP_TOKEN_LEXEME, lexeme);
}

static TokenCategory
lexer_determine_token_category_type (TokenCategory  previous_token_category,
                                     GString       *lexeme_content)
{
  gchar *content_string = lexeme_content->str;
  static LexingContext context = LEXING_CONTEXT_UNDEFINED;

  if (context == LEXING_CONTEXT_UNDEFINED &&
      previous_token_category == TOKEN_CATEGORY_UNDEFINED)
    {
      if (g_strcmp0 (content_string, "[") == 0)
          context = LEXING_CONTEXT_SET_TOKENS;
      else
          context = LEXING_CONTEXT_COMMON_TOKENS;
    }

  if (previous_token_category != TOKEN_CATEGORY_METACHARACTER_ESCAPE)
    {
      if (g_strcmp0 (content_string, "\\") == 0)
        return TOKEN_CATEGORY_METACHARACTER_ESCAPE;

      if (context == LEXING_CONTEXT_COMMON_TOKENS)
        {
          if (g_strcmp0 (content_string, "[") == 0)
            {
              context = LEXING_CONTEXT_SET_TOKENS;

              return TOKEN_CATEGORY_OPEN_BRACKET;
            }

          if (g_strcmp0 (content_string, "(") == 0)
            return TOKEN_CATEGORY_OPEN_PARENTHESIS;

          if (g_strcmp0 (content_string, ")") == 0)
            return TOKEN_CATEGORY_CLOSE_PARENTHESIS;

          if (g_strcmp0 (content_string, "*") == 0)
            return TOKEN_CATEGORY_STAR_QUANTIFICATION_OPERATOR;

          if (g_strcmp0 (content_string, "+") == 0)
            return TOKEN_CATEGORY_PLUS_QUANTIFICATION_OPERATOR;

          if (g_strcmp0 (content_string, "?") == 0)
            return TOKEN_CATEGORY_QUESTION_MARK_QUANTIFICATION_OPERATOR;

          if (g_strcmp0 (content_string, "|") == 0)
            return TOKEN_CATEGORY_ALTERNATION_OPERATOR;
        }

      if (context == LEXING_CONTEXT_SET_TOKENS)
        {
          if (g_strcmp0 (content_string, "[") == 0)
            return TOKEN_CATEGORY_OPEN_BRACKET;

          if (g_strcmp0 (content_string, "^") == 0)
            return TOKEN_CATEGORY_BRACKET_EXPRESSION_NEGATION_OPERATOR;

          if (g_strcmp0 (content_string, "-") == 0)
            return TOKEN_CATEGORY_RANGE_OPERATOR;

          if (g_strcmp0 (content_string, "]") == 0)
            {
              context = LEXING_CONTEXT_COMMON_TOKENS;

              return TOKEN_CATEGORY_CLOSE_BRACKET;
            }
        }
    }

  if (g_strcmp0 (content_string, EOI) == 0)
    return TOKEN_CATEGORY_END_OF_INPUT_MARKER;

  return TOKEN_CATEGORY_ORDINARY_CHARACTER;
}

static Token *
lexer_fetch_previous_token (GPtrArray *tokens)
{
  guint tokens_length = tokens->len;

  if (tokens_length > 0)
    {
      return g_ptr_array_index (tokens, tokens_length - 1);
    }

  return NULL;
}

static void
lexer_report_error_if_needed (const gchar  *regular_expression,
                              GError      **error)
{
  SyntacticAnalysisLexerError error_code = SYNTACTIC_ANALYSIS_LEXER_ERROR_UNDEFINED;
  const gchar *error_message = NULL;

  if (regular_expression == NULL)
    {
      error_code = SYNTACTIC_ANALYSIS_LEXER_ERROR_INPUT_NULL;
      error_message = "The regular expression must not be NULL";
    }
  else if (*regular_expression == '\0')
    {
      error_code = SYNTACTIC_ANALYSIS_LEXER_ERROR_INPUT_EMPTY;
      error_message = "The regular expression must not be an empty string";
    }
  else if (!g_str_is_ascii (regular_expression))
    {
      error_code = SYNTACTIC_ANALYSIS_LEXER_ERROR_INPUT_NOT_ASCII;
      error_message = "The regular expression must be an ASCII string";
    }

  if (error_message != NULL)
    {
      g_set_error (error,
                   SYNTACTIC_ANALYSIS_LEXER_ERROR,
                   error_code,
                   error_message);
    }
}
