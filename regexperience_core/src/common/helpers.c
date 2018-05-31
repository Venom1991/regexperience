#include "internal/common/helpers.h"

#include <glib-object.h>

gboolean
g_char_any (gconstpointer a,
            gconstpointer b)
{
  return TRUE;
}

gboolean
g_char_equal (gconstpointer a,
              gconstpointer b)
{
  const gchar a_char = (const gchar) GPOINTER_TO_INT (a);
  const gchar b_char = (const gchar) GPOINTER_TO_INT (b);

  return a_char == b_char;
}

gboolean
g_char_not_equal (gconstpointer a,
                  gconstpointer b)
{
  const gchar a_char = (const gchar) GPOINTER_TO_INT (a);
  const gchar b_char = (const gchar) GPOINTER_TO_INT (b);

  return a_char != b_char;
}

gboolean
g_ptr_array_equal (gconstpointer a,
                   gconstpointer b)
{
  return g_ptr_array_equal_with_equal_func (a, b, g_direct_equal);
}

gboolean
g_ptr_array_equal_with_equal_func (gconstpointer a,
                                   gconstpointer b,
                                   GEqualFunc    equal_func)
{
  g_return_val_if_fail (a != NULL && b != NULL, FALSE);
  g_return_val_if_fail (equal_func != NULL, FALSE);

  GPtrArray *a_ptr = (GPtrArray *) a;
  GPtrArray *b_ptr = (GPtrArray *) b;
  guint a_len = a_ptr->len;
  guint b_len = b_ptr->len;

  if (a_len == b_len)
    for (guint i = 0; i < a_len; ++i)
      {
        gpointer a_data = g_ptr_array_index (a_ptr, i);

        if (!g_ptr_array_find_with_equal_func (b_ptr, a_data, equal_func, NULL))
          return FALSE;
      }
  else
    return FALSE;

  return TRUE;
}

gint
g_compare_strings (gconstpointer a,
                   gconstpointer b)
{
  const gchar **a_ptr = (const gchar **) a;
  const gchar **b_ptr = (const gchar **) b;

  return g_strcmp0 (*a_ptr, *b_ptr);
}

gpointer
g_ptr_array_bsearch (GPtrArray     *ptr_array,
                     GCompareFunc   compare_func,
                     gconstpointer  key)
{
  g_return_val_if_fail (ptr_array != NULL, NULL);
  g_return_val_if_fail (compare_func != NULL, NULL);

  gpointer *result = NULL;

  result = bsearch (key,
                    ptr_array->pdata,
                    ptr_array->len,
                    sizeof (gpointer),
                    compare_func);

  if (result != NULL)
    return *result;

  return NULL;
}

void
g_ptr_array_add_range (GPtrArray *destination,
                       GPtrArray *source,
                       GRefFunc   ref_func)
{
  g_return_if_fail (destination != NULL);
  g_return_if_fail (source != NULL);

  for (guint i = 0; i < source->len; ++i)
    {
      gpointer data = NULL;

      if (ref_func == NULL)
        data = g_ptr_array_index (source, i);
      else
        data = ref_func (g_ptr_array_index (source, i));

      g_ptr_array_add (destination, data);
    }
}

void
g_ptr_array_add_range_distinct (GPtrArray  *destination,
                                GPtrArray  *source,
                                GEqualFunc  equal_func,
                                GRefFunc    ref_func)
{
  g_return_if_fail (destination != NULL);
  g_return_if_fail (source != NULL);

  for (guint i = 0; i < source->len; ++i)
    {
      gpointer data = g_ptr_array_index (source, i);

      g_ptr_array_add_if_not_exists (destination,
                                     data,
                                     equal_func,
                                     ref_func);
    }
}

void
g_ptr_array_add_multiple (GPtrArray *destination,
                          ...)
{
  g_return_if_fail (destination != NULL);

  va_list ap;
  gpointer data = NULL;

  va_start (ap, destination);

  while (TRUE)
    {
      data = va_arg (ap, gpointer);

      if (data != NULL)
        g_ptr_array_add (destination, data);
      else
        break;
    }

  va_end (ap);
}

void
g_ptr_array_add_if_not_exists (GPtrArray  *ptr_array,
                               gpointer    data,
                               GEqualFunc  equal_func,
                               GRefFunc    ref_func)
{
  g_return_if_fail (ptr_array != NULL);
  g_return_if_fail (equal_func != NULL);

  if (!g_ptr_array_find_with_equal_func (ptr_array, data, equal_func, NULL))
    {
      if (ref_func == NULL)
        g_ptr_array_add (ptr_array, data);
      else
        g_ptr_array_add (ptr_array, ref_func (data));
    }
}

gboolean
g_ptr_array_has_items (GPtrArray *ptr_array)
{
  return ptr_array != NULL && ptr_array->len > 0;
}

gboolean
g_array_has_items (GArray *array)
{
  return array != NULL && array->len > 0;
}

gboolean
g_queue_has_items (GQueue *queue)
{
  return queue != NULL && g_queue_get_length (queue) > 0;
}

gboolean
g_hash_table_has_items (GHashTable *hash_table)
{
  return hash_table != NULL && g_hash_table_size (hash_table) > 0;
}

GPtrArray *
g_hash_table_values_to_ptr_array (GHashTable     *hash_table,
                                  GDestroyNotify  free_func)
{
  g_return_val_if_fail (g_hash_table_has_items (hash_table), NULL);

  GPtrArray *array = NULL;
  GHashTableIter iter;
  gpointer key = NULL;
  gpointer value = NULL;
  guint hash_table_size = g_hash_table_size (hash_table);

  array = g_ptr_array_sized_new (hash_table_size);

  if (free_func != NULL)
    g_ptr_array_set_free_func (array, free_func);

  g_hash_table_iter_init (&iter, hash_table);

  while (g_hash_table_iter_next (&iter, &key, &value))
    g_ptr_array_add (array, value);

  return array;
}

void
g_queue_unref_g_objects (GQueue *queue)
{
  g_return_if_fail (queue != NULL);

  if (g_queue_has_items (queue))
    g_queue_free_full (queue, g_object_unref);
}

gboolean
g_node_unref_g_object (GNode    *node,
                       gpointer  data)
{
  g_return_val_if_fail (node != NULL, TRUE);
  g_return_val_if_fail (node->data != NULL, TRUE);
  g_return_val_if_fail (G_IS_OBJECT (node->data), TRUE);

  g_object_unref (node->data);

  return FALSE;
}

void
g_node_unref_g_objects (GNode *node)
{
  g_return_if_fail (node != NULL);

  g_node_traverse (node,
                   G_IN_ORDER,
                   G_TRAVERSE_ALL,
                   -1,
                   g_node_unref_g_object,
                   NULL);

  g_node_destroy (node);
}
