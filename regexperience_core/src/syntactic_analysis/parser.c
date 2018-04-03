#include "internal/syntactic_analysis/parser.h"
#include "internal/syntactic_analysis/derivation_item.h"
#include "internal/syntactic_analysis/lexeme.h"
#include "internal/syntactic_analysis/rule.h"
#include "internal/syntactic_analysis/symbols/terminal.h"
#include "internal/syntactic_analysis/symbols/non_terminal.h"
#include "internal/common/helpers.h"
#include "core/errors.h"

struct _Parser
{
    GObject parent_instance;
};

typedef struct
{
    GPtrArray  *analysis_queues;
    GPtrArray  *prediction_queues;
    Grammar    *grammar;
} ParserPrivate;

typedef enum
{
    FILL_QUEUES_MODE_INITIALIZATION,
    FILL_QUEUES_MODE_EXPANSION
} FillQueuesMode;

static void      parser_prepare_for_parsing        (Parser          *self);

static void      parser_fill_queues                (GPtrArray       *analysis_queues,
                                                    GQueue          *analysis_queue_for_copy,
                                                    GPtrArray       *prediction_queues,
                                                    GQueue          *prediction_queue_for_copy,
                                                    Production      *production,
                                                    FillQueuesMode   mode);

static void      parser_remove_queues              (GPtrArray       *analysis_queues,
                                                    GArray          *analyses_for_removal,
                                                    GPtrArray       *prediction_queues,
                                                    GArray          *predictions_for_removal);

static void      parser_predict                    (GPtrArray       *analysis_queues,
                                                    GPtrArray       *prediction_queues);

static gboolean  parser_can_make_prediction        (GPtrArray       *prediction_queues,
                                                    GArray         **indexes_for_expansion);

static void      parser_discard                    (GPtrArray       *analysis_queues,
                                                    GPtrArray       *prediction_queues,
                                                    Token           *token);

static gboolean  parser_can_accept                 (GPtrArray       *analysis_queues,
                                                    GPtrArray       *prediction_queues,
                                                    Token           *token);

static void      parser_match                      (GPtrArray       *analysis_queues,
                                                    GPtrArray       *prediction_queues,
                                                    Token           *token);

static GNode    *parser_transform_analysis         (GPtrArray       *analysis_queues);

static void      parser_insert_children            (GNode           *root,
                                                    GQueue          *remaining_analysis_queue);

static void      parser_report_error_if_needed     (Token           *current_token,
                                                    guint            token_position,
                                                    GPtrArray       *all_tokens,
                                                    guint            prediction_queues_count,
                                                    GError         **error);

static gboolean  parser_token_exists_in_all_tokens (GPtrArray       *all_tokens,
                                                    TokenCategory    category,
                                                    guint            starting_position,
                                                    Token          **found_token);

static void      parser_dispose                    (GObject         *object);

G_DEFINE_QUARK (syntactic-analysis-parser-error-quark, syntactic_analysis_parser_error)
#define SYNTACTIC_ANALYSIS_PARSER_ERROR (syntactic_analysis_parser_error_quark ())

G_DEFINE_TYPE_WITH_PRIVATE (Parser, parser, G_TYPE_OBJECT)

static void
parser_class_init (ParserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = parser_dispose;
}

static void
parser_init (Parser *self)
{
  ParserPrivate *priv = parser_get_instance_private (SYNTACTIC_ANALYSIS_PARSER (self));
  GPtrArray *analysis_queues = g_ptr_array_new_with_free_func ((GDestroyNotify) g_queue_free_g_objects);
  GPtrArray *prediction_queues = g_ptr_array_new_with_free_func ((GDestroyNotify) g_queue_free);
  Grammar *grammar = grammar_new ();

  priv->analysis_queues = analysis_queues;
  priv->prediction_queues = prediction_queues;
  priv->grammar = grammar;
}

