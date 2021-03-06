#include "internal/syntactic_analysis/symbols/symbol.h"
#include "internal/syntactic_analysis/grammar.h"

static gboolean symbol_default_is_match (Symbol          *self,
                                         gconstpointer    value,
                                         SymbolValueType  value_type);

static gboolean symbol_default_is_equal (Symbol          *self,
                                         Symbol          *other);

G_DEFINE_ABSTRACT_TYPE (Symbol, symbol, G_TYPE_OBJECT)

static void
symbol_class_init (SymbolClass *klass)
{
  klass->extract_value = NULL;
  klass->is_match = symbol_default_is_match;
  klass->is_equal = symbol_default_is_equal;
}

static void
symbol_init (Symbol *self)
{
  /* NOP */
}

static gboolean
symbol_default_is_match (Symbol          *self,
                         gconstpointer    value,
                         SymbolValueType  value_type)
{
  return FALSE;
}

static gboolean
symbol_default_is_equal (Symbol *self,
                         Symbol *other)
{
  return FALSE;
}

void
symbol_extract_value (Symbol *self,
                      GValue *value)
{
  g_return_if_fail (SYMBOLS_IS_SYMBOL (self));

  SymbolClass *klass = SYMBOLS_SYMBOL_GET_CLASS (self);

  g_return_if_fail (klass->extract_value != NULL);

  klass->extract_value (self, value);
}

gboolean
symbol_is_equal (Symbol *self,
                 Symbol *other)
{
  g_return_val_if_fail (SYMBOLS_IS_SYMBOL (self), FALSE);

  SymbolClass *klass = SYMBOLS_SYMBOL_GET_CLASS (self);

  g_assert (klass->is_equal != NULL);

  return klass->is_equal (self, other);
}

gboolean
symbol_is_epsilon (Symbol *self)
{
  g_return_val_if_fail (SYMBOLS_IS_SYMBOL (self), FALSE);

  return symbol_is_match (self, EPSILON);
}

gboolean
symbol_is_string_match (Symbol *self,
                        gchar  *value)
{
  g_return_val_if_fail (SYMBOLS_IS_SYMBOL (self), FALSE);

  SymbolClass *klass = SYMBOLS_SYMBOL_GET_CLASS (self);

  g_assert (klass->is_match != NULL);

  return klass->is_match (self,
                          (gconstpointer) value,
                          SYMBOL_VALUE_TYPE_POINTER_TO_GCHAR);
}

gboolean
symbol_is_production_match (Symbol     *self,
                            Production *value)
{
  g_return_val_if_fail (SYMBOLS_IS_SYMBOL (self), FALSE);

  SymbolClass *klass = SYMBOLS_SYMBOL_GET_CLASS (self);

  g_assert (klass->is_match != NULL);

  return klass->is_match (self,
                          (gconstpointer) value,
                          SYMBOL_VALUE_TYPE_POINTER_TO_PRODUCTION);
}
