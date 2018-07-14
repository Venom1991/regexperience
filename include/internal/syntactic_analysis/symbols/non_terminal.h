#ifndef REGEXPERIENCE_NON_TERMINAL_H
#define REGEXPERIENCE_NON_TERMINAL_H

#include "symbol.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define SYMBOLS_TYPE_NON_TERMINAL (non_terminal_get_type ())
#define non_terminal_new(...) (g_object_new (SYMBOLS_TYPE_NON_TERMINAL, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (NonTerminal, non_terminal, SYMBOLS, NON_TERMINAL, Symbol)

G_END_DECLS

#endif /* REGEXPERIENCE_NON_TERMINAL_H */
