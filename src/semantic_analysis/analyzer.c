#include "internal/semantic_analysis/analyzer.h"
#include "internal/semantic_analysis/ast_node_factory.h"
#include "internal/syntactic_analysis/grammar.h"
#include "internal/syntactic_analysis/production.h"
#include "internal/syntactic_analysis/symbols/terminal.h"
#include "internal/syntactic_analysis/symbols/non_terminal.h"
#include "internal/lexical_analysis/token.h"
#include "internal/common/helpers.h"

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
  FETCH_CST_CHILDREN_TOKEN = 1 << 0,
  FETCH_CST_CHILDREN_TERMINAL = 1 << 1,
  FETCH_CST_CHILDREN_NON_TERMINAL = 1 << 2,
  FETCH_CST_CHILDREN_ALL = 1 << 3,
  FETCH_CST_CHILDREN_FIRST = 1 << 4
} FetchCstChildrenFlags;

static AstNode      *analyzer_transform_concrete_syntax_tree (GNode                  *cst_root,
                                                              GHashTable             *operator_types);

static GHashTable   *analyzer_define_operator_types          (void);

static gboolean      analyzer_is_constant                    (GNode                  *cst_root,
                                                              GNode                 **first_cst_child);

static gboolean      analyzer_is_anchor                      (GNode                  *cst_root,
                                                              GNode                 **second_cst_child);

static gboolean      analyzer_is_unary_operator              (GNode                  *cst_root,
                                                              GNode                 **first_cst_child,
                                                              GNode                 **operator_type_discerning_node);

static gboolean      analyzer_is_binary_operator             (GNode                  *cst_root,
                                                              GNode                 **first_cst_child,
                                                              GNode                 **second_cst_child,
                                                              GNode                 **operator_type_discerning_node);

static gboolean      analyzer_is_empty                       (GNode                  *cst_root);

static gboolean      analyzer_is_match                       (GNode                  *cst_root,
                                                              ...);

static AstNode      *analyzer_continue                       (GNode                  *cst_root,
                                                              GHashTable             *operator_types);

static GPtrArray    *analyzer_fetch_cst_children             (GNode                  *cst_root,
                                                              FetchCstChildrenFlags   fetch_cst_children_flags);

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

  GHashTable *operator_types = analyzer_define_operator_types ();

  priv->operator_types = operator_types;
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
  GNode *operator_type_discerning_node = NULL;

  if (analyzer_is_constant (cst_root,
                            &first_cst_child))
    {
      ast_node = create_constant (first_cst_child);
    }
  else if (analyzer_is_anchor (cst_root, &second_cst_child))
    {
      g_autoptr (AstNode) anchored_node = analyzer_transform_concrete_syntax_tree (second_cst_child,
                                                                                   operator_types);

      ast_node = create_anchor (cst_root, anchored_node);
    }
  else if (analyzer_is_unary_operator (cst_root,
                                       &first_cst_child,
                                       &operator_type_discerning_node))
    {
      g_autoptr (AstNode) operand = analyzer_transform_concrete_syntax_tree (first_cst_child,
                                                                             operator_types);

      ast_node = create_unary_operator (analyzer_discern_operator_type (operator_type_discerning_node,
                                                                        operator_types),
                                        operand);
    }
  else if (analyzer_is_binary_operator (cst_root,
                                        &first_cst_child,
                                        &second_cst_child,
                                        &operator_type_discerning_node))
    {
      g_autoptr (AstNode) left_operand = analyzer_transform_concrete_syntax_tree (first_cst_child,
                                                                                  operator_types);
      g_autoptr (AstNode) right_operand = analyzer_transform_concrete_syntax_tree (second_cst_child,
                                                                                   operator_types);

      ast_node = create_binary_operator (analyzer_discern_operator_type (operator_type_discerning_node,
                                                                         operator_types),
                                         left_operand,
                                         right_operand);
    }
  else
    {
      /* Continuing the analysis with the current root node's first (and only) child. */
      ast_node = analyzer_continue (cst_root, operator_types);
    }

  return ast_node;
}

