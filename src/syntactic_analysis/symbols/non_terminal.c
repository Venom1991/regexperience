#include "internal/syntactic_analysis/symbols/non_terminal.h"
#include "internal/syntactic_analysis/production.h"

struct _NonTerminal
{
  Symbol parent_instance;
};

typedef struct
{
  GWeakRef value;
} NonTerminalPrivate;

enum
{
  PROP_VALUE = 1,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void     non_terminal_extract_value (Symbol          *self,
                                            GValue          *value);

static gboolean non_terminal_is_match      (Symbol          *self,
                                            gconstpointer    value,
                                            SymbolValueType  value_type);

static gboolean non_terminal_is_equal      (Symbol          *self,
                                            Symbol          *other);

static void     non_terminal_set_property  (GObject         *object,
                                            guint            property_id,
                                            const GValue    *value,
                                            GParamSpec      *pspec);

static void     non_terminal_dispose       (GObject         *object);

G_DEFINE_TYPE_WITH_PRIVATE (NonTerminal, non_terminal, SYMBOLS_TYPE_SYMBOL)

static void
non_terminal_class_init (NonTerminalClass *klass)
{
  SymbolClass *symbol_class = SYMBOLS_SYMBOL_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  symbol_class->extract_value = non_terminal_extract_value;
  symbol_class->is_match = non_terminal_is_match;
  symbol_class->is_equal = non_terminal_is_equal;

  object_class->set_property = non_terminal_set_property;
  object_class->dispose = non_terminal_dispose;

  obj_properties[PROP_VALUE] =
    g_param_spec_object (PROP_SYMBOL_VALUE,
                         "Value",
                         "Production whose rules can be used to rewrite the non-terminal symbol.",
                         SYNTACTIC_ANALYSIS_TYPE_PRODUCTION,
                         G_PARAM_WRITABLE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
non_terminal_init (NonTerminal *self)
{
  NonTerminalPrivate *priv = non_terminal_get_instance_private (self);

  g_weak_ref_init (&priv->value, NULL);
}

static void
non_terminal_extract_value (Symbol *self,
                            GValue *value)
{
  g_return_if_fail (SYMBOLS_IS_NON_TERMINAL (self));
  g_return_if_fail (value != NULL);

  NonTerminalPrivate *priv = non_terminal_get_instance_private (SYMBOLS_NON_TERMINAL (self));
  Production *production = g_weak_ref_get (&priv->value);

  g_value_init (value, SYNTACTIC_ANALYSIS_TYPE_PRODUCTION);
  g_value_take_object (value, production);
}

static gboolean
non_terminal_is_match (Symbol          *self,
                       gconstpointer    value,
                       SymbolValueType  value_type)
{
  g_return_val_if_fail (SYMBOLS_IS_NON_TERMINAL (self), FALSE);
  g_return_val_if_fail (value != NULL, FALSE);

  if (value_type == SYMBOL_VALUE_TYPE_POINTER_TO_PRODUCTION)
    {
      NonTerminalPrivate *priv = non_terminal_get_instance_private (SYMBOLS_NON_TERMINAL (self));
      const Production *value_as_production = (const Production *) value;
      g_autoptr (Production) self_value = g_weak_ref_get (&priv->value);

      return g_direct_equal (self_value, value_as_production);
    }

  return FALSE;
}

static gboolean
non_terminal_is_equal (Symbol *self,
                       Symbol *other)
{
  g_return_val_if_fail (SYMBOLS_IS_NON_TERMINAL (self), FALSE);

  if (SYMBOLS_IS_NON_TERMINAL (other))
    {
      NonTerminalPrivate *priv = non_terminal_get_instance_private (SYMBOLS_NON_TERMINAL (self));
      g_autoptr (Production) self_value = g_weak_ref_get (&priv->value);
      g_autoptr (Production) other_value = NULL;
      GValue value = G_VALUE_INIT;

      symbol_extract_value (other, &value);

      other_value = g_value_get_object (&value);

      return g_direct_equal (self_value, other_value);
    }

  return FALSE;
}

static void
non_terminal_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  NonTerminalPrivate *priv = non_terminal_get_instance_private (SYMBOLS_NON_TERMINAL (object));

  switch (property_id)
    {
    case PROP_VALUE:
      {
        Production *production = g_value_get_object (value);

        g_weak_ref_set (&priv->value, production);
      }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
non_terminal_dispose (GObject *object)
{
  NonTerminalPrivate *priv = non_terminal_get_instance_private (SYMBOLS_NON_TERMINAL (object));

  g_weak_ref_clear (&priv->value);

  G_OBJECT_CLASS (non_terminal_parent_class)->dispose (object);
}
