#include "internal/syntactic_analysis/occurrence.h"

struct _Occurrence
{
  DerivationItem parent_instance;
};

typedef struct
{
  guint position;
} OccurrencePrivate;

enum
{
  PROP_POSITION = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void occurrence_get_property (GObject      *object,
                                     guint         property_id,
                                     GValue       *value,
                                     GParamSpec   *pspec);

static void occurrence_set_property (GObject      *object,
                                     guint         property_id,
                                     const GValue *value,
                                     GParamSpec   *pspec);

G_DEFINE_TYPE_WITH_PRIVATE (Occurrence, occurrence, SYNTACTIC_ANALYSIS_TYPE_DERIVATION_ITEM)

static void
occurrence_class_init (OccurrenceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = occurrence_get_property;
  object_class->set_property = occurrence_set_property;

  obj_properties[PROP_POSITION] =
    g_param_spec_uint (PROP_OCCURRENCE_POSITION,
                       "Position",
                       "Exact position in which the production is appearing as"
                         "a non-terminal symbol's underlying value.",
                       0,
                       G_MAXUINT32,
                       0,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
occurrence_init (Occurrence *self)
{
  /* NOP */
}

static void
occurrence_get_property (GObject    *object,
                         guint       property_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  OccurrencePrivate *priv = occurrence_get_instance_private (SYNTACTIC_ANALYSIS_OCCURRENCE (object));

  switch (property_id)
    {
    case PROP_POSITION:
      g_value_set_uint (value, priv->position);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
occurrence_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  OccurrencePrivate *priv = occurrence_get_instance_private (SYNTACTIC_ANALYSIS_OCCURRENCE (object));

  switch (property_id)
    {
    case PROP_POSITION:
      priv->position = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}
