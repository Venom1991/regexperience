#include "internal/syntactic_analysis/rule.h"

struct _Rule
{
    GObject parent_instance;
};

typedef struct
{
    GPtrArray *symbols;
} RulePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (Rule, rule, G_TYPE_OBJECT)

enum
{
    PROP_SYMBOLS = 1,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void rule_get_property (GObject    *object,
                               guint       property_id,
                               GValue     *value,
                               GParamSpec *pspec);

static void rule_set_property (GObject      *object,
                               guint         property_id,
                               const GValue *value,
                               GParamSpec   *pspec);

static void rule_dispose (GObject *object);

static void
rule_class_init(RuleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = rule_get_property;
  object_class->set_property = rule_set_property;
  object_class->dispose = rule_dispose;

  obj_properties[PROP_SYMBOLS] =
      g_param_spec_boxed (PROP_RULE_SYMBOLS,
                          "Symbols",
                          "Array of terminal and non-terminal symbols.",
                          G_TYPE_PTR_ARRAY,
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

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
rule_get_property (GObject    *object,
                   guint       property_id,
                   GValue     *value,
                   GParamSpec *pspec)
{
  RulePrivate *priv = rule_get_instance_private (SYNTACTIC_ANALYSIS_RULE(object));

  switch (property_id)
    {
    case PROP_SYMBOLS:
      g_value_set_boxed (value, priv->symbols);
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
  RulePrivate *priv = rule_get_instance_private (SYNTACTIC_ANALYSIS_RULE(object));

  switch (property_id)
    {
    case PROP_SYMBOLS:
      if (priv->symbols != NULL)
        g_ptr_array_unref (priv->symbols);

      priv->symbols = g_value_dup_boxed (value);
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

  G_OBJECT_CLASS (rule_parent_class)->dispose (object);
}
