#include "internal/semantic_analysis/analyzer.h"
#include "internal/semantic_analysis/ast_node_factory.h"
#include "internal/syntactic_analysis/grammar.h"
#include "internal/syntactic_analysis/production.h"
#include "internal/syntactic_analysis/symbols/non_terminal.h"
#include "internal/syntactic_analysis/token.h"

struct _Analyzer
{
    GObject parent_instance;
};

typedef struct
{
    GHashTable *operator_types;
} AnalyzerPrivate;

typedef enum
{
    FETCH_CST_CHILDREN_NON_TERMINAL = 1 << 0,
    FETCH_CST_CHILDREN_TOKEN = 1 << 1,
    FETCH_CST_CHILDREN_ALL = 1 << 2,
    FETCH_CST_CHILDREN_FIRST = 1 << 3
} FetchCstChildrenFlags;

static AstNode      *analyzer_transform_concrete_syntax_tree (GNode                  *cst_root,
                                                              GHashTable             *operator_types);

static void          analyzer_define_operator_types          (GHashTable             *operator_types);

static gboolean      analyzer_is_constant                    (GNode                  *cst_root,
                                                              GNode                 **first_cst_child);

static gboolean      analyzer_is_unary_operator              (GNode                  *cst_root,
                                                              GNode                 **first_cst_child);

static gboolean      analyzer_is_binary_operator             (GNode                  *cst_root,
                                                              GNode                 **first_cst_child,
                                                              GNode                 **second_cst_child);

static gboolean      analyzer_is_match                       (GNode                  *cst_root,
                                                              ...);

static AstNode      *analyzer_continue                       (GNode                  *cst_root,
                                                              GHashTable             *operator_types);

static GPtrArray    *analyzer_fetch_cst_children             (GNode                  *cst_root,
                                                              FetchCstChildrenFlags   fetchCstChildrenFlags);

static const gchar  *analyzer_fetch_node_caption             (GNode                  *cst_root);

static OperatorType  analyzer_discern_operator_type          (GNode                  *cst_root,
                                                              GHashTable             *operator_types);

static void          analyzer_dispose                        (GObject                *object);

G_DEFINE_TYPE_WITH_PRIVATE (Analyzer, analyzer, G_TYPE_OBJECT)

static void
analyzer_class_init (AnalyzerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = analyzer_dispose;
}

static void
analyzer_init (Analyzer *self)
{
  AnalyzerPrivate *priv = analyzer_get_instance_private (self);

  priv->operator_types = g_hash_table_new (g_str_hash,
                                           g_str_equal);

  analyzer_define_operator_types (priv->operator_types);
}

