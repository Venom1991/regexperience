#ifndef REGEXPERIENCE_DERIVATION_ITEM_H
#define REGEXPERIENCE_DERIVATION_ITEM_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SYNTACTIC_ANALYSIS_TYPE_DERIVATION_ITEM (derivation_item_get_type ())
#define derivation_item_new(...) (g_object_new (SYNTACTIC_ANALYSIS_TYPE_DERIVATION_ITEM, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (DerivationItem, derivation_item, SYNTACTIC_ANALYSIS, DERIVATION_ITEM, GObject)

#define PROP_DERIVATION_ITEM_LEFT_HAND_SIDE  "left-hand-side"
#define PROP_DERIVATION_ITEM_RIGHT_HAND_SIDE "right-hand-side"

G_END_DECLS

#endif /* REGEXPERIENCE_DERIVATION_ITEM_H */
