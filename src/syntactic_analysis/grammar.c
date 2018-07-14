#include "internal/syntactic_analysis/grammar.h"
#include "internal/syntactic_analysis/production.h"
#include "internal/syntactic_analysis/rule.h"
#include "internal/syntactic_analysis/derivation_item.h"
#include "internal/syntactic_analysis/parsing_table_key.h"
#include "internal/syntactic_analysis/symbols/symbol.h"
#include "internal/syntactic_analysis/symbols/non_terminal.h"
#include "internal/syntactic_analysis/symbols/terminal.h"
#include "internal/common/helpers.h"

struct _Grammar
{
  GObject parent_instance;
};

typedef struct
{
  GPtrArray  *all_productions;
  GPtrArray  *all_terminals;
  Production *start_production;
  GHashTable *parsing_table;
} GrammarPrivate;

enum
{
  PROP_ALL_PRODUCTIONS = 1,
  PROP_ALL_TERMINALS,
  PROP_START_PRODUCTION,
  PROP_PARSING_TABLE,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };
static Grammar *singleton = NULL;

static Production *grammar_define_productions            (GPtrArray              **productions,
                                                          GPtrArray              **terminals);

static GPtrArray  *grammar_define_rules                  (gchar                 ***right_hand_sides,
                                                          GHashTable              *productions_table,
                                                          GPtrArray               *terminals_array,
                                                          GPtrArray               *non_terminals_array);

static GPtrArray  *grammar_define_symbols                (gchar                  **symbols_array,
                                                          GHashTable              *productions_table,
                                                          GPtrArray               *terminals_array,
                                                          GPtrArray               *non_terminals_array);

static Symbol     *grammar_get_or_create_symbol          (Symbol                  *symbol,
                                                          GPtrArray               *symbols_array);

static void        grammar_mark_non_terminal_occurrences (Production              *production,
                                                          GPtrArray               *rules);

static GHashTable *grammar_build_parsing_table           (GPtrArray               *productions);

static void        grammar_insert_parsing_table_entries  (GHashTable              *parsing_table,
                                                          Production              *production,
                                                          GPtrArray               *terminals,
                                                          Rule                    *rule);

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

  obj_properties[PROP_ALL_TERMINALS] =
    g_param_spec_boxed (PROP_GRAMMAR_ALL_TERMINALS,
                        "All terminals",
                        "Array of terminal symbols that appear in the grammar.",
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

  GPtrArray *productions = NULL;
  GPtrArray *terminals = NULL;
  Production *start_production = NULL;
  GHashTable *parsing_table = NULL;

  start_production = grammar_define_productions (&productions,
                                                 &terminals);
  parsing_table = grammar_build_parsing_table (productions);

  priv->all_productions = productions;
  priv->all_terminals = terminals;
  priv->start_production = g_object_ref (start_production);
  priv->parsing_table = parsing_table;

  singleton = self;
}

static Production*
grammar_define_productions (GPtrArray **productions,
                            GPtrArray **terminals)
{
  g_autoptr (GHashTable) productions_table = g_hash_table_new (g_str_hash,
                                                               g_str_equal);
  g_autoptr (GPtrArray) terminals_array = g_ptr_array_new ();
  g_autoptr (GPtrArray) non_terminals_array = g_ptr_array_new ();

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
      (gchar*[]) { ANY_CHARACTER, NULL                     },
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
                   "/" DELIMITER ":" DELIMITER ";" DELIMITER ">" DELIMITER "=" DELIMITER "<" DELIMITER
                   "@" DELIMITER "^" DELIMITER "_" DELIMITER "`" DELIMITER "{" DELIMITER "}" DELIMITER
                   " " DELIMITER "\n" DELIMITER "\t", NULL },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) { REGULAR_METACHARACTER },
      (gchar*[]) { "[" DELIMITER "(" DELIMITER ")" DELIMITER "*" DELIMITER "+" DELIMITER "?" DELIMITER
                   "|" DELIMITER ".", NULL },
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
      (gchar*[]) { ANY_CHARACTER },
      (gchar*[]) { ".", NULL },
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
                                                          productions_table,
                                                          terminals_array,
                                                          non_terminals_array);

      /* Keeping track of the exact positions in which the non-terminals' underlying productions
       * appear as members of the current production's rules.
       */
      grammar_mark_non_terminal_occurrences (production,
                                             rules);
      g_object_set (production,
                    PROP_PRODUCTION_RULES, rules,
                    NULL);
    }

  *productions = g_hash_table_values_to_ptr_array (productions_table,
                                                   g_object_unref);
  *terminals = g_ptr_array_ref (terminals_array);

  return g_hash_table_lookup (productions_table, START);
}

