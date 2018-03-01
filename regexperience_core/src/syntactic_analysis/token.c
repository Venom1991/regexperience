#include "internal/syntactic_analysis/token.h"
#include "internal/syntactic_analysis/lexeme.h"

struct _Token
{
    GObject parent_instance;
};

typedef struct
{
    TokenCategory category;
    Lexeme *lexeme;
} TokenPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (Token, token, G_TYPE_OBJECT)

enum
{
    PROP_CATEGORY = 1,
    PROP_LEXEME,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void token_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec);

static void token_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec);

static void token_dispose (GObject *object);

static void
token_class_init (TokenClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = token_get_property;
  object_class->set_property = token_set_property;
  object_class->dispose = token_dispose;

  obj_properties[PROP_CATEGORY] =
      g_param_spec_uint (PROP_TOKEN_CATEGORY,
                         "Category",
                         "Category of the token.",
                         TOKEN_CATEGORY_UNDEFINED,
                         TOKEN_CATEGORY_N_CATEGORIES - 1,
                         TOKEN_CATEGORY_UNDEFINED,
                         G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_LEXEME] =
      g_param_spec_object (PROP_TOKEN_LEXEME,
                           "Lexeme",
                           "Lexeme associated with the token.",
                           SYNTACTIC_ANALYSIS_TYPE_LEXEME,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
token_init (Token *self)
{
  /* NOP */
}

static void
token_get_property (GObject    *object,
                    guint       property_id,
                    GValue     *value,
                    GParamSpec *pspec)
{
  TokenPrivate *priv = token_get_instance_private (SYNTACTIC_ANALYSIS_TOKEN (object));

  switch (property_id)
    {
    case PROP_CATEGORY:
      g_value_set_uint (value, priv->category);
      break;

    case PROP_LEXEME:
      g_value_set_object (value, priv->lexeme);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
token_set_property (GObject      *object,
                    guint         property_id,
                    const GValue *value,
                    GParamSpec   *pspec)
{
  TokenPrivate *priv = token_get_instance_private (SYNTACTIC_ANALYSIS_TOKEN (object));

  switch (property_id)
    {
    case PROP_CATEGORY:
      priv->category = (TokenCategory) g_value_get_uint (value);
      break;

    case PROP_LEXEME:
      if (priv->lexeme != NULL)
        g_object_unref (priv->lexeme);

      priv->lexeme = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
token_dispose (GObject *object)
{
  TokenPrivate *priv = token_get_instance_private (SYNTACTIC_ANALYSIS_TOKEN (object));

  if (priv->lexeme != NULL)
    g_clear_object (&priv->lexeme);

  G_OBJECT_CLASS (token_parent_class)->dispose (object);
}
