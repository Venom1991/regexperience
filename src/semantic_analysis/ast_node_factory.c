#include "internal/semantic_analysis/ast_node_factory.h"
#include "internal/semantic_analysis/ast_nodes/alternation.h"
#include "internal/semantic_analysis/ast_nodes/anchor.h"
#include "internal/semantic_analysis/ast_nodes/concatenation.h"
#include "internal/semantic_analysis/ast_nodes/empty.h"
#include "internal/semantic_analysis/ast_nodes/quantification.h"
#include "internal/semantic_analysis/ast_nodes/range.h"
#include "internal/syntactic_analysis/symbols/symbol.h"
#include "internal/lexical_analysis/lexeme.h"
#include "internal/lexical_analysis/token.h"
#include "internal/state_machines/transitions/transition.h"

AstNode *
create_constant (GNode *cst_context)
{
  g_return_val_if_fail (cst_context != NULL, NULL);

  Token *token = LEXICAL_ANALYSIS_TOKEN (cst_context->data);
  TokenCategory token_category = TOKEN_CATEGORY_UNDEFINED;

  g_object_get (token,
                PROP_TOKEN_CATEGORY, &token_category,
                NULL);

  if (token_category == TOKEN_CATEGORY_EMPTY_EXPRESSION_MARKER)
    {
      return empty_new ();
    }
  else
    {
      g_autoptr (Lexeme) lexeme = NULL;
      g_autoptr (GString) lexeme_content = NULL;
      const guint acceptable_lexeme_content_length = 1;
      guint lexeme_start_position = 0;
      guint lexeme_end_position = 0;
      gchar expected_character = 0;

      g_object_get (token,
                    PROP_TOKEN_LEXEME, &lexeme,
                    NULL);
      g_object_get (lexeme,
                    PROP_LEXEME_CONTENT, &lexeme_content,
                    PROP_LEXEME_START_POSITION, &lexeme_start_position,
                    PROP_LEXEME_END_POSITION, &lexeme_end_position,
                    NULL);

      g_return_val_if_fail (lexeme_content->len == acceptable_lexeme_content_length, NULL);

      switch (token_category)
        {
        case TOKEN_CATEGORY_ANY_CHARACTER:
          expected_character = ANY;
          break;

        case TOKEN_CATEGORY_ORDINARY_CHARACTER:
          expected_character = lexeme_content->str[0];
          break;

        default:
          g_return_val_if_reached (NULL);
        }

      return constant_new (PROP_CONSTANT_VALUE, expected_character,
                           PROP_CONSTANT_POSITION, (lexeme_start_position + lexeme_end_position) / 2);
    }
}

AnchorType
discern_anchor_type (GNode *anchor_cst_node)
{
  AnchorType anchor_type = ANCHOR_TYPE_UNDEFINED;
  GNode *anchor_cst_node_child = g_node_nth_child (anchor_cst_node, 0);
  gpointer child_data = anchor_cst_node_child->data;

  if (LEXICAL_ANALYSIS_IS_TOKEN (child_data))
    {
      Token *token = LEXICAL_ANALYSIS_TOKEN (child_data);
      TokenCategory token_category = TOKEN_CATEGORY_UNDEFINED;

      g_object_get (token,
                    PROP_TOKEN_CATEGORY, &token_category,
                    NULL);

      if (token_category == TOKEN_CATEGORY_BEGIN_ANCHOR ||
          token_category == TOKEN_CATEGORY_END_ANCHOR)
        anchor_type = ANCHOR_TYPE_ANCHORED;
    }
  else if (SYMBOLS_IS_SYMBOL (child_data))
    {
      Symbol *symbol = SYMBOLS_SYMBOL (child_data);

      if (symbol_is_epsilon (symbol))
        anchor_type = ANCHOR_TYPE_UNANCHORED;
    }

  return anchor_type;
}

