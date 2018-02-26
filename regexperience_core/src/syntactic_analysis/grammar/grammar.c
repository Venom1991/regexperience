#include "internal/syntactic_analysis/grammar/grammar.h"
#include "internal/syntactic_analysis/grammar/production.h"
#include "internal/syntactic_analysis/grammar/rule.h"
#include "internal/syntactic_analysis/grammar/symbols/symbol.h"
#include "internal/syntactic_analysis/grammar/symbols/non_terminal.h"
#include "internal/syntactic_analysis/grammar/symbols/terminal.h"
#include "internal/common/macros.h"

typedef struct
{
    GHashTable *productions;
} GrammarPrivate;

struct _Grammar
{
    GObject parent_instance;
};

G_DEFINE_TYPE_WITH_PRIVATE (Grammar, grammar, G_TYPE_OBJECT)

enum
{
    PROP_PRODUCTIONS = 1,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };
static Grammar *singleton = NULL;

static void grammar_define_productions (GHashTable *productions);

static GPtrArray *grammar_define_rules (gchar      ***right_hand_sides,
                                        GHashTable   *productions);

static GPtrArray *grammar_define_symbols (gchar      **symbol_array,
                                          GHashTable  *productions);

static GObject *grammar_constructor (GType                  type,
                                     guint                  n_construct_properties,
                                     GObjectConstructParam *construct_properties);

static void grammar_get_property (GObject    *object,
                                  guint       property_id,
                                  GValue     *value,
                                  GParamSpec *pspec);

static void grammar_dispose (GObject *object);

