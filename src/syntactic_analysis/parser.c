#include "internal/syntactic_analysis/parser.h"
#include "internal/syntactic_analysis/grammar.h"
#include "internal/syntactic_analysis/production.h"
#include "internal/syntactic_analysis/rule.h"
#include "internal/syntactic_analysis/derivation_item.h"
#include "internal/syntactic_analysis/parsing_table_key.h"
#include "internal/syntactic_analysis/symbols/terminal.h"
#include "internal/syntactic_analysis/symbols/non_terminal.h"
#include "internal/lexical_analysis/token.h"
#include "internal/lexical_analysis/lexeme.h"
#include "internal/common/helpers.h"
#include "core/errors.h"

struct _Parser
{
  GObject parent_instance;
};

typedef struct
{
  GQueue  *analysis_queue;
  GQueue  *prediction_queue;
  Grammar *grammar;
} ParserPrivate;

static void       parser_prepare_for_parsing        (Parser          *self);

static gboolean   parser_predict                    (Grammar         *grammar,
                                                     Symbol          *non_terminal,
                                                     Token           *token,
                                                     GQueue          *analysis_queue,
                                                     GQueue          *prediction_queue);

static void       parser_expand_queues              (GQueue          *analysis_queue,
                                                     GQueue          *prediction_queue,
                                                     Production      *production,
                                                     Rule            *rule);

static GPtrArray *parser_create_parsing_table_keys  (Grammar         *grammar,
                                                     Symbol          *non_terminal,
                                                     Token           *token,
                                                     Production     **extracted_production);

static gboolean   parser_can_accept                 (Symbol          *terminal,
                                                     Token           *token);

static GNode     *parser_transform_analysis         (GQueue          *analysis_queue);

static void       parser_insert_children            (GNode           *root,
                                                     GQueue          *analysis_queue);

static void       parser_report_error               (guint            token_position,
                                                     GPtrArray       *all_tokens,
                                                     gboolean         parsing_table_entry_found,
                                                     Symbol          *prediction_head,
                                                     GError         **error);

static gboolean   parser_token_exists_in_all_tokens (GPtrArray       *all_tokens,
                                                     TokenCategory    category,
                                                     guint            starting_position,
                                                     Token          **found_token,
                                                     guint           *found_token_position);

static void       parser_dispose                    (GObject         *object);

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
  GQueue *analysis_queue = g_queue_new ();
  GQueue *prediction_queue = g_queue_new ();
  Grammar *grammar = grammar_new ();

  priv->analysis_queue = analysis_queue;
  priv->prediction_queue = prediction_queue;
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

  /* Preparing the prediction and analysis queues using
   * the grammar's start production and its sole rule.
   */
  parser_prepare_for_parsing (self);

  ParserPrivate *priv = parser_get_instance_private (self);
  GQueue *prediction_queue = priv->prediction_queue;
  GQueue *analysis_queue = priv->analysis_queue;
  Grammar *grammar = priv->grammar;
  guint token_position = 0;
  Symbol *prediction_head = g_queue_pop_head (prediction_queue);
  gboolean parsing_table_entry_found = FALSE;
  GNode *concrete_syntax_tree = NULL;

  /* LL(1) parsing. */
  while (TRUE)
    {
      Token *token = g_ptr_array_index (tokens, token_position);
      GError *temporary_error = NULL;

      if (SYMBOLS_IS_TERMINAL (prediction_head))
        {
          /* Skipping the epsilon symbol. */
          if (!symbol_is_epsilon (prediction_head))
            {
              g_queue_push_tail (analysis_queue, g_object_ref (token));

              /* Checking if the input is exhausted by examining whether the
               * current token is an end of input marker and whether or not
               * it is equal to the prediction queue's head.
               */
              if (parser_can_accept (prediction_head, token))
                {
                  /* Transforming the analysis queue (which actually represents
                   * a leftmost derivation of the input, in reverse) into a
                   * concrete syntax tree.
                   */
                  concrete_syntax_tree = parser_transform_analysis (analysis_queue);

                  break;
                }

              /* Moving on to the next token. */
              token_position++;
            }

          prediction_head = g_queue_pop_head (prediction_queue);
        }
      else if (SYMBOLS_IS_NON_TERMINAL (prediction_head))
        {
          /* Trying to find an eligible rule belonging to the prediction head's
           * underlying production.
           * In case it is found both the prediction queue as well as the
           * analysis queue are expanded accordingly.
           */
          parsing_table_entry_found = parser_predict (grammar,
                                                      prediction_head,
                                                      token,
                                                      analysis_queue,
                                                      prediction_queue);

          if (parsing_table_entry_found)
            prediction_head = g_queue_pop_head (prediction_queue);
        }

      /* Reporting errors (if required). */
      parser_report_error (token_position,
                           tokens,
                           parsing_table_entry_found,
                           prediction_head,
                           &temporary_error);

      if (temporary_error != NULL)
        {
          g_propagate_error (error, temporary_error);

          break;
        }
    }

  return concrete_syntax_tree;
}

