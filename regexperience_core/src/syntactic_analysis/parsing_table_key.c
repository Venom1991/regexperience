#include "internal/syntactic_analysis/parsing_table_key.h"
#include "internal/syntactic_analysis/production.h"
#include "internal/syntactic_analysis/symbols/symbol.h"

struct _ParsingTableKey
{
    GObject parent_instance;
};

typedef struct
{
    Production *production;
    Symbol     *symbol;
} ParsingTableKeyPrivate;

enum
{
    PROP_PRODUCTION = 1,
    PROP_SYMBOL,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void parsing_table_key_set_property (GObject      *object,
                                            guint         property_id,
                                            const GValue *value,
                                            GParamSpec   *pspec);

static void parsing_table_key_dispose      (GObject      *object);

G_DEFINE_TYPE_WITH_PRIVATE (ParsingTableKey, parsing_table_key, G_TYPE_OBJECT)

static void
parsing_table_key_class_init (ParsingTableKeyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = parsing_table_key_set_property;
  object_class->dispose = parsing_table_key_dispose;

  obj_properties[PROP_PRODUCTION] =
      g_param_spec_object (PROP_PARSING_TABLE_KEY_PRODUCTION,
                           "Production",
                           "Production representing the first dimension of any given parsing table entry.",
                           SYNTACTIC_ANALYSIS_TYPE_PRODUCTION,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_SYMBOL] =
      g_param_spec_object (PROP_PARSING_TABLE_KEY_SYMBOL,
                           "Symbol",
                           "Symbol representing the second dimension of any given parsing table entry.",
                           SYMBOLS_TYPE_SYMBOL,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
parsing_table_key_init (ParsingTableKey *self)
{
  /* NOP */
}

guint
parsing_table_key_hash (gconstpointer key)
{
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_PARSING_TABLE_KEY (key), 0);



  return 0;
}

gboolean
parsing_table_key_is_equal (gconstpointer a,
                            gconstpointer b)
{
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_PARSING_TABLE_KEY (a), FALSE);
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_PARSING_TABLE_KEY (b), FALSE);



  return FALSE;
}

static void
parsing_table_key_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  ParsingTableKeyPrivate *priv = parsing_table_key_get_instance_private (SYNTACTIC_ANALYSIS_PARSING_TABLE_KEY (object));

  switch (property_id)
    {
    case PROP_PRODUCTION:
      if (priv->production != NULL)
        g_object_unref (priv->production);

      priv->production = g_value_dup_object (value);
      break;

    case PROP_SYMBOL:
      if (priv->symbol != NULL)
        g_object_unref (priv->symbol);

      priv->symbol = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
parsing_table_key_dispose (GObject *object)
{
  ParsingTableKeyPrivate *priv = parsing_table_key_get_instance_private (SYNTACTIC_ANALYSIS_PARSING_TABLE_KEY (object));

  if (priv->production != NULL)
    g_clear_object (&priv->production);

  if (priv->symbol != NULL)
    g_clear_object (&priv->symbol);

  G_OBJECT_CLASS (parsing_table_key_parent_class)->dispose (object);
}