static GPtrArray *
grammar_define_rules (gchar      ***right_hand_sides,
                      GHashTable   *productions_table,
                      GPtrArray    *terminals_array,
                      GPtrArray    *non_terminals_array)
{
  GPtrArray *rules = g_ptr_array_new_with_free_func (g_object_unref);
  gchar **current_symbols_array = *right_hand_sides;

  do
    {
      g_autoptr (GPtrArray) symbols = grammar_define_symbols (current_symbols_array,
                                                              productions_table,
                                                              terminals_array,
                                                              non_terminals_array);

      Rule *rule = rule_new (PROP_RULE_SYMBOLS, symbols);

      g_ptr_array_add (rules, rule);

      current_symbols_array = *(++right_hand_sides);
    }
  while (current_symbols_array != NULL);

  return rules;
}

static GPtrArray *
grammar_define_symbols (gchar      **symbols_array,
                        GHashTable  *productions_table,
                        GPtrArray   *terminals_array,
                        GPtrArray   *non_terminals_array)
{
  GPtrArray *symbols = g_ptr_array_new_with_free_func (g_object_unref);
  gchar *current_symbol_value = *symbols_array;

  do
    {
      Symbol *symbol = NULL;
      Production *production = g_hash_table_lookup (productions_table,
                                                    current_symbol_value);

      if (production == NULL)
        {
          g_autoptr (Symbol) terminal = terminal_new (PROP_SYMBOL_VALUE, current_symbol_value);

          /* Avoiding the duplication of terminal symbols. */
          symbol = grammar_get_or_create_symbol (terminal,
                                                 terminals_array);
        }
      else
        {
          g_autoptr (Symbol) non_terminal = non_terminal_new (PROP_SYMBOL_VALUE, production);

          /* Avoiding the duplication of non-terminal symbols. */
          symbol = grammar_get_or_create_symbol (non_terminal,
                                                 non_terminals_array);
        }

      g_ptr_array_add (symbols, symbol);

      current_symbol_value = *(++symbols_array);
    }
  while (current_symbol_value != NULL);

  return symbols;
}

static Symbol *
grammar_get_or_create_symbol (Symbol    *symbol,
                              GPtrArray *symbols_array)
{
  if (g_ptr_array_has_items (symbols_array))
    for (guint i = 0; i < symbols_array->len; ++i)
      {
        Symbol *current = g_ptr_array_index (symbols_array, i);

        if (symbol_is_equal (current, symbol))
          return g_object_ref (current);
      }

  g_ptr_array_add (symbols_array, symbol);

  return g_object_ref (symbol);
}

static void
grammar_mark_non_terminal_occurrences (Production *production,
                                       GPtrArray  *rules)
{
  for (guint i = 0; i < rules->len; ++i)
    {
      Rule *rule = g_ptr_array_index (rules, i);
      DerivationItem *occurrence = NULL;
      g_autoptr (GPtrArray) symbols = NULL;

      g_object_get (rule,
                    PROP_RULE_SYMBOLS, &symbols,
                    NULL);

      for (guint j = 0; j < symbols->len; ++j)
        {
          Symbol *symbol = g_ptr_array_index (symbols, j);

          if (SYMBOLS_IS_NON_TERMINAL (symbol))
            {
              GValue value = G_VALUE_INIT;
              g_autoptr (Production) underlying_production = NULL;

              symbol_extract_value (symbol, &value);

              underlying_production = g_value_get_object (&value);

              if (occurrence == NULL)
                {
                  occurrence = derivation_item_new (PROP_DERIVATION_ITEM_LEFT_HAND_SIDE, production,
                                                    PROP_DERIVATION_ITEM_RIGHT_HAND_SIDE, rule);

                  production_mark_occurrence (underlying_production, occurrence);
                }
              else
                {
                  production_mark_occurrence (underlying_production, g_object_ref (occurrence));
                }
            }
        }
    }
}

