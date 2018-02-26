#include "internal/syntactic_analysis/derivation_item.h"
#include "internal/syntactic_analysis/grammar/production.h"
#include "internal/syntactic_analysis/grammar/rule.h"

typedef struct
{
    Production *left_hand_side;
    Rule       *right_hand_side;
} DerivationItemPrivate;

struct _DerivationItem
{
    GObject parent_instance;
};

G_DEFINE_TYPE_WITH_PRIVATE (DerivationItem, derivation_item, G_TYPE_OBJECT)

enum
{
    PROP_LEFT_HAND_SIDE = 1,
    PROP_RIGHT_HAND_SIDE,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void derivation_item_get_property (GObject    *object,
                                          guint       property_id,
                                          GValue     *value,
                                          GParamSpec *pspec);

static void derivation_item_set_property (GObject      *object,
                                          guint         property_id,
                                          const GValue *value,
                                          GParamSpec   *pspec);

static void derivation_item_dispose (GObject *object);

static void
derivation_item_class_init (DerivationItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = derivation_item_get_property;
  object_class->set_property = derivation_item_set_property;
  object_class->dispose = derivation_item_dispose;

  obj_properties[PROP_LEFT_HAND_SIDE] =
      g_param_spec_object (PROP_DERIVATION_ITEM_LEFT_HAND_SIDE,
                           "Left-hand side",
                           "Production representing the left-hand side of the derivation item.",
                           SYNTACTIC_ANALYSIS_TYPE_PRODUCTION,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  obj_properties[PROP_RIGHT_HAND_SIDE] =
      g_param_spec_object (PROP_DERIVATION_ITEM_RIGHT_HAND_SIDE,
                           "Right-hand side",
                           "Rule representing the right-hand side of the derivation item.",
                           SYNTACTIC_ANALYSIS_TYPE_RULE,
                           G_PARAM_CONSTRUCT | G_PARAM_READWRITE);

  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     obj_properties);
}

static void
derivation_item_init (DerivationItem *self)
{
  /* NOP */
}

static void
derivation_item_get_property (GObject    *object,
                              guint       property_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  DerivationItemPrivate *priv = derivation_item_get_instance_private (SYNTACTIC_ANALYSIS_DERIVATION_ITEM (object));

  switch (property_id)
    {
    case PROP_LEFT_HAND_SIDE:
      g_value_set_object (value, priv->left_hand_side);
      break;

    case PROP_RIGHT_HAND_SIDE:
      g_value_set_object (value, priv->right_hand_side);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
derivation_item_set_property (GObject      *object,
                              guint         property_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  DerivationItemPrivate *priv = derivation_item_get_instance_private (SYNTACTIC_ANALYSIS_DERIVATION_ITEM (object));

  switch (property_id)
    {
    case PROP_LEFT_HAND_SIDE:
      if (priv->left_hand_side != NULL)
        g_object_unref (priv->left_hand_side);

      priv->left_hand_side = g_value_dup_object (value);
      break;

    case PROP_RIGHT_HAND_SIDE:
      if (priv->right_hand_side != NULL)
        g_object_unref (priv->right_hand_side);

      priv->right_hand_side = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
derivation_item_dispose (GObject *object)
{
  DerivationItemPrivate *priv = derivation_item_get_instance_private (SYNTACTIC_ANALYSIS_DERIVATION_ITEM (object));

  if (priv->left_hand_side != NULL)
    g_clear_object (&priv->left_hand_side);

  if (priv->right_hand_side != NULL)
    g_clear_object (&priv->right_hand_side);

  G_OBJECT_CLASS (derivation_item_parent_class)->dispose (object);
}
