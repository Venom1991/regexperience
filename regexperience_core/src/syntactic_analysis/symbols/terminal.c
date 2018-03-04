#include "internal/syntactic_analysis/symbols/terminal.h"
#include "internal/common/macros.h"
#include "internal/common/helpers.h"

struct _Terminal
{
    Symbol parent_instance;
};

typedef struct
{
    gchar     *concatenated_value;
    GPtrArray *split_values;
} TerminalPrivate;

enum
{
    PROP_VALUE = 1,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void     terminal_extract_value (Symbol       *self,
                                        GValue       *value);

static gboolean terminal_is_match      (Symbol       *self,
                                        const gchar  *value);

static void     terminal_constructed   (GObject      *object);

static void     terminal_set_property  (GObject      *object,
                                        guint         property_id,
                                        const GValue *value,
                                        GParamSpec   *pspec);

static void     terminal_finalize      (GObject      *object);

G_DEFINE_TYPE_WITH_PRIVATE (Terminal, terminal, SYMBOLS_TYPE_SYMBOL)

static void
terminal_class_init (TerminalClass *klass)
{
  SymbolClass *symbol_class = SYMBOLS_SYMBOL_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  symbol_class->extract_value = terminal_extract_value;
  symbol_class->is_match = terminal_is_match;

  object_class->constructed = terminal_constructed;
  object_class->set_property = terminal_set_property;
  object_class->finalize = terminal_finalize;

  obj_properties[PROP_VALUE] =
    g_param_spec_string (PROP_SYMBOL_VALUE,
                         "Value",
                         "Textual content of the terminal symbol, can be a delimited string.",
                         NULL,
                         G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
terminal_init (Terminal *self)
{
  TerminalPrivate *priv = terminal_get_instance_private (self);

  priv->split_values = g_ptr_array_new_with_free_func (g_free);
}

static void
terminal_extract_value (Symbol *self,
                        GValue *value)
{
  g_return_if_fail (SYMBOLS_IS_TERMINAL (self));
  g_return_if_fail (value != NULL);

  TerminalPrivate *priv = terminal_get_instance_private (SYMBOLS_TERMINAL (self));

  g_value_init (value, G_TYPE_STRING);
  g_value_set_string (value, priv->concatenated_value);
}

static gboolean
terminal_is_match (Symbol      *self,
                   const gchar *value)
{
  g_return_val_if_fail (SYMBOLS_IS_TERMINAL (self), FALSE);
  g_return_val_if_fail (value != NULL, FALSE);

  TerminalPrivate *priv = terminal_get_instance_private (SYMBOLS_TERMINAL (self));

  GPtrArray *split_values = priv->split_values;
  GCompareFunc terminal_compare_func = g_compare_strings;
  gpointer split_value = g_ptr_array_bsearch (split_values,
                                              terminal_compare_func,
                                              &value);

  return split_value != NULL;
}

static void
terminal_constructed (GObject *object)
{
  TerminalPrivate *priv = terminal_get_instance_private (SYMBOLS_TERMINAL (object));

  gchar *concatenated_value = priv->concatenated_value;
  GPtrArray *split_values = priv->split_values;

  GCompareFunc terminal_compare_func = g_compare_strings;
  g_auto (GStrv) split_values_as_vector = g_strsplit (concatenated_value, DELIMITER, -1);
  guint split_values_length = g_strv_length (split_values_as_vector);

  for (guint i = 0; i < split_values_length; ++i)
    {
      gchar *current_split_value = g_strdup (split_values_as_vector[i]);

      g_ptr_array_add (split_values, current_split_value);
    }

  g_ptr_array_sort (split_values, terminal_compare_func);

  G_OBJECT_CLASS (terminal_parent_class)->constructed (object);
}

static void
terminal_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  TerminalPrivate *priv = terminal_get_instance_private (SYMBOLS_TERMINAL (object));

  switch (property_id)
    {
    case PROP_VALUE:
      if (priv->concatenated_value != NULL)
        g_free (priv->concatenated_value);

      priv->concatenated_value = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
terminal_finalize (GObject *object)
{
  TerminalPrivate *priv = terminal_get_instance_private (SYMBOLS_TERMINAL (object));

  if (priv->concatenated_value != NULL)
    g_free (priv->concatenated_value);

  if (priv->split_values != NULL)
    g_ptr_array_unref (priv->split_values);

  G_OBJECT_CLASS (terminal_parent_class)->finalize (object);
}
