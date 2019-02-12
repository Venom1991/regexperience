#include "internal/lexical_analysis/lexer.h"
#include "internal/lexical_analysis/lexeme.h"
#include "internal/lexical_analysis/token.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transducers/transducer_runnable.h"
#include "internal/state_machines/transducers/mealy.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"
#include "core/errors.h"

struct _Lexer
{
  GObject parent_instance;
};

typedef struct
{
  TransducerRunnable *transducer;
} LexerPrivate;

typedef struct
{
  gchar          character;
  State         *next_state;
  TokenCategory  token_category;
} MealyMapping;

typedef struct
{
  gchar *original_substring;
  guint  original_length;
  gchar *replacement_substring;
} ExpressionNormalizer;

static void              lexer_create_token            (TokenCategory  category,
                                                        gchar         *content,
                                                        guint         *character_position,
                                                        GPtrArray     *tokens);

static FsmInitializable *lexer_build_transducer        (void);

static GPtrArray        *lexer_create_transitions_from (MealyMapping  *mappings,
                                                        gsize          mappings_count);

static void              lexer_report_error            (const gchar   *expression,
                                                        GError       **error);

static GString *         lexer_normalize_expression    (const gchar   *expression);

static void              lexer_dispose                 (GObject       *object);

G_DEFINE_QUARK (lexical-analysis-lexer-error-quark, lexical_analysis_lexer_error)
#define LEXICAL_ANALYSIS_LEXER_ERROR (lexical_analysis_lexer_error_quark ())

G_DEFINE_TYPE_WITH_PRIVATE (Lexer, lexer, G_TYPE_OBJECT)

static void
lexer_class_init (LexerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = lexer_dispose;
}

static void
lexer_init (Lexer *self)
{
  LexerPrivate *priv = lexer_get_instance_private (self);
  /* Initializing the Mealy transducer which is used to
   * determine the category of each token.
   */
  FsmInitializable *mealy = lexer_build_transducer ();

  priv->transducer = TRANSDUCERS_TRANSDUCER_RUNNABLE (mealy);
}