AstNode *
create_anchor (GNode   *cst_context,
               AstNode *anchored_node)
{
  g_return_val_if_fail (cst_context != NULL, NULL);
  g_return_val_if_fail (anchored_node != NULL, NULL);

  AnchorType start_anchor_type = discern_anchor_type (g_node_first_child (cst_context));
  AnchorType end_anchor_type = discern_anchor_type (g_node_last_child (cst_context));

  return anchor_new (PROP_UNARY_OPERATOR_OPERAND, anchored_node,
                     PROP_ANCHOR_START_TYPE, start_anchor_type,
                     PROP_ANCHOR_END_TYPE, end_anchor_type);
}

void
initialize_quantification_bounds (OperatorType             operator_type,
                                  QuantificationBoundType *lower_bound,
                                  QuantificationBoundType *upper_bound)
{
  g_return_if_fail (lower_bound != NULL);
  g_return_if_fail (upper_bound != NULL);

  switch (operator_type)
    {
    case OPERATOR_TYPE_STAR_QUANTIFICATION:
      *lower_bound = QUANTIFICATION_BOUND_TYPE_ZERO;
      *upper_bound = QUANTIFICATION_BOUND_TYPE_INFINITY;
      break;

    case OPERATOR_TYPE_PLUS_QUANTIFICATION:
      *lower_bound = QUANTIFICATION_BOUND_TYPE_ONE;
      *upper_bound = QUANTIFICATION_BOUND_TYPE_INFINITY;
      break;

    case OPERATOR_TYPE_QUESTION_MARK_QUANTIFICATION:
      *lower_bound = QUANTIFICATION_BOUND_TYPE_ZERO;
      *upper_bound = QUANTIFICATION_BOUND_TYPE_ONE;
      break;

    default:
      *lower_bound = QUANTIFICATION_BOUND_TYPE_UNDEFINED;
      *upper_bound = QUANTIFICATION_BOUND_TYPE_UNDEFINED;
      break;
    }
}

AstNode *
create_unary_operator (OperatorType  operator_type,
                       AstNode      *operand)
{
  g_return_val_if_fail (operand != OPERATOR_TYPE_UNDEFINED, NULL);
  g_return_val_if_fail (operand != NULL, NULL);

  switch (operator_type)
    {
    case OPERATOR_TYPE_STAR_QUANTIFICATION:
    case OPERATOR_TYPE_PLUS_QUANTIFICATION:
    case OPERATOR_TYPE_QUESTION_MARK_QUANTIFICATION:
      {
        QuantificationBoundType lower_bound = QUANTIFICATION_BOUND_TYPE_UNDEFINED;
        QuantificationBoundType upper_bound = QUANTIFICATION_BOUND_TYPE_UNDEFINED;

        initialize_quantification_bounds (operator_type,
                                          &lower_bound,
                                          &upper_bound);

        return quantification_new (PROP_UNARY_OPERATOR_OPERAND, operand,
                                   PROP_QUANTIFICATION_LOWER_BOUND, lower_bound,
                                   PROP_QUANTIFICATION_UPPER_BOUND, upper_bound);
      }

    default:
      g_return_val_if_reached (NULL);
    }
}

AstNode *
create_binary_operator (OperatorType  operator_type,
                        AstNode      *left_operand,
                        AstNode      *right_operand)
{
  g_return_val_if_fail (operator_type != OPERATOR_TYPE_UNDEFINED, NULL);
  g_return_val_if_fail (left_operand != NULL, NULL);
  g_return_val_if_fail (right_operand != NULL, NULL);

  switch (operator_type)
    {
    case OPERATOR_TYPE_ALTERNATION:
      return alternation_new (PROP_BINARY_OPERATOR_LEFT_OPERAND, left_operand,
                              PROP_BINARY_OPERATOR_RIGHT_OPERAND, right_operand);

    case OPERATOR_TYPE_CONCATENATION:
      return concatenation_new (PROP_BINARY_OPERATOR_LEFT_OPERAND, left_operand,
                                PROP_BINARY_OPERATOR_RIGHT_OPERAND, right_operand);

    case OPERATOR_TYPE_RANGE:
      return range_new (PROP_BINARY_OPERATOR_LEFT_OPERAND, left_operand,
                        PROP_BINARY_OPERATOR_RIGHT_OPERAND, right_operand);

    default:
      g_return_val_if_reached (NULL);
    }
}
