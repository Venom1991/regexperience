#ifndef REGEXPERIENCE_CORE_TERMINAL_H
#define REGEXPERIENCE_CORE_TERMINAL_H

#include <glib-object.h>

#include "symbol.h"

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_TERMINAL (terminal_get_type ())
#define terminal_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_TERMINAL, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Terminal, terminal, SYNTACTIC_ANALYSIS, TERMINAL, Symbol)

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_TERMINAL_H */
