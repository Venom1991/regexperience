#ifndef REGEXPERIENCE_CORE_SYMBOL_H
#define REGEXPERIENCE_CORE_SYMBOL_H

#include "internal/syntactic_analysis/production.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define SYMBOLS_TYPE_SYMBOL (symbol_get_type ())

G_DECLARE_DERIVABLE_TYPE (Symbol, symbol, SYMBOLS, SYMBOL, GObject)

typedef enum
{
    SYMBOL_VALUE_TYPE_POINTER_TO_GCHAR,
    SYMBOL_VALUE_TYPE_POINTER_TO_PRODUCTION,
    SYMBOL_VALUE_TYPE_UNDEFINED
} SymbolValueType;

#define symbol_value_type(x) _Generic(((void) 0, x),     \
       gchar *: SYMBOL_VALUE_TYPE_POINTER_TO_GCHAR,      \
  Production *: SYMBOL_VALUE_TYPE_POINTER_TO_PRODUCTION, \
       default: SYMBOL_VALUE_TYPE_UNDEFINED)

struct _SymbolClass
{
    GObjectClass parent_class;

    void     (*extract_value) (Symbol          *self,
                               GValue          *value);
    gboolean (*is_match)      (Symbol          *self,
                               gconstpointer    value,
                               SymbolValueType  value_type);
    gboolean (*is_equal)      (Symbol          *self,
                               Symbol          *other);

    gpointer padding[8];
};

void     symbol_extract_value (Symbol          *self,
                               GValue          *value);

gboolean symbol_is_match      (Symbol          *self,
                               gconstpointer    value,
                               SymbolValueType  value_type);

gboolean symbol_is_equal      (Symbol          *self,
                               Symbol          *other);

#define PROP_SYMBOL_VALUE "value"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_SYMBOL_H */
