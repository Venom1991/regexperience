#ifndef REGEXPERIENCE_EMPTY_H
#define REGEXPERIENCE_EMPTY_H

#include "constant.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define AST_NODES_TYPE_EMPTY (empty_get_type ())
#define empty_new(...) (g_object_new (AST_NODES_TYPE_EMPTY, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Empty, empty, AST_NODES, EMPTY, Constant)

G_END_DECLS

#endif /* REGEXPERIENCE_EMPTY_H */
