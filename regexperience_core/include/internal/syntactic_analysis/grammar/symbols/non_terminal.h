#ifndef REGEXPERIENCE_CORE_NON_TERMINAL_H
#define REGEXPERIENCE_CORE_NON_TERMINAL_H

#include <glib-object.h>

#include "symbol.h"

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_NON_TERMINAL (non_terminal_get_type ())
#define non_terminal_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_NON_TERMINAL, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (NonTerminal, non_terminal, SYNTACTIC_ANALYSIS, NON_TERMINAL, Symbol)

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_NON_TERMINAL_H */