GNode *
parser_build_concrete_syntax_tree (Parser     *self,
                                   GPtrArray  *tokens,
                                   GError    **error)
{
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_PARSER (self), NULL);
  g_return_val_if_fail (g_ptr_array_has_items (tokens), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  parser_prepare_for_parsing (self);

  ParserPrivate *priv = parser_get_instance_private (self);
  GPtrArray *analysis_queues = priv->analysis_queues;
  GPtrArray *prediction_queues = priv->prediction_queues;
  GNode *concrete_syntax_tree = NULL;
  guint token_position = 0;
  GError *temporary_error = NULL;

  /* Breadth-first top-down parsing. */
  while (TRUE)
    {
      Token *token = g_ptr_array_index (tokens, token_position);

      /* Expanding the prediction queues until a terminal symbol appears at
       * each of their heads. Analysis queues keep track of the productions and
       * rules used during the expansion itself.
       */
      parser_predict (analysis_queues, prediction_queues);

      /* Discarding predictions and their corresponding analyses
       * that cannot possibly lead to a correct parse.
       */
      parser_discard (analysis_queues, prediction_queues, token);

      /* Checking if the input is exhausted as well as only one
       * prediction queue remaining with the end of input marker located at its head.
       * In case these two conditions are met the last token is pushed to
       * the remaining analysis queue, as well.
       */
      if (parser_can_accept (analysis_queues, prediction_queues, token))
        {
          /* Transforming the last remaining analysis queue (which actually
           * represents a leftmost derivation of the input, in reverse) into a
           * concrete syntax tree.
           */
          concrete_syntax_tree = parser_transform_analysis (analysis_queues);

          break;
        }

      /* Reporting errors. */
      parser_report_error_if_needed (token,
                                     token_position,
                                     tokens,
                                     prediction_queues->len,
                                     &temporary_error);

      if (temporary_error != NULL)
        {
          g_propagate_error (error, temporary_error);

          return NULL;
        }

      /* Removing terminals from all the current prediction queues' heads
       * so as to enable further predictions.
       * The current token is also pushed to all the current analysis queues.
       */
      parser_match (analysis_queues, prediction_queues, token);

      token_position++;
    }

  return concrete_syntax_tree;
}

static void
parser_prepare_for_parsing (Parser *self)
{
  ParserPrivate *priv = parser_get_instance_private (SYNTACTIC_ANALYSIS_PARSER (self));
  GPtrArray *analysis_queues = priv->analysis_queues;
  GPtrArray *prediction_queues = priv->prediction_queues;
  Grammar *grammar = priv->grammar;

  g_ptr_array_set_size (analysis_queues, 0);
  g_ptr_array_set_size (prediction_queues, 0);

  g_autoptr (GHashTable) productions = NULL;
  const gchar *start_production_caption = START;

  g_object_get (grammar,
                PROP_GRAMMAR_PRODUCTIONS, &productions,
                NULL);

  Production *start = g_hash_table_lookup (productions, start_production_caption);

  /* Initializing the analysis and prediction queues using the start production. */
  parser_fill_queues (analysis_queues,
                      NULL,
                      prediction_queues,
                      NULL,
                      start,
                      FILL_QUEUES_MODE_INITIALIZATION);
}