static GHashTable *
grammar_build_parsing_table (GPtrArray *productions)
{
   GHashTable *parsing_table = g_hash_table_new_full ((GHashFunc) parsing_table_key_hash,
                                                      (GEqualFunc) parsing_table_key_equal,
                                                      g_object_unref,
                                                      NULL);

  for (guint i = 0; i < productions->len; ++i)
    {
      Production *production = g_ptr_array_index (productions, i);
      g_autoptr (GPtrArray) rules = NULL;

      /* Preparing the process firstly by computing the current production's first set. */
      production_compute_first_set (production);

      g_object_get (production,
                    PROP_PRODUCTION_RULES, &rules,
                    NULL);

      /* The previous computation's results are stored for each
       * of the current production's rules, as well.
       */
      for (guint j = 0; j < rules->len; ++j)
        {
          Rule *rule = g_ptr_array_index (rules, j);
          g_autoptr (GPtrArray) first_set = NULL;
          gboolean can_derive_epsilon = FALSE;

          g_object_get (rule,
                        PROP_RULE_FIRST_SET, &first_set,
                        PROP_RULE_CAN_DERIVE_EPSILON, &can_derive_epsilon,
                        NULL);

          /* Using the terminal symbols found in the current rule's
           * first set to insert new parsing table entries.
           */
          grammar_insert_parsing_table_entries (parsing_table,
                                                production,
                                                first_set,
                                                rule);

          /* Avoiding the computation of the current production's follow set
           * in case the current rule cannot be used to derive epsilon.
           */
           if (can_derive_epsilon)
             {
               GPtrArray *follow_set = production_compute_follow_set (production);

               /* Using the terminal symbols found in the current productions's
                * follow set to insert new parsing table entries.
                */
               grammar_insert_parsing_table_entries (parsing_table,
                                                     production,
                                                     follow_set,
                                                     rule);
             }
        }
    }

  return parsing_table;
}

static void
grammar_insert_parsing_table_entries (GHashTable *parsing_table,
                                      Production *production,
                                      GPtrArray  *terminals,
                                      Rule       *rule)
{
  if (g_ptr_array_has_items (terminals))
    for (guint i = 0; i < terminals->len; ++i)
      {
        Symbol *terminal = g_ptr_array_index (terminals, i);

        /* Epsilon should not appear as the second dimension of a parsing table entry. */
        if (!symbol_is_match (terminal, EPSILON))
          {
            ParsingTableKey *parsing_table_key = parsing_table_key_new (PROP_PARSING_TABLE_KEY_PRODUCTION, production,
                                                                        PROP_PARSING_TABLE_KEY_TERMINAL, terminal);
            gboolean parsing_table_key_is_new = g_hash_table_insert (parsing_table,
                                                                     parsing_table_key,
                                                                     rule);

            /* Asserting whether or not all of the parsing table keys
             * are unique - i.e., there are no LL(1) grammar conflicts.
             */
            g_assert (parsing_table_key_is_new);
          }
      }
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
    case PROP_ALL_PRODUCTIONS:
      g_value_set_boxed (value, priv->all_productions);
      break;

    case PROP_ALL_TERMINALS:
      g_value_set_boxed (value, priv->all_terminals);
      break;

    case PROP_START_PRODUCTION:
      g_value_set_object (value, priv->start_production);
      break;

    case PROP_PARSING_TABLE:
      g_value_set_boxed (value, priv->parsing_table);
      break;

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

  if (priv->all_terminals != NULL)
    g_clear_pointer (&priv->all_terminals, g_ptr_array_unref);

  if (priv->start_production != NULL)
    g_clear_object (&priv->start_production);

  if (priv->parsing_table != NULL)
    g_clear_pointer (&priv->parsing_table, g_hash_table_unref);

  G_OBJECT_CLASS (grammar_parent_class)->dispose (object);
}