static GHashTable *
analyzer_define_operator_types (void)
{
  GHashTable *operator_types = g_hash_table_new (g_str_hash,
                                                 g_str_equal);

  g_hash_table_insert (operator_types,
                       EXPRESSION_PRIME,
                       GINT_TO_POINTER (OPERATOR_TYPE_ALTERNATION));
  g_hash_table_insert (operator_types,
                       ALTERNATION_PRIME,
                       GINT_TO_POINTER (OPERATOR_TYPE_ALTERNATION));
  g_hash_table_insert (operator_types,
                       SIMPLE_EXPRESSION_PRIME,
                       GINT_TO_POINTER (OPERATOR_TYPE_CONCATENATION));
  g_hash_table_insert (operator_types,
                       CONCATENATION_PRIME,
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
  /* Bracket expression items behave in exactly the same way as alternation does but without
   * the usage of an explicit operator ("|").
   */
  g_hash_table_insert (operator_types,
                       BRACKET_EXPRESSION_ITEMS_PRIME,
                       GINT_TO_POINTER (OPERATOR_TYPE_ALTERNATION));
  g_hash_table_insert (operator_types,
                       UPPER_CASE_LETTER_RANGE,
                       GINT_TO_POINTER (OPERATOR_TYPE_RANGE));
  g_hash_table_insert (operator_types,
                       LOWER_CASE_LETTER_RANGE,
                       GINT_TO_POINTER (OPERATOR_TYPE_RANGE));
  g_hash_table_insert (operator_types,
                       DIGIT_RANGE,
                       GINT_TO_POINTER (OPERATOR_TYPE_RANGE));

  return operator_types;
}

static gboolean
analyzer_is_constant (GNode  *cst_root,
                      GNode **first_cst_child)
{
  if (analyzer_is_match (cst_root,
                         UPPER_CASE_LETTER,
                         LOWER_CASE_LETTER,
                         DIGIT,
                         SPECIAL_CHARACTER,
                         REGULAR_METACHARACTER,
                         BRACKET_EXPRESSION_METACHARACTER,
                         ANY_CHARACTER,
                         METACHARACTER_ESCAPE,
                         SPACE,
                         HORIZONTAL_TAB,
                         EMPTY_EXPRESSION,
                         NULL))
    {
      g_autoptr (GPtrArray) cst_children =
        analyzer_fetch_cst_children (cst_root,
                                     FETCH_CST_CHILDREN_TOKEN | FETCH_CST_CHILDREN_FIRST);

      if (g_collection_has_items (cst_children))
        {
          const guint constant_node_children_count = 1;

          if (cst_children->len == constant_node_children_count)
            {
              *first_cst_child = g_ptr_array_index (cst_children, 0);

              return TRUE;
            }
        }
    }

  return FALSE;
}

static gboolean
analyzer_is_anchor (GNode  *cst_root,
                    GNode **second_cst_child)
{
  if (analyzer_is_match (cst_root,
                         ANCHORED_EXPRESSION,
                         NULL))
    {
      g_autoptr (GPtrArray) cst_children =
          analyzer_fetch_cst_children (cst_root,
                                       FETCH_CST_CHILDREN_NON_TERMINAL | FETCH_CST_CHILDREN_ALL);

      if (g_collection_has_items (cst_children))
        {
          const guint anchor_node_children_count = 3;

          if (cst_children->len == anchor_node_children_count)
            {
              *second_cst_child = g_ptr_array_index (cst_children, 1);

              return TRUE;
            }
        }
    }

  return FALSE;
}

static gboolean
analyzer_is_unary_operator (GNode  *cst_root,
                            GNode **first_cst_child,
                            GNode **operator_type_discerning_node)
{
  if (analyzer_is_match (cst_root,
                         BASIC_EXPRESSION,
                         NULL))
    {
      g_autoptr (GPtrArray) cst_children =
          analyzer_fetch_cst_children (cst_root,
                                       FETCH_CST_CHILDREN_NON_TERMINAL | FETCH_CST_CHILDREN_ALL);

      if (g_collection_has_items (cst_children))
        {
          for (guint i = 0; i < cst_children->len; ++i)
            {
              GNode *cst_child = g_ptr_array_index (cst_children, i);

              if (analyzer_is_match (cst_child,
                                     BASIC_EXPRESSION_PRIME,
                                     NULL) &&
                  !analyzer_is_empty (cst_child))
                {
                  g_autoptr (GPtrArray) cst_grandchildren =
                    analyzer_fetch_cst_children (cst_child,
                                                 FETCH_CST_CHILDREN_NON_TERMINAL | FETCH_CST_CHILDREN_FIRST);

                  if (g_collection_has_items (cst_grandchildren))
                    {
                      GNode *cst_grandchild = g_ptr_array_index (cst_grandchildren, 0);

                      if (analyzer_is_match (cst_grandchild,
                                             STAR_QUANTIFICATION,
                                             PLUS_QUANTIFICATION,
                                             QUESTION_MARK_QUANTIFICATION,
                                             NULL))
                        {
                          *first_cst_child = g_ptr_array_index (cst_children, 0);
                          *operator_type_discerning_node = cst_grandchild;

                          return TRUE;
                        }
                    }
                }
            }
        }
    }

  return FALSE;
}

static gboolean
analyzer_is_binary_operator (GNode  *cst_root,
                             GNode **first_cst_child,
                             GNode **second_cst_child,
                             GNode **operator_type_discerning_node)
{
  if (analyzer_is_match (cst_root,
                         EXPRESSION,
                         ALTERNATION,
                         SIMPLE_EXPRESSION,
                         CONCATENATION,
                         BRACKET_EXPRESSION_ITEMS,
                         BRACKET_EXPRESSION_ITEM,
                         NULL))
    {
      g_autoptr (GPtrArray) cst_children =
        analyzer_fetch_cst_children (cst_root,
                                     FETCH_CST_CHILDREN_NON_TERMINAL | FETCH_CST_CHILDREN_ALL);

      if (g_collection_has_items (cst_children))
        {
          for (guint i = 0; i < cst_children->len; ++i)
            {
              GNode *cst_child = g_ptr_array_index (cst_children, i);

              if (analyzer_is_match (cst_child,
                                     EXPRESSION_PRIME,
                                     ALTERNATION_PRIME,
                                     SIMPLE_EXPRESSION_PRIME,
                                     CONCATENATION_PRIME,
                                     BRACKET_EXPRESSION_ITEMS_PRIME,
                                     UPPER_CASE_LETTER_RANGE,
                                     LOWER_CASE_LETTER_RANGE,
                                     DIGIT_RANGE,
                                     NULL) &&
                  !analyzer_is_empty (cst_child))
                {
                  g_autoptr (GPtrArray) cst_grandchildren =
                    analyzer_fetch_cst_children (cst_child,
                                                 FETCH_CST_CHILDREN_NON_TERMINAL | FETCH_CST_CHILDREN_ALL);

                  if (g_collection_has_items (cst_grandchildren))
                    {
                      *first_cst_child = g_ptr_array_index (cst_children, 0);
                      *second_cst_child = g_ptr_array_index (cst_grandchildren, cst_grandchildren->len - 1);
                      *operator_type_discerning_node = cst_child;

                      return TRUE;
                    }
                }
            }
        }
    }

  return FALSE;
}

static gboolean
analyzer_is_empty (GNode *cst_root)
{
  g_autoptr (GPtrArray) cst_children =
    analyzer_fetch_cst_children (cst_root,
                                 FETCH_CST_CHILDREN_TERMINAL | FETCH_CST_CHILDREN_FIRST);

  if (g_collection_has_items (cst_children))
    {
      const guint empty_node_children_count = 1;

      if (cst_children->len == empty_node_children_count)
        {
          GNode *cst_child = g_ptr_array_index (cst_children, 0);
          Symbol *terminal = SYMBOLS_SYMBOL (cst_child->data);

          return symbol_is_epsilon (terminal);
        }
    }

  return FALSE;
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

  g_assert (g_collection_has_items (cst_children));

  GNode *first_cst_child = g_ptr_array_index (cst_children, 0);

  return analyzer_transform_concrete_syntax_tree (first_cst_child,
                                                  operator_types);
}

static GPtrArray *
analyzer_fetch_cst_children (GNode                 *cst_root,
                             FetchCstChildrenFlags  fetch_cst_children_flags)
{
  GPtrArray *cst_children = g_ptr_array_new ();
  guint cst_children_count = g_node_n_children (cst_root);

  for (guint i = 0; i < cst_children_count; ++i)
    {
      GNode *cst_child = g_node_nth_child (cst_root, i);
      gpointer data = cst_child->data;

      if (((fetch_cst_children_flags & FETCH_CST_CHILDREN_TOKEN) &&
           LEXICAL_ANALYSIS_IS_TOKEN (data)) ||
          ((fetch_cst_children_flags & FETCH_CST_CHILDREN_TERMINAL) &&
           SYMBOLS_IS_TERMINAL (data)) ||
          ((fetch_cst_children_flags & FETCH_CST_CHILDREN_NON_TERMINAL) &&
           SYMBOLS_IS_NON_TERMINAL (data)))
        {
          g_ptr_array_add (cst_children, cst_child);

          if (fetch_cst_children_flags & FETCH_CST_CHILDREN_ALL)
            continue;
          else if (fetch_cst_children_flags & FETCH_CST_CHILDREN_FIRST)
            break;
        }
    }

  return cst_children;
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

  if (g_hash_table_contains (operator_types, cst_root_caption))
    {
      operator_type = (OperatorType) GPOINTER_TO_INT (g_hash_table_lookup (operator_types,
                                                                           cst_root_caption));
    }

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
