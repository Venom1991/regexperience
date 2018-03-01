
#include "internal/state_machines/transducers/mealy.h"
#include "internal/state_machines/transducers/transducer_runnable.h"
#include "internal/state_machines/fsm_initializable.h"
#include "internal/state_machines/transitions/mealy_transition.h"
#include "internal/common/helpers.h"

struct _Mealy
{
    Fsm parent_instance;
};

typedef struct
{
    State *current_state;
} MealyPrivate;

static void mealy_transducer_runnable_interface_init (TransducerRunnableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (Mealy, mealy, STATE_MACHINES_TYPE_FSM,
                         G_ADD_PRIVATE (Mealy)
                         G_IMPLEMENT_INTERFACE (TRANSDUCERS_TYPE_TRANSDUCER_RUNNABLE,
                                                mealy_transducer_runnable_interface_init))

static void mealy_reset (TransducerRunnable *self);

static gpointer mealy_run (TransducerRunnable *self, gchar input);

static void mealy_dispose (GObject *object);

static void
mealy_class_init (MealyClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = mealy_dispose;
}

static void
mealy_init (Mealy *self)
{
  /* NOP */
}

static void
mealy_transducer_runnable_interface_init (TransducerRunnableInterface *iface)
{
  iface->reset = mealy_reset;
  iface->run = mealy_run;
}

static void
mealy_reset (TransducerRunnable *self)
{
  MealyPrivate *priv = mealy_get_instance_private (TRANSDUCERS_MEALY (self));

  if (priv->current_state != NULL)
    g_object_unref (priv->current_state);

  State *start_state = NULL;

  g_object_get (self,
                PROP_FSM_INITIALIZABLE_START_STATE, &start_state,
                NULL);

  priv->current_state = start_state;
}

static gpointer
mealy_run (TransducerRunnable *self, gchar input)
{
  MealyPrivate *priv = mealy_get_instance_private (TRANSDUCERS_MEALY (self));

  State *current_state = priv->current_state;

  g_return_val_if_fail (current_state != NULL, NULL);

  g_autoptr (GPtrArray) transitions = NULL;

  g_object_get (current_state,
                PROP_STATE_TRANSITIONS, &transitions,
                NULL);

  g_return_val_if_fail (g_ptr_array_has_items (transitions), NULL);

  for (guint i = 0; i < transitions->len; ++i)
    {
      Transition *transition = g_ptr_array_index (transitions, i);

      g_return_val_if_fail (TRANSITIONS_IS_MEALY_TRANSITION (transition), NULL);

      if (transition_is_possible (transition, input))
        {
          State *next_state = NULL;
          gpointer *output_data = NULL;

          g_object_unref (current_state);
          g_object_get (transition,
                        PROP_DETERMINISTIC_TRANSITION_OUTPUT_STATE, &next_state,
                        PROP_MEALY_TRANSITION_OUTPUT_DATA, &output_data,
                        NULL);

          priv->current_state = next_state;

          return output_data;
        }
    }

  return NULL;
}

static void
mealy_dispose (GObject *object)
{
  MealyPrivate *priv = mealy_get_instance_private (TRANSDUCERS_MEALY (object));

  if (priv->current_state != NULL)
    g_clear_object (&priv->current_state);

  G_OBJECT_CLASS (mealy_parent_class)->dispose (object);
}
