#ifndef REGEXPERIENCE_TRANSITION_H
#define REGEXPERIENCE_TRANSITION_H

#include <glib-object.h>

G_BEGIN_DECLS

#define TRANSITIONS_TYPE_TRANSITION (transition_get_type ())

G_DECLARE_DERIVABLE_TYPE (Transition, transition, TRANSITIONS, TRANSITION, GObject)

typedef enum
{
  EQUALITY_CONDITION_TYPE_UNDEFINED,
  EQUALITY_CONDITION_TYPE_ANY,
  EQUALITY_CONDITION_TYPE_EQUAL,
  EQUALITY_CONDITION_TYPE_NOT_EQUAL
} EqualityConditionType;

struct _TransitionClass
{
  GObjectClass parent_class;

  void (*supplement_states_array_with_output) (Transition *self,
                                               GPtrArray  *states_array);

  gpointer padding[8];
};

void     transition_supplement_states_array_with_output (Transition *self,
                                                         GPtrArray  *states_array);

gboolean transition_is_possible                         (Transition *self,
                                                         gchar       input_character);

gboolean transition_is_allowed                          (Transition *self,
                                                         gchar       input_character);

gboolean transition_is_epsilon                          (Transition *self);

void     transition_convert_to_epsilon                  (Transition *self);

gint     transition_compare_equality_condition_type     (Transition *a,
                                                         Transition *b);

#define PROP_TRANSITION_EXPECTED_CHARACTER      "expected-character"
#define PROP_TRANSITION_REQUIRES_INPUT          "requires-input"
#define PROP_TRANSITION_EQUALITY_CONDITION_TYPE "equality-condition-type"

#define EPSILON                                 0x00 /* ASCII "NUL" */
#define START                                   0x02 /* ASCII "STX" */
#define END                                     0x03 /* ASCII "ETX" */
#define ANY                                     0x1A /* ASCII "SUB" */
#define EMPTY                                   0x1B /* ASCII "ESC" */

G_END_DECLS

#endif /* REGEXPERIENCE_TRANSITION_H */
