#include "internal/syntactic_analysis/grammar/symbols/symbol.h"

G_DEFINE_ABSTRACT_TYPE (Symbol, symbol, G_TYPE_OBJECT)

static gboolean symbol_default_is_match (Symbol      *self,
                                         const gchar *value);

static void
symbol_class_init (SymbolClass *klass)
{
  klass->extract_value = NULL;
  klass->is_match = symbol_default_is_match;
}

static void
symbol_init (Symbol *self)
{
  /* NOP */
}

static gboolean
symbol_default_is_match (Symbol      *self,
                         const gchar *value)
{
  return FALSE;
}

void
symbol_extract_value (Symbol *self,
                      GValue *value)
{
  SymbolClass *klass;

  g_return_if_fail (SYNTACTIC_ANALYSIS_IS_SYMBOL (self));

  klass = SYNTACTIC_ANALYSIS_SYMBOL_GET_CLASS (self);

  g_return_if_fail (klass->extract_value != NULL);

  klass->extract_value (self, value);
}

gboolean
symbol_is_match (Symbol      *self,
                 const gchar *value)
{
  SymbolClass *klass;

  g_return_val_if_fail (SYNTACTIC_ANALYSIS_IS_SYMBOL (self), FALSE);

  klass = SYNTACTIC_ANALYSIS_SYMBOL_GET_CLASS (self);

  g_assert (klass->is_match != NULL);

  return klass->is_match (self, value);
}
