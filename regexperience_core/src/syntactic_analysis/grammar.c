#include "internal/syntactic_analysis/grammar.h"
#include "internal/syntactic_analysis/production.h"
#include "internal/syntactic_analysis/rule.h"
#include "internal/syntactic_analysis/derivation_item.h"
#include "internal/syntactic_analysis/symbols/symbol.h"
#include "internal/syntactic_analysis/symbols/non_terminal.h"
#include "internal/syntactic_analysis/symbols/terminal.h"
#include "internal/common/helpers.h"

#include <glib/gprintf.h>

struct _Grammar
{
    GObject parent_instance;
};

typedef struct
{
    GPtrArray  *all_productions;
    Production *start_production;
    GHashTable *parsing_table;
} GrammarPrivate;

enum
{
    PROP_ALL_PRODUCTIONS = 1,
    PROP_START_PRODUCTION,
    PROP_PARSING_TABLE,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };
static Grammar *singleton = NULL;

static GPtrArray  *grammar_define_productions            (void);

static GPtrArray  *grammar_define_rules                  (gchar                 ***right_hand_sides,
                                                          GHashTable              *productions_table);

static GPtrArray  *grammar_define_symbols                (gchar                  **symbols_array,
                                                          GHashTable              *productions_table);

static void        grammar_mark_non_terminal_occurrences (Production              *left_hand_side,
                                                          GPtrArray               *rules);

static GHashTable *grammar_build_parsing_table           (GPtrArray               *productions);

static GObject    *grammar_constructor                   (GType                    type,
                                                          guint                    n_construct_properties,
                                                          GObjectConstructParam   *construct_properties);

static void        grammar_get_property                  (GObject                 *object,
                                                          guint                    property_id,
                                                          GValue                  *value,
                                                          GParamSpec              *pspec);

static void        grammar_dispose                       (GObject                 *object);

G_DEFINE_TYPE_WITH_PRIVATE (Grammar, grammar, G_TYPE_OBJECT)

