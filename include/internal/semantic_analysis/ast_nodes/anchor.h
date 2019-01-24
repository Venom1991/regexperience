#ifndef REGEXPERIENCE_ANCHOR_H
#define REGEXPERIENCE_ANCHOR_H

#include "unary_operator.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define AST_NODES_TYPE_ANCHOR (anchor_get_type ())
#define anchor_new(...) (g_object_new (AST_NODES_TYPE_ANCHOR, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Anchor, anchor, AST_NODES, ANCHOR, UnaryOperator)

typedef enum
{
  ANCHOR_TYPE_UNDEFINED,
  ANCHOR_TYPE_ANCHORED,
  ANCHOR_TYPE_UNANCHORED
} AnchorType;

#define PROP_ANCHOR_START_TYPE "start-type"
#define PROP_ANCHOR_END_TYPE   "end-type"

G_END_DECLS

#endif /* REGEXPERIENCE_ANCHOR_H */
