#include "internal/syntactic_analysis/production.h"
#include "internal/syntactic_analysis/grammar.h"
#include "internal/syntactic_analysis/rule.h"
#include "internal/syntactic_analysis/derivation_item.h"
#include "internal/syntactic_analysis/symbols/symbol.h"
#include "internal/syntactic_analysis/symbols/terminal.h"
#include "internal/syntactic_analysis/symbols/non_terminal.h"
#include "internal/common/helpers.h"

struct _Production
{
    GObject parent_instance;
};

typedef struct
{
    gchar     *caption;
    GPtrArray *rules;
    GPtrArray *occurrences;
    GPtrArray *first_set;
    GPtrArray *follow_set;
} ProductionPrivate;

enum
{
    PROP_CAPTION = 1,
    PROP_RULES,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static GPtrArray *production_fetch_non_terminal_first_set (Symbol       *non_terminal,
                                                           gboolean     *can_derive_epsilon);

static void       production_get_property                 (GObject      *object,
                                                           guint         property_id,
                                                           GValue       *value,
                                                           GParamSpec   *pspec);

static void       production_set_property                 (GObject      *object,
                                                           guint         property_id,
                                                           const GValue *value,
                                                           GParamSpec   *pspec);

static void       production_dispose                      (GObject      *object);

static void       production_finalize                     (GObject      *object);

G_DEFINE_TYPE_WITH_PRIVATE (Production, production, G_TYPE_OBJECT)

static void
production_class_init (ProductionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = production_get_property;
  object_class->set_property = production_set_property;
  object_class->dispose = production_dispose;
  object_class->finalize = production_finalize;

  obj_properties[PROP_CAPTION] =
    g_param_spec_string (PROP_PRODUCTION_CAPTION,
                         "Caption",
                         "Name of the production.",
                         NULL,
                         G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_RULES] =
    g_param_spec_boxed (PROP_PRODUCTION_RULES,
                        "Rules",
                        "Array of rules that describe possible ways of rewriting the production"
                            "where it appears on the right-hand-side as a non-terminal symbol.",
                        G_TYPE_PTR_ARRAY,
                        G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
production_init (Production *self)
{
  /* NOP */
}

void
production_mark_occurrence (Production     *self,
                            DerivationItem *occurrence)
{
  g_return_if_fail (SYNTACTIC_ANALYSIS_IS_PRODUCTION (self));
  g_return_if_fail (occurrence != NULL);

  ProductionPrivate *priv = production_get_instance_private (self);
  GPtrArray **occurrences = &priv->occurrences;

  if (*occurrences == NULL)
    *occurrences = g_ptr_array_new_with_free_func (g_object_unref);

  g_ptr_array_add (*occurrences, occurrence);
}

GPtrArray *
production_compute_first_set (Production *self)
{
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_PRODUCTION (self), NULL);

  ProductionPrivate *priv = production_get_instance_private (self);
  GPtrArray **production_first_set = &priv->first_set;

  if (*production_first_set == NULL)
    {
      *production_first_set = g_ptr_array_new ();

      GPtrArray *rules = priv->rules;
      GEqualFunc symbol_equal_func = (GEqualFunc) symbol_is_equal;

      for (guint i = 0; i < rules->len; ++i)
        {
          Rule *rule = g_ptr_array_index (rules, i);
          g_autoptr (GPtrArray) symbols = NULL;
          g_autoptr (GPtrArray) rule_first_set = g_ptr_array_new_with_free_func (g_object_unref);
          guint epsilon_derivable_symbols_count = 0;

          g_object_get (rule,
                        PROP_RULE_SYMBOLS, &symbols,
                        NULL);

          g_assert (g_ptr_array_has_items (symbols));

          for (guint j = 0; j < symbols->len; ++j)
            {
              Symbol *symbol = g_ptr_array_index (symbols, j);
              gboolean symbol_can_derive_epsilon = FALSE;

              if (SYMBOLS_IS_TERMINAL (symbol))
                {
                  g_ptr_array_add_if_not_exists (rule_first_set,
                                                 symbol,
                                                 symbol_equal_func,
                                                 g_object_ref);
                }
              else if (SYMBOLS_IS_NON_TERMINAL (symbol))
                {
                  g_autoptr (GPtrArray) non_terminal_first_set =
                    production_fetch_non_terminal_first_set (symbol,
                                                             &symbol_can_derive_epsilon);

                  g_ptr_array_add_range_distinct (rule_first_set,
                                                  non_terminal_first_set,
                                                  symbol_equal_func,
                                                  g_object_ref);
                }

              if (!symbol_can_derive_epsilon)
                break;

              epsilon_derivable_symbols_count++;
            }

          if (epsilon_derivable_symbols_count == symbols->len)
            {
              g_autoptr (Terminal) epsilon = terminal_new (PROP_SYMBOL_VALUE, EPSILON);

              g_ptr_array_add_if_not_exists (rule_first_set,
                                             epsilon,
                                             symbol_equal_func,
                                             g_object_ref);
            }

          g_object_set (rule,
                        PROP_RULE_FIRST_SET, rule_first_set,
                        NULL);
          g_ptr_array_add_range (*production_first_set,
                                 rule_first_set,
                                 NULL);
        }
    }

  return *production_first_set;
}

GPtrArray *
production_compute_follow_set (Production *self)
{
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_PRODUCTION (self), NULL);

  ProductionPrivate *priv = production_get_instance_private (self);
  GPtrArray **production_follow_set = &priv->follow_set;

  if (*production_follow_set == NULL)
    {
      *production_follow_set = g_ptr_array_new_with_free_func (g_object_unref);

      GPtrArray *occurrences = priv->occurrences;
      GEqualFunc symbol_equal_func = (GEqualFunc) symbol_is_equal;

      if (g_ptr_array_has_items (occurrences))
        {
          for (guint i = 0; i < occurrences->len; ++i)
            {
              DerivationItem *occurrence = g_ptr_array_index (occurrences, i);
              g_autoptr (Production) left_hand_side = NULL;
              g_autoptr (Rule) right_hand_side = NULL;
              g_autoptr (GPtrArray) symbols = NULL;

              g_object_get (occurrence,
                            PROP_DERIVATION_ITEM_LEFT_HAND_SIDE, &left_hand_side,
                            PROP_DERIVATION_ITEM_RIGHT_HAND_SIDE, &right_hand_side,
                            NULL);
              g_object_get (right_hand_side,
                            PROP_RULE_SYMBOLS, &symbols,
                            NULL);

              for (guint j = 0; j < symbols->len; ++j)
                {
                  Symbol *symbol = g_ptr_array_index (symbols, j);

                  if (symbol_is_match (symbol, self))
                    {
                      guint k = j + 1;
                      gboolean should_add_left_hand_side_follow_set = FALSE;

                      if (k < symbols->len)
                        do
                          {
                            Symbol *following_symbol = g_ptr_array_index (symbols, k);

                            if (SYMBOLS_IS_TERMINAL (following_symbol))
                              {
                                g_ptr_array_add_if_not_exists (*production_follow_set,
                                                               following_symbol,
                                                               symbol_equal_func,
                                                               g_object_ref);
                              }
                            else if (SYMBOLS_IS_NON_TERMINAL (following_symbol))
                              {
                                gboolean symbol_can_derive_epsilon = FALSE;

                                g_autoptr (GPtrArray) non_terminal_first_set =
                                    production_fetch_non_terminal_first_set (following_symbol,
                                                                             &symbol_can_derive_epsilon);

                                g_ptr_array_add_range_distinct (*production_follow_set,
                                                                non_terminal_first_set,
                                                                symbol_equal_func,
                                                                g_object_ref);

                                if (symbol_can_derive_epsilon)
                                  should_add_left_hand_side_follow_set = TRUE;
                                else
                                  break;
                              }

                            k++;
                          }
                        while (k < symbols->len);
                      else
                        should_add_left_hand_side_follow_set = TRUE;

                      if (should_add_left_hand_side_follow_set)
                        {
                          GPtrArray *left_hand_side_follow_set =
                            production_compute_follow_set (left_hand_side);

                          g_ptr_array_add_range_distinct (*production_follow_set,
                                                          left_hand_side_follow_set,
                                                          symbol_equal_func,
                                                          g_object_ref);
                        }
                    }
                }
            }
        }
    }

  return *production_follow_set;
}

static GPtrArray *
production_fetch_non_terminal_first_set (Symbol   *non_terminal,
                                         gboolean *can_derive_epsilon)
{
  g_return_val_if_fail (SYMBOLS_IS_NON_TERMINAL (non_terminal), NULL);
  g_return_val_if_fail (can_derive_epsilon != NULL, NULL);

  GValue value = G_VALUE_INIT;
  g_autoptr (Production) production = NULL;
  GPtrArray *filtered_first_set = g_ptr_array_new ();

  symbol_extract_value (non_terminal, &value);

  production = g_value_get_object (&value);

  GPtrArray *non_terminal_first_set = production_compute_first_set (production);

  for (guint i = 0; i < non_terminal_first_set->len; ++i)
    {
      Symbol *symbol = g_ptr_array_index (non_terminal_first_set, i);

      if (symbol_is_match (symbol, EPSILON))
        {
          *can_derive_epsilon = TRUE;

          continue;
        }

      g_ptr_array_add (filtered_first_set, symbol);
    }

  return filtered_first_set;
}

static void
production_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  ProductionPrivate *priv = production_get_instance_private (SYNTACTIC_ANALYSIS_PRODUCTION (object));