static void
parser_fill_queues (GPtrArray      *analysis_queues,
                    GQueue         *analysis_queue_for_copy,
                    GPtrArray      *prediction_queues,
                    GQueue         *prediction_queue_for_copy,
                    Production     *production,
                    FillQueuesMode  mode)
{
  g_return_if_fail
  (
    (mode == FILL_QUEUES_MODE_INITIALIZATION &&
     (analysis_queue_for_copy == NULL && prediction_queue_for_copy == NULL)) ||
    (mode == FILL_QUEUES_MODE_EXPANSION &&
     (analysis_queue_for_copy != NULL && prediction_queue_for_copy != NULL))
  );

  g_autoptr (GPtrArray) rules = NULL;

  g_object_get (production,
                PROP_PRODUCTION_RULES, &rules,
                NULL);

  for (guint i = rules->len - 1; i != G_MAXUINT; --i)
    {
      Rule *rule = g_ptr_array_index (rules, i);
      g_autoptr (GPtrArray) symbols = NULL;

      g_object_get (rule,
                    PROP_RULE_SYMBOLS, &symbols,
                    NULL);

      GQueue *new_analysis_queue = NULL;
      GQueue *new_prediction_queue = NULL;

      switch (mode)
        {
        case FILL_QUEUES_MODE_INITIALIZATION:
          new_analysis_queue = g_queue_new ();
          new_prediction_queue = g_queue_new ();
          break;

        case FILL_QUEUES_MODE_EXPANSION:
          new_analysis_queue = g_queue_copy_g_objects (analysis_queue_for_copy);
          new_prediction_queue = g_queue_copy (prediction_queue_for_copy);
          break;
        }

      DerivationItem *derivation_item = derivation_item_new (PROP_DERIVATION_ITEM_LEFT_HAND_SIDE, production,
                                                             PROP_DERIVATION_ITEM_RIGHT_HAND_SIDE, rule);

      g_queue_push_tail (new_analysis_queue, derivation_item);

      for (guint j = symbols->len - 1; j != G_MAXUINT; --j)
        {
          Symbol *symbol = g_ptr_array_index (symbols, j);

          g_queue_push_head (new_prediction_queue, symbol);
        }

      g_ptr_array_add (analysis_queues, new_analysis_queue);
      g_ptr_array_add (prediction_queues, new_prediction_queue);
    }
}

static void
parser_remove_queues (GPtrArray *analysis_queues,
                      GArray    *analyses_for_removal,
                      GPtrArray *prediction_queues,
                      GArray    *predictions_for_removal)
{
  g_assert (analyses_for_removal->len == predictions_for_removal->len);

  guint removal_count = analyses_for_removal->len = predictions_for_removal->len;

  for (guint i = 0; i < removal_count; ++i)
    {
      GQueue *analysis_for_removal = g_array_index (analyses_for_removal, gpointer, i);
      GQueue *prediction_for_removal = g_array_index (predictions_for_removal, gpointer, i);

      g_ptr_array_remove_fast (analysis_queues, analysis_for_removal);
      g_ptr_array_remove_fast (prediction_queues, prediction_for_removal);
    }
}

static void
parser_predict (GPtrArray  *analysis_queues,
                GPtrArray  *prediction_queues)
{
  g_assert (analysis_queues->len == prediction_queues->len);

  g_autoptr (GArray) indexes_for_expansion = NULL;

  /* Checking whether or not any of the prediction queues' heads are non-terminals. */
  if (!parser_can_make_prediction (prediction_queues, &indexes_for_expansion))
    return;

  g_autoptr (GPtrArray) expansion_analysis_queues = g_ptr_array_new ();
  g_autoptr (GPtrArray) expansion_prediction_queues = g_ptr_array_new ();
  g_autoptr (GArray) analyses_for_removal = g_array_new (FALSE, FALSE, sizeof (gpointer));
  g_autoptr (GArray) predictions_for_removal = g_array_new (FALSE, FALSE, sizeof (gpointer));

  for (guint i = 0; i < indexes_for_expansion->len; ++i)
    {
      guint index_for_expansion = g_array_index (indexes_for_expansion, guint, i);
      GQueue *analysis_queue = g_ptr_array_index (analysis_queues, index_for_expansion);
      GQueue *prediction_queue = g_ptr_array_index (prediction_queues, index_for_expansion);
      Symbol *prediction_head = g_queue_pop_head (prediction_queue);

      g_assert (SYMBOLS_IS_NON_TERMINAL (prediction_head));

      g_array_append_val (analyses_for_removal, analysis_queue);
      g_array_append_val (predictions_for_removal, prediction_queue);

      g_autoptr (Production) production = NULL;
      GValue value = G_VALUE_INIT;

      symbol_extract_value (prediction_head, &value);

      production = g_value_get_object (&value);

      parser_fill_queues (expansion_analysis_queues,
                          analysis_queue,
                          expansion_prediction_queues,
                          prediction_queue,
                          production,
                          FILL_QUEUES_MODE_EXPANSION);
    }

  /* Removing queues that need to be replaced with their expanded versions. */
  parser_remove_queues (analysis_queues,
                        analyses_for_removal,
                        prediction_queues,
                        predictions_for_removal);

  g_assert (expansion_analysis_queues->len == expansion_prediction_queues->len);

  /* Combining original queues with expansions. */
  g_ptr_array_add_range (analysis_queues,
                         expansion_analysis_queues,
                         NULL);
  g_ptr_array_add_range (prediction_queues,
                         expansion_prediction_queues,
                         NULL);

  parser_predict (analysis_queues, prediction_queues);
}

