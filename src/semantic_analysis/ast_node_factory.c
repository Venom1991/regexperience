#include "internal/semantic_analysis/ast_node_factory.h"
#include "internal/semantic_analysis/ast_nodes/constant.h"
#include "internal/semantic_analysis/ast_nodes/alternation.h"
#include "internal/semantic_analysis/ast_nodes/concatenation.h"
#include "internal/semantic_analysis/ast_nodes/quantification.h"
#include "internal/semantic_analysis/ast_nodes/range.h"
#include "internal/lexical_analysis/token.h"
#include "internal/lexical_analysis/lexeme.h"
#include "internal/state_machines/transitions/transition.h"

AstNode *
create_constant (GNode *cst_context)
{
  g_return_val_if_fail (cst_context != NULL, NULL);

  Token *token = LEXICAL_ANALYSIS_TOKEN (cst_context->data);
  TokenCategory token_category = TOKEN_CATEGORY_UNDEFINED;
  g_autoptr (Lexeme) lexeme = NULL;
  g_autoptr (GString) lexeme_content = NULL;
  const guint acceptable_lexeme_content_length = 1;
  guint lexeme_start_position = 0;
  guint lexeme_end_position = 0;
  gchar expected_character = 0;

  g_object_get (token,
                PROP_TOKEN_CATEGORY, &token_category,
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

AstNode *
create_unary_operator (OperatorType  operator_type,
                       AstNode      *operand)
{
  g_return_val_if_fail (operand != NULL, NULL);

  switch (operator_type)
    {
    case OPERATOR_TYPE_STAR_QUANTIFICATION:
      return quantification_new (PROP_UNARY_OPERATOR_OPERAND, operand,
                                 PROP_QUANTIFICATION_LOWER_BOUND, QUANTIFICATION_BOUND_TYPE_ZERO,
                                 PROP_QUANTIFICATION_UPPER_BOUND, QUANTIFICATION_BOUND_TYPE_INFINITY);

    case OPERATOR_TYPE_PLUS_QUANTIFICATION:
      return quantification_new (PROP_UNARY_OPERATOR_OPERAND, operand,
                                 PROP_QUANTIFICATION_LOWER_BOUND, QUANTIFICATION_BOUND_TYPE_ONE,
                                 PROP_QUANTIFICATION_UPPER_BOUND, QUANTIFICATION_BOUND_TYPE_INFINITY);

    case OPERATOR_TYPE_QUESTION_MARK_QUANTIFICATION:
      return quantification_new (PROP_UNARY_OPERATOR_OPERAND, operand,
                                 PROP_QUANTIFICATION_LOWER_BOUND, QUANTIFICATION_BOUND_TYPE_ZERO,
                                 PROP_QUANTIFICATION_UPPER_BOUND, QUANTIFICATION_BOUND_TYPE_ONE);

    default:
      return NULL;
    }
}

AstNode *
create_binary_operator (OperatorType  operator_type,
                        AstNode      *left_operand,
                        AstNode      *right_operand)
{
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
      return NULL;
    }
}
