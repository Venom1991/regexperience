#ifndef REGEXPERIENCE_TERMINAL_H
#define REGEXPERIENCE_TERMINAL_H

#include "symbol.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define SYMBOLS_TYPE_TERMINAL (terminal_get_type ())
#define terminal_new(...) (g_object_new (SYMBOLS_TYPE_TERMINAL, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Terminal, terminal, SYMBOLS, TERMINAL, Symbol)

G_END_DECLS

#endif /* REGEXPERIENCE_TERMINAL_H */
