#include "internal/syntactic_analysis/rule.h"
#include "internal/syntactic_analysis/grammar.h"
#include "internal/syntactic_analysis/symbols/symbol.h"
#include "internal/common/helpers.h"

struct _Rule
{
  GObject parent_instance;
};

typedef struct
{
  GPtrArray *symbols;
  GPtrArray *first_set;
  gboolean   derives_epsilon;
} RulePrivate;

enum
{
  PROP_SYMBOLS = 1,
  PROP_FIRST_SET,
  PROP_CAN_DERIVE_EPSILON,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void rule_discern_epsilon_derivability (Rule         *self);

static void rule_get_property                 (GObject      *object,
                                               guint         property_id,
                                               GValue       *value,
                                               GParamSpec   *pspec);

static void rule_set_property                 (GObject      *object,
                                               guint         property_id,
                                               const GValue *value,
                                               GParamSpec   *pspec);

static void rule_dispose                      (GObject      *object);

G_DEFINE_TYPE_WITH_PRIVATE (Rule, rule, G_TYPE_OBJECT)

static void
rule_class_init (RuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = rule_get_property;
  object_class->set_property = rule_set_property;
  object_class->dispose = rule_dispose;

  obj_properties[PROP_SYMBOLS] =
    g_param_spec_boxed (PROP_RULE_SYMBOLS,
                        "Symbols",
                        "Array of terminal and non-terminal symbols (any combination of the two).",
                        G_TYPE_PTR_ARRAY,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  obj_properties[PROP_FIRST_SET] =
    g_param_spec_boxed (PROP_RULE_FIRST_SET,
                        "First set",
                        "Array of terminal symbols with which a string derived using the rule can begin.",
                        G_TYPE_PTR_ARRAY,
                        G_PARAM_READWRITE);

  obj_properties[PROP_CAN_DERIVE_EPSILON] =
    g_param_spec_boolean (PROP_RULE_CAN_DERIVE_EPSILON,
                          "Can derive epsilon",
                          "Describes whether or not epsilon can be derived using the rule.",
                          FALSE,
                          G_PARAM_READABLE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
rule_init (Rule *self)
{
  /* NOP */
}

static void
rule_discern_epsilon_derivability (Rule *self)
{
  g_return_if_fail (SYNTACTIC_ANALYSIS_IS_RULE (self));

  RulePrivate *priv = rule_get_instance_private (self);
  GPtrArray *first_set = priv->first_set;

  if (g_ptr_array_has_items (first_set))
    {
      for (guint i = 0; i < first_set->len; ++i)
        {
          Symbol *symbol = g_ptr_array_index (first_set, i);

          if (symbol_is_match (symbol, EPSILON))
            {
              priv->derives_epsilon = TRUE;

              break;
            }
        }
    }
}

static void
rule_get_property (GObject    *object,
                   guint       property_id,
                   GValue     *value,
                   GParamSpec *pspec)
{
  RulePrivate *priv = rule_get_instance_private (SYNTACTIC_ANALYSIS_RULE (object));

  switch (property_id)
    {
    case PROP_SYMBOLS:
      g_value_set_boxed (value, priv->symbols);
      break;

    case PROP_FIRST_SET:
      g_value_set_boxed (value, priv->first_set);
      break;

    case PROP_CAN_DERIVE_EPSILON:
      g_value_set_boolean (value, priv->derives_epsilon);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
rule_set_property (GObject      *object,
                   guint         property_id,
                   const GValue *value,
                   GParamSpec   *pspec)
{
  RulePrivate *priv = rule_get_instance_private (SYNTACTIC_ANALYSIS_RULE (object));

  switch (property_id)
    {
    case PROP_SYMBOLS:
      if (priv->symbols != NULL)
        g_ptr_array_unref (priv->symbols);

      priv->symbols = g_value_dup_boxed (value);
      break;

    case PROP_FIRST_SET:
      if (priv->first_set != NULL)
        g_ptr_array_unref (priv->first_set);

      priv->first_set = g_value_dup_boxed (value);

      rule_discern_epsilon_derivability (SYNTACTIC_ANALYSIS_RULE (object));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
rule_dispose (GObject *object)
{
  RulePrivate *priv = rule_get_instance_private (SYNTACTIC_ANALYSIS_RULE (object));

  if (priv->symbols != NULL)
    g_clear_pointer (&priv->symbols, g_ptr_array_unref);

  if (priv->first_set != NULL)
    g_clear_pointer (&priv->first_set, g_ptr_array_unref);

  G_OBJECT_CLASS (rule_parent_class)->dispose (object);
}