static void
parser_prepare_for_parsing (Parser *self)
{
  ParserPrivate *priv = parser_get_instance_private (SYNTACTIC_ANALYSIS_PARSER (self));
  GQueue *analysis_queue = priv->analysis_queue;
  GQueue *prediction_queue = priv->prediction_queue;
  Grammar *grammar = priv->grammar;
  g_autoptr (Production) start_production = NULL;
  g_autoptr (GPtrArray) rules = NULL;

  g_queue_clear (analysis_queue);
  g_queue_clear (prediction_queue);

  g_object_get (grammar,
                PROP_GRAMMAR_START_PRODUCTION, &start_production,
                NULL);
  g_object_get (start_production,
                PROP_PRODUCTION_RULES, &rules,
                NULL);

  Rule *first_rule = g_ptr_array_index (rules, 0);

  parser_expand_queues (analysis_queue,
                        prediction_queue,
                        start_production,
                        first_rule);
}

static gboolean
parser_predict (Grammar *grammar,
                Symbol  *non_terminal,
                Token   *token,
                GQueue  *analysis_queue,
                GQueue  *prediction_queue)
{
  g_return_val_if_fail (SYMBOLS_IS_NON_TERMINAL (non_terminal), FALSE);

  Production *extracted_production = NULL;
  g_autoptr (GPtrArray) parsing_table_keys = parser_create_parsing_table_keys (grammar,
                                                                               non_terminal,
                                                                               token,
                                                                               &extracted_production);
  g_autoptr (GHashTable) parsing_table = NULL;

  g_object_get (grammar,
                PROP_GRAMMAR_PARSING_TABLE, &parsing_table,
                NULL);

  for (guint i = 0; i < parsing_table_keys->len; ++i)
    {
      ParsingTableKey *parsing_table_key = g_ptr_array_index (parsing_table_keys, i);
      Rule *found_rule = g_hash_table_lookup (parsing_table, parsing_table_key);

      if (found_rule != NULL)
        {
          parser_expand_queues (analysis_queue,
                                prediction_queue,
                                extracted_production,
                                found_rule);

          return TRUE;
        }
    }

  return FALSE;
}

static void
parser_expand_queues (GQueue     *analysis_queue,
                      GQueue     *prediction_queue,
                      Production *production,
                      Rule       *rule)
{
  g_autoptr (GPtrArray) symbols = NULL;

  g_object_get (rule,
                PROP_RULE_SYMBOLS, &symbols,
                NULL);

  DerivationItem *derivation_item = derivation_item_new (PROP_DERIVATION_ITEM_LEFT_HAND_SIDE, production,
                                                         PROP_DERIVATION_ITEM_RIGHT_HAND_SIDE, rule);

  g_queue_push_tail (analysis_queue, derivation_item);

  for (guint i = symbols->len - 1; i != G_MAXUINT; --i)
    {
      Symbol *symbol = g_ptr_array_index (symbols, i);

      g_queue_push_head (prediction_queue, symbol);
    }
}