static gboolean
parser_can_make_prediction (GPtrArray  *prediction_queues,
                            GArray    **indexes_for_expansion)
{
  guint prediction_queues_count = prediction_queues->len;

  GArray *local_indexes_for_expansion = NULL;

  for (guint i = 0; i < prediction_queues_count; ++i)
    {
      GQueue *prediction_queue = g_ptr_array_index (prediction_queues, i);
      Symbol *prediction_head = g_queue_peek_head (prediction_queue);

      if (SYMBOLS_IS_NON_TERMINAL (prediction_head))
        {
          if (local_indexes_for_expansion == NULL)
              local_indexes_for_expansion = g_array_new (FALSE, FALSE, sizeof (guint));

           g_array_append_val (local_indexes_for_expansion, i);
        }
    }

  *indexes_for_expansion = local_indexes_for_expansion;

  return local_indexes_for_expansion != NULL;
}

static void
parser_discard (GPtrArray *analysis_queues,
                GPtrArray *prediction_queues,
                Token     *token)
{
  g_assert (analysis_queues->len == prediction_queues->len);

  guint queues_count = analysis_queues->len = prediction_queues->len;
  g_autoptr (GArray) analyses_for_removal = g_array_new (FALSE, FALSE, sizeof (gpointer));
  g_autoptr (GArray) predictions_for_removal = g_array_new (FALSE, FALSE, sizeof (gpointer));
  g_autoptr (Lexeme) lexeme = NULL;
  g_autoptr (GString) lexeme_content = NULL;
  gchar *value = NULL;

  g_object_get (token,
                PROP_TOKEN_LEXEME, &lexeme,
                NULL);
  g_object_get (lexeme,
                PROP_LEXEME_CONTENT, &lexeme_content,
                NULL);

  value = lexeme_content->str;

  for (guint i = 0; i < queues_count; ++i)
    {
      GQueue *analysis_queue = g_ptr_array_index (analysis_queues, i);
      GQueue *prediction_queue = g_ptr_array_index (prediction_queues, i);
      Symbol *prediction_head = g_queue_peek_head (prediction_queue);

      g_assert (SYMBOLS_IS_TERMINAL (prediction_head));

      if (!symbol_is_match (prediction_head, value, symbol_value_type (value)))
        {
          g_array_append_val (analyses_for_removal, analysis_queue);
          g_array_append_val (predictions_for_removal, prediction_queue);
        }
    }

  parser_remove_queues (analysis_queues,
                        analyses_for_removal,
                        prediction_queues,
                        predictions_for_removal);
}

