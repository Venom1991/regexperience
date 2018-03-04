#include "internal/syntactic_analysis/lexer.h"
#include "internal/syntactic_analysis/lexeme.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transducers/transducer_runnable.h"
#include "internal/state_machines/transducers/mealy.h"
#include "internal/state_machines/transitions/transition_factory.h"
#include "internal/common/helpers.h"
#include "internal/common/macros.h"
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

static Token            *lexer_create_token            (TokenCategory  category,
                                                        gchar         *content,
                                                        guint         *character_position);

static FsmInitializable *lexer_build_transducer        (void);

static GPtrArray        *lexer_create_transitions_from (MealyMapping  *mappings,
                                                        gsize          mappings_size);

static void              lexer_report_error_if_needed  (const gchar   *regular_expression,
                                                        GError       **error);

static void              lexer_dispose                 (GObject       *object);

G_DEFINE_QUARK (syntactic-analysis-lexer-error-quark, syntactic_analysis_lexer_error)
#define SYNTACTIC_ANALYSIS_LEXER_ERROR (syntactic_analysis_lexer_error_quark ())

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
                const gchar  *regular_expression,
                GError      **error)
{
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_LEXER (self), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  LexerPrivate *priv = lexer_get_instance_private (self);

  TransducerRunnable *transducer = priv->transducer;
  GPtrArray *tokens = g_ptr_array_new_with_free_func (g_object_unref);
  guint character_position = 1;
  GError *temporary_error = NULL;

  lexer_report_error_if_needed (regular_expression, &temporary_error);

  if (temporary_error != NULL)
    {
      g_propagate_error (error, temporary_error);

      return NULL;
    }

  transducer_runnable_reset (transducer);

  while (TRUE)
    {
      gchar current_character = *regular_expression++;

      if (current_character == '\0')
        break;

      TokenCategory category = (TokenCategory) GPOINTER_TO_INT (transducer_runnable_run (transducer,
                                                                                         current_character));
      g_autofree gchar *content = g_strdup_printf ("%c", current_character);
      Token *current_token = lexer_create_token (category,
                                                 content,
                                                 &character_position);

      g_ptr_array_add (tokens, current_token);
    }

  /* Appending the end of input marker. */
  Token *end_of_input_marker = lexer_create_token (TOKEN_CATEGORY_END_OF_INPUT_MARKER,
                                                   EOI,
                                                   &character_position);

  g_ptr_array_add (tokens, end_of_input_marker);

  return tokens;
}

static Token*
lexer_create_token (TokenCategory  category,
                    gchar         *content,
                    guint         *character_position)
{
  g_autoptr (Lexeme) lexeme = NULL;
  g_autoptr (GString) lexeme_content = g_string_new (content);
  gsize lexeme_content_length = lexeme_content->len;

  lexeme = lexeme_new (PROP_LEXEME_CONTENT, lexeme_content,
                       PROP_LEXEME_START_POSITION, *character_position,
                       PROP_LEXEME_END_POSITION, *character_position + (lexeme_content_length - 1));

  *character_position += lexeme_content_length;

  return token_new (PROP_TOKEN_CATEGORY, category,
                    PROP_TOKEN_LEXEME, lexeme);
}

