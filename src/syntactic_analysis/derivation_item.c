#include "internal/syntactic_analysis/derivation_item.h"
#include "internal/syntactic_analysis/production.h"
#include "internal/syntactic_analysis/rule.h"

struct _DerivationItem
{
    GObject parent_instance;
};

typedef struct
{
    GWeakRef left_hand_side;
    GWeakRef right_hand_side;
} DerivationItemPrivate;

enum
{
    PROP_LEFT_HAND_SIDE = 1,
    PROP_RIGHT_HAND_SIDE,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL };

static void derivation_item_get_property (GObject      *object,
                                          guint         property_id,
                                          GValue       *value,
                                          GParamSpec   *pspec);

static void derivation_item_set_property (GObject      *object,
                                          guint         property_id,
                                          const GValue *value,
                                          GParamSpec   *pspec);

static void derivation_item_dispose      (GObject      *object);

G_DEFINE_TYPE_WITH_PRIVATE (DerivationItem, derivation_item, G_TYPE_OBJECT)

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
  DerivationItemPrivate *priv = derivation_item_get_instance_private (self);

  g_weak_ref_init (&priv->left_hand_side, NULL);
  g_weak_ref_init (&priv->right_hand_side, NULL);
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
      {
        Production *production = g_weak_ref_get (&priv->left_hand_side);

        g_value_take_object (value, production);
      }
      break;

    case PROP_RIGHT_HAND_SIDE:
      {
        Rule *rule = g_weak_ref_get (&priv->right_hand_side);

        g_value_take_object (value, rule);
      }
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
      {
        Production *production = g_value_get_object (value);

        g_weak_ref_set (&priv->left_hand_side, production);
      }
      break;

    case PROP_RIGHT_HAND_SIDE:
      {
        Rule *production = g_value_get_object (value);

        g_weak_ref_set (&priv->right_hand_side, production);
      }
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

  g_weak_ref_clear (&priv->left_hand_side);
  g_weak_ref_clear (&priv->right_hand_side);

  G_OBJECT_CLASS (derivation_item_parent_class)->dispose (object);
}