GPtrArray *
lexer_tokenize (Lexer        *self,
                const gchar  *expression,
                GError      **error)
{
  g_return_val_if_fail (LEXICAL_ANALYSIS_IS_LEXER (self), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  LexerPrivate *priv = lexer_get_instance_private (self);
  TransducerRunnable *transducer = priv->transducer;
  GError *temporary_error = NULL;

  lexer_report_error (expression, &temporary_error);

  if (temporary_error != NULL)
    {
      g_propagate_error (error, temporary_error);

      return NULL;
    }

  transducer_runnable_reset (transducer);

  g_autoptr (GString) normalized_expression_string = lexer_normalize_expression (expression);
  gchar *normalized_expression = normalized_expression_string->str;
  GPtrArray *tokens = g_ptr_array_new_with_free_func (g_object_unref);
  guint character_position = 1;

  while (TRUE)
    {
      gchar current_character = *normalized_expression++;

      if (current_character == END_OF_STRING)
        break;

      TokenCategory category = (TokenCategory) GPOINTER_TO_INT (transducer_runnable_run (transducer,
                                                                                         current_character));

      if (category != TOKEN_CATEGORY_UNDEFINED)
        {
          g_autofree gchar *content = NULL;

          if (category == TOKEN_CATEGORY_EMPTY_EXPRESSION_MARKER)
            content = g_strdup (EMPTY_STRING);
          else
            content = g_strdup_printf ("%c", current_character);

          lexer_create_token (category,
                              content,
                              &character_position,
                              tokens);
        }
    }

  /* Appending the end of input marker. */
  lexer_create_token (TOKEN_CATEGORY_END_OF_INPUT_MARKER,
                      END_OF_INPUT,
                      &character_position,
                      tokens);

  return tokens;
}

static void
lexer_create_token (TokenCategory  category,
                    gchar         *content,
                    guint         *character_position,
                    GPtrArray     *tokens)
{
  g_return_if_fail (tokens != NULL);

  g_autoptr (Lexeme) lexeme = NULL;
  g_autoptr (GString) lexeme_content = g_string_new (content);
  guint lexeme_start_position = 0;
  guint lexeme_end_position = 0;
  gsize lexeme_content_length = lexeme_content->len;

  if (character_position != NULL)
    {
      guint position_offset = 0;

      if (lexeme_content_length > 0)
        position_offset = (guint) lexeme_content_length - 1;

      lexeme_start_position = *character_position;
      lexeme_end_position = (*character_position + position_offset);
      *character_position += lexeme_content_length;
    }

  lexeme = lexeme_new (PROP_LEXEME_CONTENT, lexeme_content,
                       PROP_LEXEME_START_POSITION, lexeme_start_position,
                       PROP_LEXEME_END_POSITION, lexeme_end_position);

  Token *token = token_new (PROP_TOKEN_CATEGORY, category,
                            PROP_TOKEN_LEXEME, lexeme);

  g_ptr_array_add (tokens, token);
}

static FsmInitializable *
lexer_build_transducer (void)
{
  g_autoptr (GPtrArray) all_states = g_ptr_array_new_with_free_func (g_object_unref);
  State *regular_context = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_START);
  State *regular_context_escape = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT);
  State *bracket_context = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT);
  State *bracket_context_escape = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT);

  MealyMapping regular_context_mappings[] =
    {
      { '[',   bracket_context,        TOKEN_CATEGORY_OPEN_BRACKET,                         },
      { '(',   regular_context,        TOKEN_CATEGORY_OPEN_PARENTHESIS                      },
      { ')',   regular_context,        TOKEN_CATEGORY_CLOSE_PARENTHESIS                     },
      { '^',   regular_context,        TOKEN_CATEGORY_START_ANCHOR,                         },
      { '$',   regular_context,        TOKEN_CATEGORY_END_ANCHOR,                           },
      { '*',   regular_context,        TOKEN_CATEGORY_STAR_QUANTIFICATION_OPERATOR          },
      { '+',   regular_context,        TOKEN_CATEGORY_PLUS_QUANTIFICATION_OPERATOR          },
      { '?',   regular_context,        TOKEN_CATEGORY_QUESTION_MARK_QUANTIFICATION_OPERATOR },
      { '|',   regular_context,        TOKEN_CATEGORY_ALTERNATION_OPERATOR                  },
      { '.',   regular_context,        TOKEN_CATEGORY_ANY_CHARACTER                         },
      { '\\',  regular_context_escape, TOKEN_CATEGORY_METACHARACTER_ESCAPE                  },
      { EMPTY, regular_context,        TOKEN_CATEGORY_EMPTY_EXPRESSION_MARKER               },
      { ANY,   regular_context,        TOKEN_CATEGORY_ORDINARY_CHARACTER                    }
    };
  g_autoptr (GPtrArray) regular_context_transitions =
    lexer_create_transitions_from (regular_context_mappings,
                                   G_N_ELEMENTS (regular_context_mappings));

  MealyMapping regular_context_escape_mappings[] =
    {
      { ANY, regular_context, TOKEN_CATEGORY_ORDINARY_CHARACTER }
    };
  g_autoptr (GPtrArray) regular_context_escape_transitions =
    lexer_create_transitions_from (regular_context_escape_mappings,
                                   G_N_ELEMENTS (regular_context_escape_mappings));

  MealyMapping bracket_context_mappings[] =
    {
      { '-',   bracket_context,        TOKEN_CATEGORY_RANGE_OPERATOR       },
      { ']',   regular_context,        TOKEN_CATEGORY_CLOSE_BRACKET        },
      { '\\',  bracket_context_escape, TOKEN_CATEGORY_METACHARACTER_ESCAPE },
      { EMPTY, bracket_context,        TOKEN_CATEGORY_UNDEFINED            },
      { ANY,   bracket_context,        TOKEN_CATEGORY_ORDINARY_CHARACTER   }
    };
  g_autoptr (GPtrArray) bracket_context_transitions =
    lexer_create_transitions_from (bracket_context_mappings,
                                   G_N_ELEMENTS (bracket_context_mappings));

  MealyMapping bracket_context_escape_mappings[] =
    {
      { ANY, bracket_context, TOKEN_CATEGORY_ORDINARY_CHARACTER }
    };
  g_autoptr (GPtrArray) bracket_context_escape_transitions =
    lexer_create_transitions_from (bracket_context_escape_mappings,
                                   G_N_ELEMENTS (bracket_context_escape_mappings));

  g_object_set (regular_context,
                PROP_STATE_TRANSITIONS, regular_context_transitions,
                NULL);
  g_object_set (regular_context_escape,
                PROP_STATE_TRANSITIONS, regular_context_escape_transitions,
                NULL);
  g_object_set (bracket_context,
                PROP_STATE_TRANSITIONS, bracket_context_transitions,
                NULL);
  g_object_set (bracket_context_escape,
                PROP_STATE_TRANSITIONS, bracket_context_escape_transitions,
                NULL);

  g_ptr_array_add_multiple (all_states,
                            regular_context, regular_context_escape, bracket_context, bracket_context_escape,
                            NULL);

  return mealy_new (PROP_FSM_INITIALIZABLE_ALL_STATES, all_states);
}

