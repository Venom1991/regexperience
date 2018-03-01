#ifndef REGEXPERIENCE_CORE_TRANSITION_H
#define REGEXPERIENCE_CORE_TRANSITION_H

#include <glib-object.h>

G_BEGIN_DECLS

#define TRANSITIONS_TYPE_TRANSITION (transition_get_type())

G_DECLARE_DERIVABLE_TYPE (Transition, transition, TRANSITIONS, TRANSITION, GObject)

struct _TransitionClass
{
    GObjectClass parent_class;
};

gboolean transition_is_possible (Transition *self,
                                 gchar       input_character);

gboolean transition_is_epsilon (Transition *self);

#define PROP_TRANSITION_EXPECTED_CHARACTER      "expected-character"
#define PROP_TRANSITION_REQUIRES_INPUT          "requires-input"
#define PROP_TRANSITION_EQUALITY_CONDITION_TYPE "equality-condition-type"

typedef enum _EqualityConditionType
{
    EQUALITY_CONDITION_TYPE_UNDEFINED,
    EQUALITY_CONDITION_TYPE_ANY,
    EQUALITY_CONDITION_TYPE_EQUAL,
    EQUALITY_CONDITION_TYPE_NOT_EQUAL
} EqualityConditionType;

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_TRANSITION_H */