static void
grammar_class_init (GrammarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = grammar_constructor;
  object_class->get_property = grammar_get_property;
  object_class->dispose = grammar_dispose;

  obj_properties[PROP_PRODUCTIONS] =
      g_param_spec_boxed (PROP_GRAMMAR_PRODUCTIONS,
                          "Productions",
                          "Hash table of productions used to formally describe the grammar.",
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

  priv->productions = g_hash_table_new_full (g_str_hash,
                                             g_str_equal,
                                             g_free,
                                             g_object_unref);

  grammar_define_productions (priv->productions);

  singleton = self;
}

static void
grammar_define_productions (GHashTable *productions)
{
  gchar *left_hand_sides[] =
  {
    START,
    EXPRESSION,
    ALTERNATION,
    SIMPLE_EXPRESSION,
    CONCATENATION,
    BASIC_EXPRESSION,
    STAR_QUANTIFICATION,
    PLUS_QUANTIFICATION,
    QUESTION_MARK_QUANTIFICATION,
    ELEMENTARY_EXPRESSION,
    GROUP,
    UPPER_CASE_LETTER,
    LOWER_CASE_LETTER,
    DIGIT,
    SPECIAL_CHARACTER,
    REGULAR_METACHARACTER,
    BRACKET_EXPRESSION_METACHARACTER,
    METACHARACTER_ESCAPE,
    BRACKET_EXPRESSION,
    BRACKET_EXPRESSION_ITEMS,
    BRACKET_EXPRESSION_ITEM,
    RANGE
  };

  /* The first element of each of the innermost nested arrays contains
   * the corresponding left-hand side's identifier which is used to
   * establish a relation with its own right-hand side.
   * "NULL" is used as a sentinel value in the rest of the nested arrays.
   * Also, some of the right hand sides (such as letters or digits) are
   * represented using delimited strings instead of multiple
   * single-character strings. This approach significantly reduces
   * the number of symbols without changing the behavior.
   */
  gchar ***right_hand_sides[] =
  {
    (gchar**[])
    {
      (gchar*[]) {START},
      (gchar*[]) {EXPRESSION, EOI, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {EXPRESSION},
      (gchar*[]) {SIMPLE_EXPRESSION, ALTERNATION, NULL},
      (gchar*[]) {SIMPLE_EXPRESSION, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {ALTERNATION},
      (gchar*[]) {"|", SIMPLE_EXPRESSION, ALTERNATION, NULL},
      (gchar*[]) {"|", SIMPLE_EXPRESSION, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {SIMPLE_EXPRESSION},
      (gchar*[]) {BASIC_EXPRESSION, CONCATENATION, NULL},
      (gchar*[]) {BASIC_EXPRESSION, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {CONCATENATION},
      (gchar*[]) {BASIC_EXPRESSION, CONCATENATION, NULL},
      (gchar*[]) {BASIC_EXPRESSION, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {BASIC_EXPRESSION},
      (gchar*[]) {STAR_QUANTIFICATION, NULL},
      (gchar*[]) {PLUS_QUANTIFICATION, NULL},
      (gchar*[]) {QUESTION_MARK_QUANTIFICATION, NULL},
      (gchar*[]) {ELEMENTARY_EXPRESSION, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {STAR_QUANTIFICATION},
      (gchar*[]) {ELEMENTARY_EXPRESSION, "*", NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {PLUS_QUANTIFICATION},
      (gchar*[]) {ELEMENTARY_EXPRESSION, "+", NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {QUESTION_MARK_QUANTIFICATION},
      (gchar*[]) {ELEMENTARY_EXPRESSION, "?", NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {ELEMENTARY_EXPRESSION},
      (gchar*[]) {GROUP, NULL},
      (gchar*[]) {UPPER_CASE_LETTER, NULL},
      (gchar*[]) {LOWER_CASE_LETTER, NULL},
      (gchar*[]) {DIGIT, NULL},
      (gchar*[]) {SPECIAL_CHARACTER, NULL},
      (gchar*[]) {BRACKET_EXPRESSION_METACHARACTER, NULL},
      (gchar*[]) {"[", BRACKET_EXPRESSION, "]", NULL},
      (gchar*[]) {"\\", REGULAR_METACHARACTER, NULL},
      (gchar*[]) {"\\", METACHARACTER_ESCAPE, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {GROUP},
      (gchar*[]) {"(", EXPRESSION, ")", NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {UPPER_CASE_LETTER},
      (gchar*[])
      {
        "A" DELIMITER "B" DELIMITER "C" DELIMITER "D" DELIMITER "E" DELIMITER "F" DELIMITER
        "G" DELIMITER "H" DELIMITER "I" DELIMITER "J" DELIMITER "K" DELIMITER "L" DELIMITER
        "M" DELIMITER "N" DELIMITER "O" DELIMITER "P" DELIMITER "Q" DELIMITER "R" DELIMITER
        "S" DELIMITER "T" DELIMITER "U" DELIMITER "V" DELIMITER "W" DELIMITER "X" DELIMITER
        "Y" DELIMITER "Z", NULL
      },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {LOWER_CASE_LETTER},
      (gchar*[])
      {
        "a" DELIMITER "b" DELIMITER "c" DELIMITER "d" DELIMITER "e" DELIMITER "f" DELIMITER
        "g" DELIMITER "h" DELIMITER "i" DELIMITER "j" DELIMITER "k" DELIMITER "l" DELIMITER
        "m" DELIMITER "n" DELIMITER "o" DELIMITER "p" DELIMITER "q" DELIMITER "r" DELIMITER
        "s" DELIMITER "t" DELIMITER "u" DELIMITER "v" DELIMITER "w" DELIMITER "x" DELIMITER
        "y" DELIMITER "z", NULL
      },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {DIGIT},
      (gchar*[])
      {
        "0" DELIMITER "1" DELIMITER "2" DELIMITER "3" DELIMITER "4" DELIMITER "5" DELIMITER
        "6" DELIMITER "7" DELIMITER "8" DELIMITER "9", NULL
      },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {SPECIAL_CHARACTER},
      (gchar*[])
      {
        "!" DELIMITER "#" DELIMITER "$" DELIMITER "%" DELIMITER "&" DELIMITER "," DELIMITER
        "." DELIMITER "/" DELIMITER ":" DELIMITER ";" DELIMITER ">" DELIMITER "=" DELIMITER
        "<" DELIMITER "@" DELIMITER "^" DELIMITER "_" DELIMITER "`" DELIMITER "{" DELIMITER
        "}" DELIMITER " " DELIMITER "\n" DELIMITER "\t", NULL
      },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {REGULAR_METACHARACTER},
      (gchar*[])
      {
        "[" DELIMITER "(" DELIMITER ")" DELIMITER "*" DELIMITER "+" DELIMITER "?" DELIMITER
        "|", NULL
      },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {BRACKET_EXPRESSION_METACHARACTER},
      (gchar*[])
      {
        "-" DELIMITER "]", NULL
      },
      NULL
    },
    (gchar**[])
    {
      (gchar *[]) {METACHARACTER_ESCAPE},
      (gchar *[])
      {
        "\\", NULL
      },
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {BRACKET_EXPRESSION},
      (gchar*[]) {BRACKET_EXPRESSION_ITEMS, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {BRACKET_EXPRESSION_ITEMS},
      (gchar*[]) {BRACKET_EXPRESSION_ITEM, NULL},
      (gchar*[]) {BRACKET_EXPRESSION_ITEM, BRACKET_EXPRESSION_ITEMS, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {BRACKET_EXPRESSION_ITEM},
      (gchar*[]) {RANGE, NULL},
      (gchar*[]) {UPPER_CASE_LETTER, NULL},
      (gchar*[]) {LOWER_CASE_LETTER, NULL},
      (gchar*[]) {DIGIT, NULL},
      (gchar*[]) {SPECIAL_CHARACTER, NULL},
      (gchar*[]) {REGULAR_METACHARACTER, NULL},
      (gchar*[]) {"\\", BRACKET_EXPRESSION_METACHARACTER, NULL},
      (gchar*[]) {"\\", METACHARACTER_ESCAPE, NULL},
      NULL
    },
    (gchar**[])
    {
      (gchar*[]) {RANGE},
      (gchar*[]) {UPPER_CASE_LETTER, "-", UPPER_CASE_LETTER, NULL},
      (gchar*[]) {LOWER_CASE_LETTER, "-", LOWER_CASE_LETTER, NULL},
      (gchar*[]) {DIGIT, "-", DIGIT, NULL},
      NULL
    }
  };

  gsize left_hand_sides_size = G_N_ELEMENTS (left_hand_sides);
  gsize right_hand_sides_size = G_N_ELEMENTS (right_hand_sides);

  g_assert (left_hand_sides_size == right_hand_sides_size);

  /* Initializing the productions beforehand so that they can be used as non-terminals. */
  for (guint i = 0; i < left_hand_sides_size; ++i)
    {
      gchar *left_hand_side = g_strdup (left_hand_sides[i]);
      Production *production = production_new (PROP_PRODUCTION_CAPTION, left_hand_side);

      g_hash_table_insert (productions, left_hand_side, production);
    }

  /* Defining the right-hand-sides for previously initialized productions. */
  for (guint i = 0; i < right_hand_sides_size; ++i)
    {
      gchar ***current_right_hand_sides = right_hand_sides[i];
      gchar *corresponding_left_hand_side = current_right_hand_sides[0][0];
      Production *production = g_hash_table_lookup (productions,
                                                    corresponding_left_hand_side);
      g_autoptr (GPtrArray) rules = grammar_define_rules (++current_right_hand_sides,
                                                          productions);

      g_object_set (production,
                    PROP_PRODUCTION_RULES, rules,
                    NULL);
    }
}

static GPtrArray *
grammar_define_rules (gchar      ***right_hand_sides,
                      GHashTable   *productions)
{
  GPtrArray *rules = g_ptr_array_new_with_free_func (g_object_unref);
  gchar **current_symbol_array = *right_hand_sides;

  do
    {
      g_autoptr (GPtrArray) symbols = grammar_define_symbols (current_symbol_array,
                                                              productions);

      Rule *rule = rule_new (PROP_RULE_SYMBOLS, symbols);

      g_ptr_array_add (rules, rule);

      current_symbol_array = *(++right_hand_sides);
    }
  while (current_symbol_array != NULL);

  return rules;
}

static GPtrArray *
grammar_define_symbols (gchar      **symbol_array,
                        GHashTable  *productions)
{
  GPtrArray *symbols = g_ptr_array_new_with_free_func (g_object_unref);
  gchar *current_symbol_value = *symbol_array;

  do
    {
      Symbol *symbol = NULL;
      Production *production = g_hash_table_lookup (productions,
                                                    current_symbol_value);

      if (production == NULL)
        symbol = terminal_new (PROP_SYMBOL_VALUE, current_symbol_value);
      else
        symbol = non_terminal_new (PROP_SYMBOL_VALUE, production);

      g_ptr_array_add (symbols, symbol);

      current_symbol_value = *(++symbol_array);
    }
  while (current_symbol_value != NULL);

  return symbols;
}

static GObject *grammar_constructor (GType                  type,
                                     guint                  n_construct_properties,
                                     GObjectConstructParam *construct_properties)
{
  if (singleton != NULL)
    return g_object_ref (singleton);

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
    case PROP_PRODUCTIONS:
      g_value_set_boxed (value, priv->productions);
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

  if (priv->productions != NULL)
    g_clear_pointer (&priv->productions, g_hash_table_unref);

  G_OBJECT_CLASS (grammar_parent_class)->dispose (object);
}