static GPtrArray *
parser_create_parsing_table_keys (Grammar     *grammar,
                                  Symbol      *non_terminal,
                                  Token       *token,
                                  Production **extracted_production)
{
  g_return_val_if_fail (extracted_production != NULL, NULL);

  GPtrArray *parsing_table_keys = g_ptr_array_new_with_free_func (g_object_unref);
  g_autoptr (GPtrArray) all_terminals = NULL;
  g_autoptr (Production) production = NULL;
  GValue non_terminal_value = G_VALUE_INIT;
  g_autoptr (Lexeme) lexeme = NULL;
  g_autoptr (GString) lexeme_content = NULL;

  symbol_extract_value (non_terminal, &non_terminal_value);

  production = g_value_get_object (&non_terminal_value);

  g_object_get (grammar,
                PROP_GRAMMAR_ALL_TERMINALS, &all_terminals,
                NULL);
  g_object_get (token,
                PROP_TOKEN_LEXEME, &lexeme,
                NULL);
  g_object_get (lexeme,
                PROP_LEXEME_CONTENT, &lexeme_content,
                NULL);

  /* Multiple parsing table keys are required as certain overlaps
   * exist concerning some of the terminal symbols' values.
   */
  for (guint i = 0; i < all_terminals->len; ++i)
    {
      Symbol *terminal = g_ptr_array_index (all_terminals, i);

      if (symbol_is_match (terminal, lexeme_content->str))
        {
          ParsingTableKey *parsing_table_key = parsing_table_key_new (PROP_PARSING_TABLE_KEY_PRODUCTION, production,
                                                                      PROP_PARSING_TABLE_KEY_TERMINAL, terminal);

          g_ptr_array_add (parsing_table_keys, parsing_table_key);
        }
    }

  *extracted_production = production;

  return parsing_table_keys;
}

static gboolean
parser_can_accept (Symbol *terminal,
                   Token  *token)
{
  g_return_val_if_fail (SYMBOLS_IS_TERMINAL (terminal), FALSE);

  TokenCategory token_category = TOKEN_CATEGORY_UNDEFINED;

  g_object_get (token,
                PROP_TOKEN_CATEGORY, &token_category,
                NULL);

  if (token_category == TOKEN_CATEGORY_END_OF_INPUT_MARKER)
    {
      g_autoptr (Lexeme) lexeme = NULL;
      g_autoptr (GString) lexeme_content = NULL;

      g_object_get (token,
                    PROP_TOKEN_LEXEME, &lexeme,
                    NULL);
      g_object_get (lexeme,
                    PROP_LEXEME_CONTENT, &lexeme_content,
                    NULL);

      return symbol_is_match (terminal, lexeme_content->str);
    }

  return FALSE;
}

static GNode *
parser_transform_analysis (GQueue *analysis_queue)
{
  GNode *concrete_syntax_tree = NULL;

  g_queue_reverse (analysis_queue);

  DerivationItem *start_derivation_item = g_queue_peek_tail (analysis_queue);
  g_autoptr (Production) start_left_hand_side = NULL;

  g_object_get (start_derivation_item,
                PROP_DERIVATION_ITEM_LEFT_HAND_SIDE, &start_left_hand_side,
                NULL);

  Symbol *symbol = non_terminal_new (PROP_SYMBOL_VALUE, start_left_hand_side);

  concrete_syntax_tree = g_node_new (symbol);

  parser_insert_children (concrete_syntax_tree, analysis_queue);

  return concrete_syntax_tree;
}

