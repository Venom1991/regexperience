#include <internal/syntactic_analysis/symbols/terminal.h>
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
    Symbol     *terminal;
    guint       hash;
} ParsingTableKeyPrivate;

enum
{
    PROP_PRODUCTION = 1,
    PROP_TERMINAL,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void parsing_table_key_constructed  (GObject      *object);

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

  object_class->constructed = parsing_table_key_constructed;
  object_class->set_property = parsing_table_key_set_property;
  object_class->dispose = parsing_table_key_dispose;

  obj_properties[PROP_PRODUCTION] =
    g_param_spec_object (PROP_PARSING_TABLE_KEY_PRODUCTION,
                         "Production",
                         "Production representing the first dimension of any given parsing table entry.",
                         SYNTACTIC_ANALYSIS_TYPE_PRODUCTION,
                         G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);

  obj_properties[PROP_TERMINAL] =
    g_param_spec_object (PROP_PARSING_TABLE_KEY_TERMINAL,
                         "Terminal",
                         "Terminal symbol representing the second dimension of any given parsing table entry.",
                         SYMBOLS_TYPE_SYMBOL,
                         G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);

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
parsing_table_key_hash (ParsingTableKey *self)
{
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_PARSING_TABLE_KEY (self), 0);

  ParsingTableKey *parsing_table_key = SYNTACTIC_ANALYSIS_PARSING_TABLE_KEY (self);
  ParsingTableKeyPrivate *priv = parsing_table_key_get_instance_private (parsing_table_key);

  return priv->hash;
}

gboolean
parsing_table_key_is_equal (ParsingTableKey *a,
                            ParsingTableKey *b)
{
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_PARSING_TABLE_KEY (a), FALSE);
  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_PARSING_TABLE_KEY (b), FALSE);

  ParsingTableKeyPrivate *a_key_priv =
    parsing_table_key_get_instance_private (SYNTACTIC_ANALYSIS_PARSING_TABLE_KEY (a));
  ParsingTableKeyPrivate *b_key_priv =
    parsing_table_key_get_instance_private (SYNTACTIC_ANALYSIS_PARSING_TABLE_KEY (b));

  Production *a_production = a_key_priv->production;
  Production *b_production = b_key_priv->production;
  Symbol *a_terminal = a_key_priv->terminal;
  Symbol *b_terminal = b_key_priv->terminal;

  return g_direct_equal (a_production, b_production)
      && g_direct_equal (a_terminal, b_terminal);
}

static void
parsing_table_key_constructed (GObject *object)
{
  ParsingTableKeyPrivate *priv = parsing_table_key_get_instance_private (SYNTACTIC_ANALYSIS_PARSING_TABLE_KEY (object));
  Production *production = priv->production;
  Symbol *terminal = priv->terminal;
  g_autofree gchar *production_caption = NULL;
  GValue value = G_VALUE_INIT;
  const g_autofree gchar *terminal_value = NULL;

  g_object_get (production,
                PROP_PRODUCTION_CAPTION, &production_caption,
                NULL);

  symbol_extract_value (terminal, &value);

  terminal_value = g_value_get_string (&value);

  guint hash = 0;
  guint production_caption_hash = g_str_hash (production_caption);
  guint terminal_value_hash = g_str_hash (terminal_value);

  hash = (hash * 397) ^ production_caption_hash;
  hash = (hash * 397) ^ terminal_value_hash;

  priv->hash = hash;

  G_OBJECT_CLASS (parsing_table_key_parent_class)->constructed (object);
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

    case PROP_TERMINAL:
      if (priv->terminal != NULL)
        g_object_unref (priv->terminal);

      priv->terminal = g_value_dup_object (value);
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

  if (priv->terminal != NULL)
    g_clear_object (&priv->terminal);

  G_OBJECT_CLASS (parsing_table_key_parent_class)->dispose (object);
}
