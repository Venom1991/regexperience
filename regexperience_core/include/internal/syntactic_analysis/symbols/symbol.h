#ifndef REGEXPERIENCE_CORE_SYMBOL_H
#define REGEXPERIENCE_CORE_SYMBOL_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SYMBOLS_TYPE_SYMBOL (symbol_get_type ())

G_DECLARE_DERIVABLE_TYPE (Symbol, symbol, SYMBOLS, SYMBOL, GObject)

struct _SymbolClass
{
    GObjectClass parent_class;

    void (*extract_value) (Symbol *self, GValue *value);
    gboolean (*is_match) (Symbol *self, const gchar *value);

    gpointer padding[8];
};

void symbol_extract_value (Symbol *self,
                           GValue *value);

gboolean symbol_is_match (Symbol      *self,
                          const gchar *value);

#define PROP_SYMBOL_VALUE "value"

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_SYMBOL_H */