static void
grammar_class_init (GrammarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = grammar_constructor;
  object_class->get_property = grammar_get_property;
  object_class->dispose = grammar_dispose;

  obj_properties[PROP_ALL_PRODUCTIONS] =
    g_param_spec_boxed (PROP_GRAMMAR_ALL_PRODUCTIONS,
                        "All productions",
                        "Array of productions used to formally describe the grammar.",
                        G_TYPE_PTR_ARRAY,
                        G_PARAM_READABLE);

  obj_properties[PROP_START_PRODUCTION] =
    g_param_spec_object (PROP_GRAMMAR_START_PRODUCTION,
                         "Start production",
                         "Start production of the grammar.",
                         SYNTACTIC_ANALYSIS_TYPE_PRODUCTION,
                         G_PARAM_READABLE);

  obj_properties[PROP_PARSING_TABLE] =
    g_param_spec_boxed (PROP_GRAMMAR_PARSING_TABLE,
                        "Parsing table",
                        "LL(1) parsing table created using the grammar's productions.",
                        G_TYPE_HASH_TABLE,
                        G_PARAM_READABLE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
grammar_init (Grammar *self)
{
  GrammarPrivate *priv = grammar_get_instance_private (SYNTACTIC_ANALYSIS_GRAMMAR (self));
  GPtrArray *productions = grammar_define_productions ();

  priv->all_productions = productions;
  priv->start_production = g_object_ref (g_ptr_array_index (productions, 0));
  priv->parsing_table = grammar_build_parsing_table (productions);

  singleton = self;
}

static GPtrArray *
grammar_define_productions (void)
{
  g_autoptr (GHashTable) productions_table = g_hash_table_new_full (g_str_hash,
                                                                    g_str_equal,
                                                                    NULL,
                                                                    NULL);

  /* The first element of each of the innermost nested arrays contains
   * the corresponding left-hand side's identifier which is used to
   * establish a relation with its own right-hand side.
   * "NULL" is used as a sentinel value in the rest of the nested arrays.
   * Also, some of the right hand sides (such as letters or digits) are
   * represented using delimited strings instead of multiple
   * single-character strings. This approach significantly reduces
   * the number of terminal symbols without changing the behavior.
   */
  gchar ***grammar[] =
  {
    (gchar**[])
    {
      (gchar*[]) { START },
      (gchar*[]) { EXPRESSION, END_OF_INPUT, NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { EXPRESSION },
      (gchar*[]) { SIMPLE_EXPRESSION, EXPRESSION_PRIME, NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { EXPRESSION_PRIME },
      (gchar*[]) { ALTERNATION, NULL },
      (gchar*[]) { EPSILON, NULL     },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { ALTERNATION },
      (gchar*[]) { "|", SIMPLE_EXPRESSION, ALTERNATION_PRIME, NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { ALTERNATION_PRIME },
      (gchar*[]) { ALTERNATION, NULL },
      (gchar*[]) { EPSILON, NULL     },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { SIMPLE_EXPRESSION },
      (gchar*[]) { BASIC_EXPRESSION, SIMPLE_EXPRESSION_PRIME, NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { SIMPLE_EXPRESSION_PRIME },
      (gchar*[]) { CONCATENATION, NULL     },
      (gchar*[]) { EPSILON, NULL           },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { CONCATENATION },
      (gchar*[]) { BASIC_EXPRESSION, CONCATENATION_PRIME, NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { CONCATENATION_PRIME },
      (gchar*[]) { CONCATENATION, NULL },
      (gchar*[]) { EPSILON, NULL       },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { BASIC_EXPRESSION },
      (gchar*[]) { ELEMENTARY_EXPRESSION, BASIC_EXPRESSION_PRIME, NULL  },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { BASIC_EXPRESSION_PRIME },
      (gchar*[]) { STAR_QUANTIFICATION, NULL          },
      (gchar*[]) { PLUS_QUANTIFICATION, NULL          },
      (gchar*[]) { QUESTION_MARK_QUANTIFICATION, NULL },
      (gchar*[]) { EPSILON, NULL                      },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { STAR_QUANTIFICATION },
      (gchar*[]) { "*", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { PLUS_QUANTIFICATION },
      (gchar*[]) { "+", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { QUESTION_MARK_QUANTIFICATION },
      (gchar*[]) { "?", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { ELEMENTARY_EXPRESSION },
      (gchar*[]) { GROUP, NULL                             },
      (gchar*[]) { BRACKET_EXPRESSION, NULL                },
      (gchar*[]) { UPPER_CASE_LETTER, NULL                 },
      (gchar*[]) { LOWER_CASE_LETTER, NULL                 },
      (gchar*[]) { DIGIT, NULL                             },
      (gchar*[]) { SPECIAL_CHARACTER, NULL                 },
      (gchar*[]) { BRACKET_EXPRESSION_METACHARACTER, NULL  },
      (gchar*[]) { "\\", ELEMENTARY_EXPRESSION_PRIME, NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { ELEMENTARY_EXPRESSION_PRIME },
      (gchar*[]) { REGULAR_METACHARACTER, NULL },
      (gchar*[]) { METACHARACTER_ESCAPE, NULL  },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { GROUP },
      (gchar*[]) {"(", EXPRESSION, ")", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { BRACKET_EXPRESSION },
      (gchar*[]) {"[", BRACKET_EXPRESSION_ITEMS, "]", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { BRACKET_EXPRESSION_ITEMS },
      (gchar*[]) { BRACKET_EXPRESSION_ITEM, BRACKET_EXPRESSION_ITEMS_PRIME, NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { BRACKET_EXPRESSION_ITEMS_PRIME },
      (gchar*[]) { BRACKET_EXPRESSION_ITEMS, NULL },
      (gchar*[]) { EPSILON, NULL                  },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { BRACKET_EXPRESSION_ITEM },
      (gchar*[]) { UPPER_CASE_LETTER, UPPER_CASE_LETTER_RANGE, NULL },
      (gchar*[]) { LOWER_CASE_LETTER, LOWER_CASE_LETTER_RANGE, NULL },
      (gchar*[]) { DIGIT, DIGIT_RANGE, NULL                         },
      (gchar*[]) { SPECIAL_CHARACTER, NULL                          },
      (gchar*[]) { REGULAR_METACHARACTER, NULL                      },
      (gchar*[]) { "\\", BRACKET_EXPRESSION_ITEM_PRIME, NULL        },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { BRACKET_EXPRESSION_ITEM_PRIME },
      (gchar*[]) { BRACKET_EXPRESSION_METACHARACTER, NULL },
      (gchar*[]) { METACHARACTER_ESCAPE, NULL             },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { UPPER_CASE_LETTER_RANGE },
      (gchar*[]) { "-", UPPER_CASE_LETTER, NULL },
      (gchar*[]) { EPSILON, NULL                },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { LOWER_CASE_LETTER_RANGE },
      (gchar*[]) { "-", LOWER_CASE_LETTER, NULL },
      (gchar*[]) { EPSILON, NULL                },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { DIGIT_RANGE },
      (gchar*[]) { "-", DIGIT, NULL },
      (gchar*[]) { EPSILON, NULL    },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { UPPER_CASE_LETTER },
      (gchar*[]) { "A" DELIMITER "B" DELIMITER "C" DELIMITER "D" DELIMITER "E" DELIMITER "F" DELIMITER
                   "G" DELIMITER "H" DELIMITER "I" DELIMITER "J" DELIMITER "K" DELIMITER "L" DELIMITER
                   "M" DELIMITER "N" DELIMITER "O" DELIMITER "P" DELIMITER "Q" DELIMITER "R" DELIMITER
                   "S" DELIMITER "T" DELIMITER "U" DELIMITER "V" DELIMITER "W" DELIMITER "X" DELIMITER
                   "Y" DELIMITER "Z", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { LOWER_CASE_LETTER },
      (gchar*[]) { "a" DELIMITER "b" DELIMITER "c" DELIMITER "d" DELIMITER "e" DELIMITER "f" DELIMITER
                   "g" DELIMITER "h" DELIMITER "i" DELIMITER "j" DELIMITER "k" DELIMITER "l" DELIMITER
                   "m" DELIMITER "n" DELIMITER "o" DELIMITER "p" DELIMITER "q" DELIMITER "r" DELIMITER
                   "s" DELIMITER "t" DELIMITER "u" DELIMITER "v" DELIMITER "w" DELIMITER "x" DELIMITER
                   "y" DELIMITER "z", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { DIGIT },
      (gchar*[]) { "0" DELIMITER "1" DELIMITER "2" DELIMITER "3" DELIMITER "4" DELIMITER "5" DELIMITER
                   "6" DELIMITER "7" DELIMITER "8" DELIMITER "9", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { SPECIAL_CHARACTER },
      (gchar*[]) { "!" DELIMITER "#" DELIMITER "$" DELIMITER "%" DELIMITER "&" DELIMITER "," DELIMITER
                   "." DELIMITER "/" DELIMITER ":" DELIMITER ";" DELIMITER ">" DELIMITER "=" DELIMITER
                   "<" DELIMITER "@" DELIMITER "^" DELIMITER "_" DELIMITER "`" DELIMITER "{" DELIMITER
                   "}" DELIMITER " " DELIMITER "\n" DELIMITER "\t", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { REGULAR_METACHARACTER },
      (gchar*[]) { "[" DELIMITER "(" DELIMITER ")" DELIMITER "*" DELIMITER "+" DELIMITER "?" DELIMITER
                   "|", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { BRACKET_EXPRESSION_METACHARACTER },
      (gchar*[]) { "-" DELIMITER "]", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar *[]) { METACHARACTER_ESCAPE },
      (gchar *[]) { "\\", NULL },
      NULL
    }
  };

  gsize grammar_size = G_N_ELEMENTS (grammar);

  /* Initializing the productions beforehand so that they can be used as non-terminals. */
  for (guint i = 0; i < grammar_size; ++i)
    {
      gchar ***grammar_item = grammar[i];
      gchar *left_hand_side = grammar_item[0][0];
      Production *production = production_new (PROP_PRODUCTION_CAPTION, left_hand_side);

      g_hash_table_insert (productions_table, left_hand_side, production);
    }

  /* Defining the right-hand-sides for previously initialized productions. */
  for (guint i = 0; i < grammar_size; ++i)
    {
      gchar ***grammar_item = grammar[i];
      gchar *left_hand_side = grammar_item[0][0];
      gchar ***right_hand_sides = grammar_item + 1;

      Production *production = g_hash_table_lookup (productions_table, left_hand_side);
      g_autoptr (GPtrArray) rules = grammar_define_rules (right_hand_sides,
                                                          productions_table);

      grammar_mark_non_terminal_occurrences (production,
                                             rules);
      g_object_set (production,
                    PROP_PRODUCTION_RULES, rules,
                    NULL);
    }

  return g_hash_table_values_to_ptr_array (productions_table,
                                           g_object_unref);
}

static GPtrArray *
grammar_define_rules (gchar      ***right_hand_sides,
                      GHashTable   *productions_table)
{
  GPtrArray *rules = g_ptr_array_new_with_free_func (g_object_unref);
  gchar **current_symbols_array = *right_hand_sides;

  do
    {
      g_autoptr (GPtrArray) symbols = grammar_define_symbols (current_symbols_array,
                                                              productions_table);

      Rule *rule = rule_new (PROP_RULE_SYMBOLS, symbols);

      g_ptr_array_add (rules, rule);

      current_symbols_array = *(++right_hand_sides);
    }
  while (current_symbols_array != NULL);

  return rules;
}

static GPtrArray *
grammar_define_symbols (gchar      **symbols_array,
                        GHashTable  *productions_table)
{
  GPtrArray *symbols = g_ptr_array_new_with_free_func (g_object_unref);
  gchar *current_symbol_value = *symbols_array;

  do
    {
      Symbol *symbol = NULL;
      Production *production = g_hash_table_lookup (productions_table,
                                                    current_symbol_value);

      if (production == NULL)
        symbol = terminal_new (PROP_SYMBOL_VALUE, current_symbol_value);
      else
        symbol = non_terminal_new (PROP_SYMBOL_VALUE, production);

      g_ptr_array_add (symbols, symbol);

      current_symbol_value = *(++symbols_array);
    }
  while (current_symbol_value != NULL);

  return symbols;
}

static void
grammar_mark_non_terminal_occurrences (Production *left_hand_side,
                                       GPtrArray  *rules)
{
  for (guint i = 0; i < rules->len; ++i)
    {
      Rule *right_hand_side = g_ptr_array_index (rules, i);
      g_autoptr (GPtrArray) symbols = NULL;
      g_autoptr (DerivationItem) occurrence = NULL;

      g_object_get (right_hand_side,
                    PROP_RULE_SYMBOLS, &symbols,
                    NULL);

      for (guint j = 0; j < symbols->len; ++j)
        {
          Symbol *symbol = g_ptr_array_index (symbols, j);

          if (SYMBOLS_IS_NON_TERMINAL (symbol))
            {
              if (occurrence == NULL)
                occurrence = derivation_item_new (PROP_DERIVATION_ITEM_LEFT_HAND_SIDE, left_hand_side,
                                                  PROP_DERIVATION_ITEM_RIGHT_HAND_SIDE, right_hand_side);

              GValue value = G_VALUE_INIT;
              g_autoptr (Production) production = NULL;

              symbol_extract_value (symbol, &value);
              production = g_value_get_object (&value);

              production_mark_occurrence (production, occurrence);
            }
        }
    }
}

static GHashTable *
grammar_build_parsing_table (GPtrArray *productions)
{
  GHashTable *parsing_table = g_hash_table_new_full (g_str_hash,
                                                     g_str_equal,
                                                     g_free,
                                                     g_object_unref);

  for (guint i = 0; i < productions->len; ++i)
    {
      Production *production = g_ptr_array_index (productions, i);
      g_autoptr (GPtrArray) rules = NULL;

      production_compute_first_set (production);

      g_object_get (production,
                    PROP_PRODUCTION_RULES, &rules,
                    NULL);

      for (guint j = 0; j < rules->len; ++j)
        {
          Rule *rule = g_ptr_array_index (rules, j);
          g_autoptr (GPtrArray) first_set = NULL;
          gboolean can_derive_epsilon = FALSE;

          g_object_get (rule,
                        PROP_RULE_FIRST_SET, &first_set,
                        PROP_RULE_CAN_DERIVE_EPSILON, &can_derive_epsilon,
                        NULL);

          for (guint k = 0; k < first_set->len; ++k)
            {
              Symbol *symbol = g_ptr_array_index (first_set, k);

              g_assert (SYMBOLS_IS_TERMINAL (symbol));


            }
        }
    }

  return parsing_table;
}

static GObject *
grammar_constructor (GType                  type,
                     guint                  n_construct_properties,
                     GObjectConstructParam *construct_properties)
{
  if (singleton != NULL)
    return G_OBJECT (g_object_ref (singleton));

  GObjectClass *object_class = G_OBJECT_CLASS (grammar_parent_class);

  return object_class->constructor (type,
                                    n_construct_properties,
                                    construct_properties);
}

static void
grammar_get_property (GObject    *object,
                      guint       property_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  GrammarPrivate *priv = grammar_get_instance_private (SYNTACTIC_ANALYSIS_GRAMMAR (object));

  switch (property_id)
    {

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
grammar_dispose (GObject *object)
{
  GrammarPrivate *priv = grammar_get_instance_private (SYNTACTIC_ANALYSIS_GRAMMAR (object));

  if (priv->all_productions != NULL)
    g_clear_pointer (&priv->all_productions, g_ptr_array_unref);

  if (priv->start_production != NULL)
    g_clear_object (&priv->start_production);

  if (priv->parsing_table != NULL)
    g_clear_pointer (&priv->parsing_table, g_hash_table_unref);

  G_OBJECT_CLASS (grammar_parent_class)->dispose (object);
}