static gboolean
parser_can_accept (GPtrArray *analysis_queues,
                   GPtrArray *prediction_queues,
                   Token     *token)
{
  g_assert (analysis_queues->len == prediction_queues->len);

  gboolean result = FALSE;
  TokenCategory token_category = TOKEN_CATEGORY_UNDEFINED;

  g_object_get (token,
                PROP_TOKEN_CATEGORY, &token_category,
                NULL);

  if (token_category == TOKEN_CATEGORY_END_OF_INPUT_MARKER)
    {
      guint queues_count = analysis_queues->len = prediction_queues->len;
      const guint acceptance_queues_count = 1;

      if (queues_count == acceptance_queues_count)
       {
          GQueue *remaining_analysis_queue = g_ptr_array_index (analysis_queues, 0);
          GQueue *remaining_prediction_queue = g_ptr_array_index (prediction_queues, 0);
          Symbol *prediction_head = g_queue_peek_head (remaining_prediction_queue);

          if (SYMBOLS_IS_TERMINAL (prediction_head))
            {
              g_autoptr (Lexeme) lexeme = NULL;
              g_autoptr (GString) lexeme_content = NULL;
              gchar *value = NULL;

              g_object_get (token,
                            PROP_TOKEN_LEXEME, &lexeme,
                            NULL);
              g_object_get (lexeme,
                            PROP_LEXEME_CONTENT, &lexeme_content,
                            NULL);

              value = lexeme_content->str;

              if (symbol_is_match (prediction_head, value, symbol_value_type (value)))
                {
                  g_queue_push_tail (remaining_analysis_queue, g_object_ref (token));

                  result = TRUE;
                }
            }
        }
    }

  return result;
}

static void
parser_match (GPtrArray *analysis_queues,
              GPtrArray *prediction_queues,
              Token     *token)
{
  g_assert (analysis_queues->len == prediction_queues->len);

  guint queues_count = analysis_queues->len = prediction_queues->len;

  for (guint i = 0; i < queues_count; ++i)
    {
      GQueue *analysis_queue = g_ptr_array_index (analysis_queues, i);
      GQueue *prediction_queue = g_ptr_array_index (prediction_queues, i);
      Symbol *prediction_head = g_queue_pop_head (prediction_queue);

      g_assert (SYMBOLS_IS_TERMINAL (prediction_head));

      g_queue_push_tail (analysis_queue, g_object_ref (token));
    }
}

static GNode *
parser_transform_analysis (GPtrArray *analysis_queues)
{
  GQueue *remaining_analysis_queue = g_ptr_array_index (analysis_queues, 0);
  GNode *concrete_syntax_tree = NULL;

  g_queue_reverse (remaining_analysis_queue);

  DerivationItem *start_derivation_item = g_queue_peek_tail (remaining_analysis_queue);
  g_autoptr (Production) start_left_hand_side = NULL;

  g_object_get (start_derivation_item,
                PROP_DERIVATION_ITEM_LEFT_HAND_SIDE, &start_left_hand_side,
                NULL);

  Symbol *symbol = non_terminal_new (PROP_SYMBOL_VALUE, start_left_hand_side);

  concrete_syntax_tree = g_node_new (symbol);

  parser_insert_children (concrete_syntax_tree, remaining_analysis_queue);

  return concrete_syntax_tree;
}

static void
parser_insert_children (GNode  *root,
                        GQueue *remaining_analysis_queue)
{
  g_assert (!g_queue_is_empty (remaining_analysis_queue));

  g_autoptr (DerivationItem) derivation_item = g_queue_pop_tail (remaining_analysis_queue);
  g_autoptr (Rule) right_hand_side = NULL;
  g_autoptr (GPtrArray) symbols = NULL;

  g_object_get (derivation_item,
                PROP_DERIVATION_ITEM_RIGHT_HAND_SIDE, &right_hand_side,
                NULL);
  g_object_get (right_hand_side,
                PROP_RULE_SYMBOLS, &symbols,
                NULL);

  for (guint i = 0; i < symbols->len; ++i)
    {
      Symbol *symbol = g_ptr_array_index (symbols, i);
      GNode *child = NULL;

      /* Terminals need to be exchanged with their equivalent
       * tokens (which are guaranteed to be positioned right after them)
       * as these objects are more useful in the succeeding stages
       * of processing.
       */
      if (SYMBOLS_IS_TERMINAL (symbol))
        {
          Token *token = g_queue_pop_tail (remaining_analysis_queue);

          child = g_node_new (token);

          g_node_insert (root, i, child);
        }
      /* Non-terminals need to be transformed further, recursively. */
      else if (SYMBOLS_IS_NON_TERMINAL (symbol))
        {
          child = g_node_new (g_object_ref (symbol));

          g_node_insert (root, i, child);

          parser_insert_children (child, remaining_analysis_queue);
        }
    }
}