static GPtrArray *
lexer_create_transitions_from (MealyMapping *mappings,
                               gsize         mappings_count)
{
  GPtrArray *transitions = g_ptr_array_new_full ((guint) mappings_count, g_object_unref);

  for (guint i = 0; i < mappings_count; ++i)
    {
      MealyMapping mapping = mappings[i];
      gchar expected_character = mapping.character;
      State *output_state = mapping.next_state;
      gpointer output_data = GINT_TO_POINTER (mapping.token_category);
      Transition *mealy_transition = create_mealy_transition (expected_character,
                                                              output_state,
                                                              output_data);

      g_ptr_array_add (transitions, mealy_transition);
    }

  return transitions;
}

static void
lexer_report_error (const gchar  *expression,
                    GError      **error)
{
  LexicalAnalysisLexerError error_code = LEXICAL_ANALYSIS_LEXER_ERROR_UNDEFINED;
  const gchar *error_message = NULL;

  if (expression == NULL)
    {
      error_code = LEXICAL_ANALYSIS_LEXER_ERROR_INPUT_NULL;
      error_message = "The expression must not be NULL";
    }
  else if (!g_str_is_ascii (expression))
    {
      error_code = LEXICAL_ANALYSIS_LEXER_ERROR_INPUT_NOT_ASCII;
      error_message = "The expression must be an ASCII string";
    }

  if (error_message != NULL)
    {
      g_set_error (error,
                   LEXICAL_ANALYSIS_LEXER_ERROR,
                   error_code,
                   error_message);
    }
}

static GString *
lexer_normalize_expression (const gchar *expression)
{
  g_autofree gchar *normalized_expression = NULL;
  gchar **simple_normalizers[] =
    {
      (gchar*[]) { "",  "%c"  },
      (gchar*[]) { "^", "^%c" },
      (gchar*[]) { "$", "%c$" }
    };
  gsize simple_normalizers_count = G_N_ELEMENTS (simple_normalizers);

  for (guint i = 0; i < simple_normalizers_count; ++i)
    {
      gchar **simple_normalizer = simple_normalizers[i];

      if (g_strcmp0 (expression, simple_normalizer[0]) == 0)
        {
          normalized_expression = g_strdup_printf (simple_normalizer[1], EMPTY);

          break;
        }
    }

  if (normalized_expression != NULL)
    {
      return g_string_new (normalized_expression);
    }
  else
    {
      GString *expression_string = g_string_new (expression);
      ExpressionNormalizer normalizers[] =
        {
          { "^$",  G_N_ELEMENTS ("^$") - 1,  g_strdup_printf ("^%c$", EMPTY)  },
          { "()",  G_N_ELEMENTS ("()") - 1,  g_strdup_printf ("(%c)", EMPTY)  },
          { "(^)", G_N_ELEMENTS ("(^)") - 1, g_strdup_printf ("(^%c)", EMPTY) },
          { "($)", G_N_ELEMENTS ("($)") - 1, g_strdup_printf ("(%c$)", EMPTY) }
        };
      gsize normalizers_count = G_N_ELEMENTS (normalizers);

      for (guint i = 0; i < normalizers_count; ++i)
        {
          ExpressionNormalizer normalizer = normalizers[i];
          gchar *search_result = NULL;

          while ((search_result = g_strrstr (expression_string->str, normalizer.original_substring)) != NULL)
            {
              gulong position = search_result - expression_string->str;

              g_string_erase (expression_string,
                              position,
                              normalizer.original_length);
              g_string_insert (expression_string,
                               position,
                               normalizer.replacement_substring);
            }

          g_free (normalizer.replacement_substring);
        }

      return expression_string;
    }
}

static void
lexer_dispose (GObject *object)
{
  LexerPrivate *priv = lexer_get_instance_private (LEXICAL_ANALYSIS_LEXER (object));

  if (priv->transducer != NULL)
    g_clear_object (&priv->transducer);

  G_OBJECT_CLASS (lexer_parent_class)->dispose (object);
}
