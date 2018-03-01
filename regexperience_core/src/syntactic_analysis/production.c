#include "internal/syntactic_analysis/production.h"

struct _Production
{
    GObject parent_instance;
};

typedef struct
{
    gchar *caption;
    GPtrArray *rules;
} ProductionPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (Production, production, G_TYPE_OBJECT)

enum {
    PROP_CAPTION = 1,
    PROP_RULES,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void production_get_property (GObject *object,
                                     guint property_id,
                                     GValue *value,
                                     GParamSpec *pspec);

static void production_set_property (GObject *object,
                                     guint property_id,
                                     const GValue *value,
                                     GParamSpec *pspec);

static void production_dispose (GObject *object);

static void production_finalize (GObject *object);

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
                          "Array of rules that offer possible ways of rewriting the production"
                              "where it appears on the right-hand-side as a non-terminal symbol.",
                          G_TYPE_PTR_ARRAY,
                          G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
production_init (Production *self)
{
  /* NOP */
}

static void
production_get_property (GObject *object,
                         guint property_id,
                         GValue *value,
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
production_set_property (GObject *object,
                         guint property_id,
                         const GValue *value,
                         GParamSpec *pspec)
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