static FsmInitializable *
lexer_build_transducer (void)
{
  g_autoptr (GPtrArray) all_states = g_ptr_array_new_with_free_func (g_object_unref);
  State *regular_character = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_START);
  State *regular_escape = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT);
  State *bracket_expression_character = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT);
  State *bracket_expression_escape = state_new (PROP_STATE_TYPE_FLAGS, STATE_TYPE_DEFAULT);

  MealyMapping regular_metacharacter_mappings[] =
  {
    { '[',  bracket_expression_character, TOKEN_CATEGORY_OPEN_BRACKET,                         },
    { '(',  regular_character,            TOKEN_CATEGORY_OPEN_PARENTHESIS                      },
    { ')',  regular_character,            TOKEN_CATEGORY_CLOSE_PARENTHESIS                     },
    { '*',  regular_character,            TOKEN_CATEGORY_STAR_QUANTIFICATION_OPERATOR          },
    { '+',  regular_character,            TOKEN_CATEGORY_PLUS_QUANTIFICATION_OPERATOR          },
    { '?',  regular_character,            TOKEN_CATEGORY_QUESTION_MARK_QUANTIFICATION_OPERATOR },
    { '|',  regular_character,            TOKEN_CATEGORY_ALTERNATION_OPERATOR                  },
    { '\\', regular_escape,               TOKEN_CATEGORY_METACHARACTER_ESCAPE                  },
    { 0,    regular_character,            TOKEN_CATEGORY_ORDINARY_CHARACTER                    }
  };
  g_autoptr (GPtrArray) regular_transitions =
    lexer_create_transitions_from (regular_metacharacter_mappings,
                                   G_N_ELEMENTS (regular_metacharacter_mappings));

  MealyMapping regular_escape_mappings[] =
  {
    { 0, regular_character, TOKEN_CATEGORY_ORDINARY_CHARACTER }
  };
  g_autoptr (GPtrArray) regular_escape_transitions =
    lexer_create_transitions_from (regular_escape_mappings,
                                   G_N_ELEMENTS (regular_escape_mappings));

  MealyMapping bracket_expression_mappings[] =
  {
    { '-',  bracket_expression_character, TOKEN_CATEGORY_RANGE_OPERATOR       },
    { ']',  regular_character,            TOKEN_CATEGORY_CLOSE_BRACKET        },
    { '\\', bracket_expression_escape,    TOKEN_CATEGORY_METACHARACTER_ESCAPE },
    { 0,    bracket_expression_character, TOKEN_CATEGORY_ORDINARY_CHARACTER   }
  };
  g_autoptr (GPtrArray) bracket_expression_transitions =
    lexer_create_transitions_from (bracket_expression_mappings,
                                   G_N_ELEMENTS (bracket_expression_mappings));

  MealyMapping bracket_expression_escape_mappings[] =
  {
    { 0, bracket_expression_character, TOKEN_CATEGORY_ORDINARY_CHARACTER }
  };
  g_autoptr (GPtrArray) bracket_expression_escape_transitions =
    lexer_create_transitions_from (bracket_expression_escape_mappings,
                                   G_N_ELEMENTS (bracket_expression_escape_mappings));

  g_object_set (regular_character,
                PROP_STATE_TRANSITIONS, regular_transitions,
                NULL);
  g_object_set (regular_escape,
                PROP_STATE_TRANSITIONS, regular_escape_transitions,
                NULL);
  g_object_set (bracket_expression_character,
                PROP_STATE_TRANSITIONS, bracket_expression_transitions,
                NULL);
  g_object_set (bracket_expression_escape,
                PROP_STATE_TRANSITIONS, bracket_expression_escape_transitions,
                NULL);

  g_ptr_array_add_multiple (all_states,
                            regular_character, regular_escape, bracket_expression_character, bracket_expression_escape,
                            NULL);

  return mealy_new (PROP_FSM_INITIALIZABLE_ALL_STATES, all_states);
}

static GPtrArray *
lexer_create_transitions_from (MealyMapping *mappings,
                               gsize         mappings_size)
{
  GPtrArray *transitions = g_ptr_array_new_with_free_func (g_object_unref);
  const gchar uninitialized_character = 0;

  for (guint i = 0; i < mappings_size; ++i)
    {
      MealyMapping mapping = mappings[i];
      gchar expected_character = mapping.character;
      State *output_state = mapping.next_state;
      gpointer output_data = GINT_TO_POINTER (mapping.token_category);
      Transition *transition = NULL;

      if (expected_character != uninitialized_character)
        transition = create_mealy_transition (expected_character,
                                              output_state,
                                              output_data);
      else
        transition = create_mealy_unconditional_transition (output_state,
                                                            output_data);

      g_ptr_array_add (transitions, transition);
    }

  return transitions;
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

static void lexer_dispose (GObject *object)
{
  LexerPrivate *priv = lexer_get_instance_private (SYNTACTIC_ANALYSIS_LEXER (object));

  if (priv->transducer != NULL)
    g_clear_object (&priv->transducer);

  G_OBJECT_CLASS (lexer_parent_class)->dispose (object);
}
