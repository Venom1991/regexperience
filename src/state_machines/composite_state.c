#include "internal/state_machines/composite_state.h"
#include "internal/common/helpers.h"

struct _CompositeState
{
  State parent_instance;
};

typedef struct
{
  GPtrArray                      *composed_from_states;
  CompositeStateResolveTypeFlags  resolve_type_flags;
} CompositeStatePrivate;

enum
{
  PROP_COMPOSED_FROM_STATES = 1,
  PROP_RESOLVE_TYPE_FLAGS,
  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static gboolean composite_state_is_composed_from (State                 *self,
                                                  const GPtrArray       *states);

static gboolean composite_state_can_be_marked_as (CompositeStatePrivate *priv,
                                                  StateTypeFlags         state_type_flags);

static void     composite_state_constructed      (GObject               *object);

static void     composite_state_set_property     (GObject               *object,
                                                  guint                  property_id,
                                                  const GValue          *value,
                                                  GParamSpec            *pspec);

static void     composite_state_dispose          (GObject               *object);

G_DEFINE_TYPE_WITH_PRIVATE (CompositeState, composite_state, STATE_MACHINES_TYPE_STATE)

static void
composite_state_class_init (CompositeStateClass *klass)
{
  StateClass *state_class = STATE_MACHINES_STATE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  state_class->is_composed_from = composite_state_is_composed_from;

  object_class->constructed = composite_state_constructed;
  object_class->set_property = composite_state_set_property;
  object_class->dispose = composite_state_dispose;

  obj_properties[PROP_COMPOSED_FROM_STATES] =
    g_param_spec_boxed (PROP_COMPOSITE_STATE_COMPOSED_FROM_STATES,
                        "Composed from states",
                        "States which serve as a basis for constructing the composite state.",
                        G_TYPE_PTR_ARRAY,
                        G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);

  obj_properties[PROP_RESOLVE_TYPE_FLAGS] =
    g_param_spec_uint (PROP_COMPOSITE_STATE_RESOLVE_TYPE_FLAGS,
                       "Resolve type flags",
                       "Flags describing how the composite state's own type flags should be resolved.",
                       0,
                       G_MAXUINT32,
                       COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_ALL,
                       G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
composite_state_init (CompositeState *self)
{
  /* NOP */
}

static gboolean
composite_state_is_composed_from (State           *self,
                                  const GPtrArray *states)
{
  g_return_val_if_fail (STATE_MACHINES_IS_COMPOSITE_STATE (self), FALSE);

  CompositeStatePrivate *priv = composite_state_get_instance_private (STATE_MACHINES_COMPOSITE_STATE (self));
  GPtrArray *composed_from_states = priv->composed_from_states;
  GCompareFunc state_equal_func = g_direct_equal;

  return g_ptr_array_equal_with_equal_func (composed_from_states,
                                            states,
                                            state_equal_func);
}

static void
composite_state_constructed (GObject *object)
{
  CompositeStatePrivate *priv = composite_state_get_instance_private (STATE_MACHINES_COMPOSITE_STATE (object));
  GPtrArray *composed_from_states = priv->composed_from_states;
  CompositeStateResolveTypeFlags resolve_type_flags_mode = priv->resolve_type_flags;

  StateTypeFlags composite_state_type_flags = STATE_TYPE_UNDEFINED;

  g_return_if_fail (g_ptr_array_has_items (composed_from_states));

  if ((resolve_type_flags_mode & COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_START) &&
      composite_state_can_be_marked_as (priv, STATE_TYPE_START))
      composite_state_type_flags |= STATE_TYPE_START;

  if ((resolve_type_flags_mode & COMPOSITE_STATE_RESOLVE_TYPE_FLAGS_FINAL) &&
      composite_state_can_be_marked_as (priv, STATE_TYPE_FINAL))
      composite_state_type_flags |= STATE_TYPE_FINAL;

  if (composite_state_type_flags == STATE_TYPE_UNDEFINED)
    composite_state_type_flags |= STATE_TYPE_DEFAULT;

  g_object_set (object,
                PROP_STATE_TYPE_FLAGS, composite_state_type_flags,
                NULL);

  G_OBJECT_CLASS (composite_state_parent_class)->constructed (object);
}

static gboolean
composite_state_can_be_marked_as (CompositeStatePrivate *priv,
                                  StateTypeFlags         state_type_flags)
{
  GPtrArray *composed_from_states = priv->composed_from_states;

  for (guint i = 0; i < composed_from_states->len; ++i)
    {
      State *composed_from_state = g_ptr_array_index (composed_from_states, i);
      StateTypeFlags composed_from_state_type_flags = STATE_TYPE_UNDEFINED;

      g_object_get (composed_from_state,
                    PROP_STATE_TYPE_FLAGS, &composed_from_state_type_flags,
                    NULL);

      if (composed_from_state_type_flags & state_type_flags)
        return TRUE;
    }

  return FALSE;
}

static void
composite_state_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  CompositeStatePrivate *priv = composite_state_get_instance_private (STATE_MACHINES_COMPOSITE_STATE (object));

  switch (property_id)
    {
    case PROP_COMPOSED_FROM_STATES:
      if (priv->composed_from_states != NULL)
        g_ptr_array_unref (priv->composed_from_states);

      priv->composed_from_states = g_value_dup_boxed (value);
      break;

    case PROP_RESOLVE_TYPE_FLAGS:
      priv->resolve_type_flags = (CompositeStateResolveTypeFlags) g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
composite_state_dispose (GObject *object)
{
  CompositeStatePrivate *priv = composite_state_get_instance_private (STATE_MACHINES_COMPOSITE_STATE (object));

  if (priv->composed_from_states != NULL)
    g_clear_pointer (&priv->composed_from_states, g_ptr_array_unref);

  G_OBJECT_CLASS (composite_state_parent_class)->dispose (object);
}
