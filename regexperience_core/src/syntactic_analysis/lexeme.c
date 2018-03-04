#include "internal/syntactic_analysis/lexeme.h"

struct _Lexeme
{
    GObject parent_instance;
};

typedef struct
{
    GString *content;
    guint    start_position;
    guint    end_position;
} LexemePrivate;

enum
{
    PROP_CONTENT = 1,
    PROP_START_POSITION,
    PROP_END_POSITION,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void lexeme_get_property (GObject      *object,
                                 guint         property_id,
                                 GValue       *value,
                                 GParamSpec   *pspec);

static void lexeme_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec);

static void lexeme_finalize     (GObject      *object);

G_DEFINE_TYPE_WITH_PRIVATE (Lexeme, lexeme, G_TYPE_OBJECT)

static void
lexeme_class_init (LexemeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = lexeme_get_property;
  object_class->set_property = lexeme_set_property;
  object_class->finalize = lexeme_finalize;

  obj_properties[PROP_CONTENT] =
    g_param_spec_boxed (PROP_LEXEME_CONTENT,
                        "Content",
                        "Textual content of the lexeme.",
                        G_TYPE_GSTRING,
                        G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_START_POSITION] =
    g_param_spec_uint (PROP_LEXEME_START_POSITION,
                       "Start Position",
                       "Start of the position in which the lexeme appears in the input.",
                       0,
                       G_MAXUINT32,
                       0,
                       G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_END_POSITION] =
    g_param_spec_uint (PROP_LEXEME_END_POSITION,
                       "End Position",
                       "End of the position in which the lexeme appears in the input.",
                       0,
                       G_MAXUINT32,
                       0,
                       G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
lexeme_init (Lexeme *self)
{
  /* NOP */
}

static void
lexeme_get_property (GObject    *object,
                     guint       property_id,
                     GValue     *value,
                     GParamSpec *pspec)
{
  LexemePrivate *priv = lexeme_get_instance_private (SYNTACTIC_ANALYSIS_LEXEME (object));

  switch (property_id)
    {
    case PROP_CONTENT:
      g_value_set_boxed (value, priv->content);
      break;

    case PROP_START_POSITION:
      g_value_set_uint (value, priv->start_position);
      break;

    case PROP_END_POSITION:
      g_value_set_uint (value, priv->end_position);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
lexeme_set_property (GObject      *object,
                     guint         property_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
  LexemePrivate *priv = lexeme_get_instance_private (SYNTACTIC_ANALYSIS_LEXEME (object));

  switch (property_id)
    {
    case PROP_CONTENT:
      if (priv->content != NULL)
        g_string_free (priv->content, TRUE);

      priv->content = g_value_dup_boxed (value);
      break;

    case PROP_START_POSITION:
      priv->start_position = g_value_get_uint (value);
      break;

    case PROP_END_POSITION:
      priv->end_position = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
lexeme_finalize (GObject *object)
{
  LexemePrivate *priv = lexeme_get_instance_private (SYNTACTIC_ANALYSIS_LEXEME (object));

  if (priv->content != NULL)
    g_string_free (priv->content, TRUE);

  G_OBJECT_CLASS (lexeme_parent_class)->finalize (object);
}