static void
parser_insert_children (GNode  *root,
                        GQueue *analysis_queue)
{
  g_assert (!g_queue_is_empty (analysis_queue));

  g_autoptr (DerivationItem) derivation_item = g_queue_pop_tail (analysis_queue);
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
       * tokens (which are guaranteed to be positioned right after them,
       * with the notable exception of epsilon) as these objects are more
       * useful in the succeeding stages of processing.
       */
      if (SYMBOLS_IS_TERMINAL (symbol))
        {
          if (!symbol_is_epsilon (symbol))
            {
              Token *token = g_queue_pop_tail (analysis_queue);

              child = g_node_new (token);
            }
          else
            {
              child = g_node_new (g_object_ref (symbol));
            }

          g_node_insert (root, i, child);
        }
      /* Non-terminals need to be transformed further, recursively. */
      else if (SYMBOLS_IS_NON_TERMINAL (symbol))
        {
          child = g_node_new (g_object_ref (symbol));

          g_node_insert (root, i, child);

          parser_insert_children (child, analysis_queue);
        }
    }
}

static void
parser_report_error (guint       token_position,
                     GPtrArray  *all_tokens,
                     gboolean    parsing_table_entry_found,
                     Symbol     *prediction_head,
                     GError    **error)
{
  Token *current_token = NULL;
  gboolean is_last_token = (token_position == all_tokens->len);
  TokenCategory token_category = TOKEN_CATEGORY_UNDEFINED;
  Token *invalid_token = NULL;
  SyntacticAnalysisParserError error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNDEFINED;
  const gchar *error_message = NULL;
  guint starting_position = token_position;

  if (is_last_token)
    starting_position--;

  current_token = g_ptr_array_index (all_tokens, starting_position);

  /* Trying to discern an informative error message in case a parsing table entry was not found
     or the input was exhausted without being accepted beforehand.
   */
  if (!parsing_table_entry_found || prediction_head == NULL || is_last_token)
    {
      g_autoptr (GArray) additional_categories = g_array_new (FALSE, FALSE, sizeof (TokenCategory));

      g_object_get (current_token,
                    PROP_TOKEN_CATEGORY, &token_category,
                    NULL);

      if (token_category == TOKEN_CATEGORY_END_OF_INPUT_MARKER)
        {
          TokenCategory categories[] = { TOKEN_CATEGORY_OPEN_PARENTHESIS,
                                         TOKEN_CATEGORY_CLOSE_PARENTHESIS,
                                         TOKEN_CATEGORY_OPEN_BRACKET,
                                         TOKEN_CATEGORY_ALTERNATION_OPERATOR,
                                         TOKEN_CATEGORY_METACHARACTER_ESCAPE,
                                         TOKEN_CATEGORY_END_ANCHOR };

          g_array_append_vals (additional_categories, categories, G_N_ELEMENTS (categories));
        }
      else if (token_category == TOKEN_CATEGORY_CLOSE_PARENTHESIS)
        {
          TokenCategory categories[] = { TOKEN_CATEGORY_ALTERNATION_OPERATOR };

          g_array_append_vals (additional_categories, categories, G_N_ELEMENTS (categories));
        }
      else if (token_category == TOKEN_CATEGORY_CLOSE_BRACKET)
        {
          Token *found_token = NULL;
          guint found_token_position = 0;

          if (parser_token_exists_in_all_tokens (all_tokens,
                                                 TOKEN_CATEGORY_OPEN_BRACKET,
                                                 starting_position,
                                                 &found_token,
                                                 &found_token_position))
            {
              guint token_distance = token_position - found_token_position;

              if (token_distance == 1)
                {
                  invalid_token = found_token;
                  error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_EMPTY_BRACKET_EXPRESSION;
                  error_message = "Empty bracket expressions are not allowed";
                }
            }

          if (invalid_token == NULL)
            {
              TokenCategory categories[] = { TOKEN_CATEGORY_RANGE_OPERATOR };

              g_array_append_vals (additional_categories, categories, G_N_ELEMENTS (categories));
            }
        }
      else if (token_category == TOKEN_CATEGORY_ORDINARY_CHARACTER)
        {
          TokenCategory categories[] = { TOKEN_CATEGORY_CLOSE_PARENTHESIS,
                                         TOKEN_CATEGORY_END_ANCHOR };

          g_array_append_vals (additional_categories, categories, G_N_ELEMENTS (categories));
        }

      if (invalid_token == NULL)
        {
          if (g_array_has_items (additional_categories))
            {
              Token *found_token = NULL;

              for (guint i = 0; i < additional_categories->len; ++i)
                {
                  TokenCategory current_category = g_array_index (additional_categories, TokenCategory, i);

                  if (parser_token_exists_in_all_tokens (all_tokens,
                                                         current_category,
                                                         starting_position,
                                                         &found_token,
                                                         NULL))
                    {
                      invalid_token = found_token;

                      switch (current_category)
                        {
                        case TOKEN_CATEGORY_ALTERNATION_OPERATOR:
                          goto dangling_alternation_operator;

                        case TOKEN_CATEGORY_RANGE_OPERATOR:
                          goto dangling_range_operator;

                        case TOKEN_CATEGORY_METACHARACTER_ESCAPE:
                          goto dangling_metacharacter_escape;

                        case TOKEN_CATEGORY_OPEN_PARENTHESIS:
                          goto unmatched_open_parenthesis;

                        case TOKEN_CATEGORY_CLOSE_PARENTHESIS:
                          goto unmatched_close_parenthesis;

                        case TOKEN_CATEGORY_OPEN_BRACKET:
                          goto unmatched_open_bracket;

                        case TOKEN_CATEGORY_END_ANCHOR:
                          goto unexpected_end_anchor_position;

                        default:
                          break;
                        }
                    }
                }
            }

          if (invalid_token == NULL)
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

            case TOKEN_CATEGORY_CLOSE_PARENTHESIS:
            unmatched_close_parenthesis:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNMATCHED_CLOSE_PARENTHESIS;
              error_message = "Unmatched close parenthesis";
              break;

            case TOKEN_CATEGORY_OPEN_BRACKET:
            unmatched_open_bracket:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNMATCHED_OPEN_BRACKET;
              error_message = "Unmatched open bracket";
              break;

            case TOKEN_CATEGORY_END_ANCHOR:
            unexpected_end_anchor_position:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNEXPECTED_END_ANCHOR;
              error_message = "Unexpected end anchor";
              break;

            case TOKEN_CATEGORY_START_ANCHOR:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNEXPECTED_START_ANCHOR;
              error_message = "Unexpected start anchor";
              break;

            case TOKEN_CATEGORY_EMPTY_EXPRESSION_MARKER:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_UNEXPECTED_EMPTY_EXPRESSION;
              error_message = "Unexpected empty expression";
              break;

            case TOKEN_CATEGORY_STAR_QUANTIFICATION_OPERATOR:
            case TOKEN_CATEGORY_PLUS_QUANTIFICATION_OPERATOR:
            case TOKEN_CATEGORY_QUESTION_MARK_QUANTIFICATION_OPERATOR:
              error_code = SYNTACTIC_ANALYSIS_PARSER_ERROR_DANGLING_QUANTIFICATION_OPERATOR;
              error_message = "Dangling quantification operator";
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
                   "%s (position - %d)",
                   error_message, position);
    }
}

static gboolean
parser_token_exists_in_all_tokens (GPtrArray      *all_tokens,
                                   TokenCategory   category,
                                   guint           starting_position,
                                   Token         **found_token,
                                   guint          *found_token_position)
{
  g_return_val_if_fail (found_token != NULL, FALSE);

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

          if (found_token_position != NULL)
            *found_token_position = i;

          return TRUE;
        }
    }

  return FALSE;
}

static void
parser_dispose (GObject *object)
{
  ParserPrivate *priv = parser_get_instance_private (SYNTACTIC_ANALYSIS_PARSER (object));

  if (priv->analysis_queue != NULL)
    g_clear_pointer (&priv->analysis_queue, g_queue_unref_g_objects);

  if (priv->prediction_queue != NULL)
    g_clear_pointer (&priv->prediction_queue, g_queue_free);

  if (priv->grammar != NULL)
    g_clear_object (&priv->grammar);

  G_OBJECT_CLASS (parser_parent_class)->dispose (object);
}