  switch (property_id)
    {
    case PROP_CAPTION:
      g_value_set_string (value, priv->caption);
      break;

    case PROP_RULES:
      g_value_set_boxed (value, priv->rules);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
production_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  ProductionPrivate *priv = production_get_instance_private (SYNTACTIC_ANALYSIS_PRODUCTION (object));

  switch (property_id)
    {
    case PROP_CAPTION:
      if (priv->caption != NULL)
        g_free (priv->caption);

      priv->caption = g_value_dup_string (value);
      break;

    case PROP_RULES:
      if (priv->rules != NULL)
        g_ptr_array_unref (priv->rules);

      priv->rules = g_value_dup_boxed (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
production_dispose (GObject *object)
{
  ProductionPrivate *priv = production_get_instance_private (SYNTACTIC_ANALYSIS_PRODUCTION (object));

  if (priv->rules != NULL)
    g_clear_pointer (&priv->rules, g_ptr_array_unref);

  if (priv->occurrences != NULL)
    g_clear_pointer (&priv->occurrences, g_ptr_array_unref);

  if (priv->first_set != NULL)
    g_clear_pointer (&priv->first_set, g_ptr_array_unref);

  if (priv->follow_set != NULL)
    g_clear_pointer (&priv->follow_set, g_ptr_array_unref);

  G_OBJECT_CLASS (production_parent_class)->dispose (object);
}

static void
production_finalize (GObject *object)
{
  ProductionPrivate *priv = production_get_instance_private (SYNTACTIC_ANALYSIS_PRODUCTION (object));

  if (priv->caption != NULL)
    g_free (priv->caption);

  G_OBJECT_CLASS (production_parent_class)->finalize (object);
}