AstNode *
analyzer_build_abstract_syntax_tree (Analyzer  *self,
                                     GNode     *concrete_syntax_tree,
                                     GError   **error)
{
  g_return_val_if_fail (SEMANTIC_ANALYSIS_IS_ANALYZER (self), NULL);
  g_return_val_if_fail (concrete_syntax_tree != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  AnalyzerPrivate *priv = analyzer_get_instance_private (self);
  GHashTable *operator_types = priv->operator_types;
  AstNode *abstract_syntax_tree = analyzer_transform_concrete_syntax_tree (concrete_syntax_tree,
                                                                           operator_types);

  if (!ast_node_is_valid (abstract_syntax_tree, error))
    {
      g_assert (error != NULL || *error != NULL);
      g_clear_object (&abstract_syntax_tree);
    }

  return abstract_syntax_tree;
}

static AstNode *
analyzer_transform_concrete_syntax_tree (GNode      *cst_root,
                                         GHashTable *operator_types)
{
  AstNode *ast_node = NULL;
  GNode *first_cst_child = NULL;
  GNode *second_cst_child = NULL;

  if (analyzer_is_constant (cst_root,
                            &first_cst_child))
    {
      ast_node = create_constant (first_cst_child);
    }
  else if (analyzer_is_unary_operator (cst_root,
                                       &first_cst_child))
    {
      g_autoptr (AstNode) operand = analyzer_transform_concrete_syntax_tree (first_cst_child,
                                                                             operator_types);

      ast_node = create_unary_operator (analyzer_discern_operator_type (cst_root,
                                                                        operator_types),
                                        operand);
    }
  else if (analyzer_is_binary_operator (cst_root,
                                        &first_cst_child,
                                        &second_cst_child))
    {
      g_autoptr (AstNode) left_operand = analyzer_transform_concrete_syntax_tree (first_cst_child,
                                                                                  operator_types);
      g_autoptr (AstNode) right_operand = analyzer_transform_concrete_syntax_tree (second_cst_child,
                                                                                   operator_types);

      ast_node = create_binary_operator (analyzer_discern_operator_type (cst_root,
                                                                         operator_types),
                                         left_operand,
                                         right_operand);
    }
  else
    {
      ast_node = analyzer_continue (cst_root, operator_types);
    }

  return ast_node;
}

static void
analyzer_define_operator_types (GHashTable *operator_types)
{
  g_hash_table_insert (operator_types,
                       EXPRESSION,
                       GINT_TO_POINTER (OPERATOR_TYPE_ALTERNATION));
  g_hash_table_insert (operator_types,
                       ALTERNATION,
                       GINT_TO_POINTER (OPERATOR_TYPE_ALTERNATION));
  g_hash_table_insert (operator_types,
                       SIMPLE_EXPRESSION,
                       GINT_TO_POINTER (OPERATOR_TYPE_CONCATENATION));
  g_hash_table_insert (operator_types,
                       CONCATENATION,
                       GINT_TO_POINTER (OPERATOR_TYPE_CONCATENATION));
  g_hash_table_insert (operator_types,
                       STAR_QUANTIFICATION,
                       GINT_TO_POINTER (OPERATOR_TYPE_STAR_QUANTIFICATION));
  g_hash_table_insert (operator_types,
                       PLUS_QUANTIFICATION,
                       GINT_TO_POINTER (OPERATOR_TYPE_PLUS_QUANTIFICATION));
  g_hash_table_insert (operator_types,
                       QUESTION_MARK_QUANTIFICATION,
                       GINT_TO_POINTER (OPERATOR_TYPE_QUESTION_MARK_QUANTIFICATION));
  g_hash_table_insert (operator_types,
                       BRACKET_EXPRESSION,
                       GINT_TO_POINTER (OPERATOR_TYPE_BRACKET_EXPRESSION));
  /* Bracket expression items behave in exactly the same way as alternation does but without
   * the usage of an explicit operator ("|").
   */
  g_hash_table_insert (operator_types,
                       BRACKET_EXPRESSION_ITEMS,
                       GINT_TO_POINTER (OPERATOR_TYPE_ALTERNATION));
  g_hash_table_insert (operator_types,
                       UPPER_CASE_LETTER_RANGE,
                       GINT_TO_POINTER (OPERATOR_TYPE_RANGE));
}

static gboolean
analyzer_is_constant (GNode  *cst_root,
                      GNode **first_cst_child)
{
  gboolean result = FALSE;

  if (analyzer_is_match (cst_root,
                         UPPER_CASE_LETTER,
                         LOWER_CASE_LETTER,
                         DIGIT,
                         SPECIAL_CHARACTER,
                         REGULAR_METACHARACTER,
                         BRACKET_EXPRESSION_METACHARACTER,
                         METACHARACTER_ESCAPE,
                         NULL))
    {
      g_autoptr (GPtrArray) cst_children =
        analyzer_fetch_cst_children (cst_root,
                                     FETCH_CST_CHILDREN_TOKEN | FETCH_CST_CHILDREN_FIRST);

      *first_cst_child = g_ptr_array_index (cst_children, 0);

      result = TRUE;
    }

  return result;
}

static gboolean
analyzer_is_unary_operator (GNode  *cst_root,
                            GNode **first_cst_child)
{
  gboolean result = FALSE;

  if (analyzer_is_match (cst_root,
                         STAR_QUANTIFICATION,
                         PLUS_QUANTIFICATION,
                         QUESTION_MARK_QUANTIFICATION,
                         BRACKET_EXPRESSION,
                         NULL))
    {
      g_autoptr (GPtrArray) cst_children =
        analyzer_fetch_cst_children (cst_root,
                                     FETCH_CST_CHILDREN_NON_TERMINAL | FETCH_CST_CHILDREN_FIRST);
      const guint unary_operator_operands_count = 1;

      if (cst_children->len == unary_operator_operands_count)
        {
          *first_cst_child = g_ptr_array_index (cst_children, 0);

          result = TRUE;
        }
    }

  return result;
}

static gboolean
analyzer_is_binary_operator (GNode  *cst_root,
                             GNode **first_cst_child,
                             GNode **second_cst_child)
{
  gboolean result = FALSE;

  if (analyzer_is_match (cst_root,
                         EXPRESSION,
                         ALTERNATION,
                         SIMPLE_EXPRESSION,
                         CONCATENATION,
                         BRACKET_EXPRESSION_ITEMS,
                         UPPER_CASE_LETTER_RANGE,
                         NULL))
    {
      g_autoptr (GPtrArray) cst_children =
        analyzer_fetch_cst_children (cst_root,
                                     FETCH_CST_CHILDREN_NON_TERMINAL | FETCH_CST_CHILDREN_ALL);
      const guint binary_operator_operands_count = 2;

      if (cst_children->len == binary_operator_operands_count)
        {
          *first_cst_child = g_ptr_array_index (cst_children, 0);
          *second_cst_child = g_ptr_array_index (cst_children, 1);

          result = TRUE;
        }
    }

  return result;
}

static gboolean
analyzer_is_match (GNode *cst_root,
                   ...)
{
  gboolean result = FALSE;

  if (SYMBOLS_IS_NON_TERMINAL (cst_root->data))
    {
      va_list ap;
      g_autofree const gchar *cst_root_caption = analyzer_fetch_node_caption (cst_root);
      const gchar *current = NULL;

      va_start (ap, cst_root);

      while (TRUE)
        {
          current = va_arg (ap, gchar*);

          if (current != NULL)
            {
              if (g_strcmp0 (cst_root_caption, current) == 0)
                {
                  result = TRUE;
                  break;
                }
            }
          else
            {
              break;
            }
        }

      va_end (ap);
    }

  return result;
}

static AstNode *
analyzer_continue (GNode      *cst_root,
                   GHashTable *operator_types)
{
  g_autoptr (GPtrArray) cst_children =
    analyzer_fetch_cst_children (cst_root,
                                 FETCH_CST_CHILDREN_NON_TERMINAL | FETCH_CST_CHILDREN_FIRST);
  GNode *first_cst_child = g_ptr_array_index (cst_children, 0);

  return analyzer_transform_concrete_syntax_tree (first_cst_child,
                                                  operator_types);
}

static GPtrArray *
analyzer_fetch_cst_children (GNode                 *cst_root,
                             FetchCstChildrenFlags  fetchCstChildrenFlags)
{
  GPtrArray *children = g_ptr_array_new ();
  guint children_count = g_node_n_children (cst_root);

  for (guint i = 0; i < children_count; ++i)
    {
      GNode *child = g_node_nth_child (cst_root, i);

      if (((fetchCstChildrenFlags & FETCH_CST_CHILDREN_NON_TERMINAL) &&
           SYMBOLS_IS_NON_TERMINAL (child->data)) ||
          ((fetchCstChildrenFlags & FETCH_CST_CHILDREN_TOKEN) &&
           SYNTACTIC_ANALYSIS_IS_TOKEN (child->data)))
        {
          g_ptr_array_add (children, child);

          if (fetchCstChildrenFlags & FETCH_CST_CHILDREN_ALL)
            continue;
          else if (fetchCstChildrenFlags & FETCH_CST_CHILDREN_FIRST)
            break;
        }
    }

  return children;
}

static const gchar *
analyzer_fetch_node_caption (GNode *cst_root)
{
  gchar *caption = NULL;

  if (SYMBOLS_IS_NON_TERMINAL (cst_root->data))
    {
      Symbol *symbol = cst_root->data;
      g_autoptr (Production) production = NULL;
      GValue value = G_VALUE_INIT;

      symbol_extract_value (symbol, &value);

      production = g_value_get_object (&value);

      g_object_get (production,
                    PROP_PRODUCTION_CAPTION, &caption,
                    NULL);
    }

  return caption;
}

static OperatorType
analyzer_discern_operator_type (GNode      *cst_root,
                                GHashTable *operator_types)
{
  g_autofree const gchar *cst_root_caption = analyzer_fetch_node_caption (cst_root);
  OperatorType operator_type = OPERATOR_TYPE_UNDEFINED;

  operator_type = (OperatorType) GPOINTER_TO_INT (g_hash_table_lookup (operator_types,
                                                                       cst_root_caption));

  return operator_type;
}

static void
analyzer_dispose (GObject *object)
{
  AnalyzerPrivate *priv = analyzer_get_instance_private (SEMANTIC_ANALYSIS_ANALYZER (object));

  if (priv->operator_types != NULL)
    g_clear_pointer (&priv->operator_types, g_hash_table_unref);

  G_OBJECT_CLASS (analyzer_parent_class)->dispose (object);
}