static
void parser_report_error_if_needed (Token      *current_token,
                                    guint       token_position,
                                    GPtrArray  *all_tokens,
                                    guint       prediction_queues_count,
                                    GError    **error)
{
  const guint syntax_error_prediction_queues_count = 0;
  TokenCategory token_category = TOKEN_CATEGORY_UNDEFINED;
  Token *invalid_token = NULL;
  SyntacticAnalysisParserError error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNDEFINED;
  const gchar *error_message = NULL;

  /* Trying to discern an informative error message in case there are no possible predictions left. */
  if (prediction_queues_count == syntax_error_prediction_queues_count)
    {
      g_object_get (current_token,
                    PROP_TOKEN_CATEGORY, &token_category,
                    NULL);

      if (token_category == TOKEN_CATEGORY_END_OF_INPUT_MARKER)
        {
          Token *found_token = NULL;

          if (parser_token_exists_in_all_tokens (all_tokens,
                                                 TOKEN_CATEGORY_OPEN_PARENTHESIS,
                                                 token_position,
                                                 &found_token))
            {
              invalid_token = found_token;

              goto unmatched_open_parenthesis;
            }
          else if (parser_token_exists_in_all_tokens (all_tokens,
                                                      TOKEN_CATEGORY_OPEN_BRACKET,
                                                      token_position,
                                                      &found_token))
            {
              invalid_token = found_token;

              goto unmatched_open_bracket;
            }
          else if (parser_token_exists_in_all_tokens (all_tokens,
                                                      TOKEN_CATEGORY_ALTERNATION_OPERATOR,
                                                      token_position,
                                                      &found_token))
            {
              invalid_token = found_token;

              goto dangling_alternation_operator;
            }
          else if (parser_token_exists_in_all_tokens (all_tokens,
                                                      TOKEN_CATEGORY_METACHARACTER_ESCAPE,
                                                      token_position,
                                                      &found_token))
            {
              invalid_token = found_token;

              goto dangling_metacharacter_escape;
            }
        }
      else if (token_category == TOKEN_CATEGORY_CLOSE_PARENTHESIS)
      {
          Token *found_token = NULL;

          if (parser_token_exists_in_all_tokens (all_tokens,
                                                 TOKEN_CATEGORY_ALTERNATION_OPERATOR,
                                                 token_position,
                                                 &found_token))
            {
              invalid_token = found_token;

              goto dangling_alternation_operator;
            }
          else if (parser_token_exists_in_all_tokens (all_tokens,
                                                      TOKEN_CATEGORY_OPEN_PARENTHESIS,
                                                      token_position,
                                                      &found_token))
            {
              invalid_token = found_token;
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_EMPTY_GROUP;
              error_message = "Empty groups are not allowed";
            }
      }
      else if (token_category == TOKEN_CATEGORY_CLOSE_BRACKET)
        {
          Token *found_token = NULL;

          if (parser_token_exists_in_all_tokens (all_tokens,
                                                 TOKEN_CATEGORY_RANGE_OPERATOR,
                                                 token_position,
                                                 &found_token))
            {
              invalid_token = found_token;

              goto dangling_range_operator;
            }
          else if (parser_token_exists_in_all_tokens (all_tokens,
                                                      TOKEN_CATEGORY_OPEN_BRACKET,
                                                      token_position,
                                                      &found_token))
            {
              invalid_token = found_token;
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_EMPTY_BRACKET_EXPRESSION;
              error_message = "Empty bracket expressions are not allowed";
            }
        }

      if (invalid_token == NULL)
        {
          invalid_token = current_token;

          switch (token_category)
            {
            case TOKEN_CATEGORY_ALTERNATION_OPERATOR:
            dangling_alternation_operator:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_DANGLING_ALTERNATION_OPERATOR;
              error_message = "Dangling alternation operator";
              break;

            case TOKEN_CATEGORY_RANGE_OPERATOR:
            dangling_range_operator:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_DANGLING_RANGE_OPERATOR;
              error_message = "Dangling range operator";
              break;

            case TOKEN_CATEGORY_METACHARACTER_ESCAPE:
            dangling_metacharacter_escape:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_DANGLING_METACHARACTER_ESCAPE;
              error_message = "Dangling escape character";
              break;

            case TOKEN_CATEGORY_OPEN_PARENTHESIS:
            unmatched_open_parenthesis:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNMATCHED_OPEN_PARENTHESIS;
              error_message = "Unmatched open parenthesis";
              break;

            case TOKEN_CATEGORY_OPEN_BRACKET:
            unmatched_open_bracket:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNMATCHED_OPEN_BRACKET;
              error_message = "Unmatched open bracket";
              break;

            case TOKEN_CATEGORY_STAR_QUANTIFICATION_OPERATOR:
            case TOKEN_CATEGORY_PLUS_QUANTIFICATION_OPERATOR:
            case TOKEN_CATEGORY_QUESTION_MARK_QUANTIFICATION_OPERATOR:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_DANGLING_QUANTIFICATION_METACHARACTER;
              error_message = "Dangling quantification operator";
              break;

            case TOKEN_CATEGORY_CLOSE_PARENTHESIS:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNMATCHED_CLOSE_PARENTHESIS;
              error_message = "Unmatched close parenthesis";
              break;

            default:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNEXPECTED_CHARACTER;
              error_message = "Unexpected character";
              break;
            }
        }
    }

  if (invalid_token != NULL)
    {
      g_autoptr (Lexeme) lexeme = NULL;
      guint start_position = 0;
      guint end_position = 0;
      guint position = 0;

      g_object_get (invalid_token,
                    PROP_TOKEN_LEXEME, &lexeme,
                    NULL);
      g_object_get (lexeme,
                    PROP_LEXEME_START_POSITION, &start_position,
                    PROP_LEXEME_END_POSITION, &end_position,
                    NULL);

      position = (start_position + end_position) / 2;

      g_set_error (error,
                   SYNTACTIC_ANALYSIS_PARSER_ERROR,
                   error_code,
                   "%s (position - %d)", error_message, position);
    }
}

static gboolean
parser_token_exists_in_all_tokens (GPtrArray      *all_tokens,
                                   TokenCategory   category,
                                   guint           starting_position,
                                   Token         **found_token)
{
  for (guint i = starting_position; i != G_MAXUINT; --i)
    {
      Token *token = g_ptr_array_index (all_tokens, i);
      TokenCategory token_category = TOKEN_CATEGORY_UNDEFINED;

      g_object_get (token,
                    PROP_TOKEN_CATEGORY, &token_category,
                    NULL);

      if (token_category == category)
        {
          *found_token = token;

          return TRUE;
        }
    }

  return FALSE;
}

static void
parser_dispose (GObject *object)
{
  ParserPrivate *priv = parser_get_instance_private (SYNTACTIC_ANALYSIS_PARSER (object));

  if (priv->analysis_queues != NULL)
    g_clear_pointer (&priv->analysis_queues, g_ptr_array_unref);

  if (priv->prediction_queues != NULL)
    g_clear_pointer (&priv->prediction_queues, g_ptr_array_unref);

  if (priv->grammar != NULL)
    g_clear_object (&priv->grammar);

  G_OBJECT_CLASS (parser_parent_class)->dispose (object);
}
