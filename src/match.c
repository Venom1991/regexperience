#include "core/match.h"

struct _Match
{
  GObject parent_instance;
};

typedef struct
{
  GString *value;
  guint    range_begin;
  guint    range_end;
} MatchPrivate;

enum
{
  PROP_VALUE = 1,
  PROP_RANGE_BEGIN,
  PROP_RANGE_END,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void match_get_property (GObject      *object,
                                guint         property_id,
                                GValue       *value,
                                GParamSpec   *pspec);

static void match_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec);

static void match_finalize     (GObject      *object);

G_DEFINE_TYPE_WITH_PRIVATE (Match, match, G_TYPE_OBJECT)

static void
match_class_init (MatchClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = match_get_property;
  object_class->set_property = match_set_property;
  object_class->finalize = match_finalize;

  obj_properties[PROP_VALUE] =
    g_param_spec_boxed (PROP_MATCH_VALUE,
                        "Value",
                        "Textual content of the match (may be an empty string).",
                        G_TYPE_GSTRING,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  obj_properties[PROP_RANGE_BEGIN] =
    g_param_spec_uint (PROP_MATCH_RANGE_BEGIN,
                       "Range begin",
                       "The beginning position of the match's range relative to the rest of the input being"
                         "searched (may be zero).",
                       0,
                       G_MAXUINT32,
                       0,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  obj_properties[PROP_RANGE_END] =
    g_param_spec_uint (PROP_MATCH_RANGE_END,
                       "Range end",
                       "The ending position of the match's range relative to the rest of the input being"
                         "searched (may be zero).",
                       0,
                       G_MAXUINT32,
                       0,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
match_init (Match *self)
{
  /* NOP */
}

static void
match_get_property (GObject    *object,
                    guint       property_id,
                    GValue     *value,
                    GParamSpec *pspec)
{
  MatchPrivate *priv = match_get_instance_private (CORE_MATCH (object));

  switch (property_id)
    {
    case PROP_VALUE:
      g_value_set_boxed (value, priv->value);
      break;

    case PROP_RANGE_BEGIN:
      g_value_set_uint (value, priv->range_begin);
      break;

    case PROP_RANGE_END:
      g_value_set_uint (value, priv->range_end);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
match_set_property (GObject      *object,
                    guint         property_id,
                    const GValue *value,
                    GParamSpec   *pspec)
{
  MatchPrivate *priv = match_get_instance_private (CORE_MATCH (object));

  switch (property_id)
    {
    case PROP_VALUE:
      if (priv->value != NULL)
        g_string_free (priv->value, TRUE);

      priv->value = g_value_dup_boxed (value);
      break;

    case PROP_RANGE_BEGIN:
      priv->range_begin = g_value_get_uint (value);
      break;

    case PROP_RANGE_END:
      priv->range_end = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
match_finalize (GObject *object)
{
  MatchPrivate *priv = match_get_instance_private (CORE_MATCH (object));

  if (priv->value != NULL)
    g_string_free (priv->value, TRUE);

  G_OBJECT_CLASS (match_parent_class)->finalize (object);
}
