#ifndef REGEXPERIENCE_CORE_HELPERS_H
#define REGEXPERIENCE_CORE_HELPERS_H

#include <glib.h>

typedef gpointer (*GRefFunc) (gpointer data);

gboolean g_char_any (gconstpointer a, gconstpointer b);

gboolean g_char_equal (gconstpointer a, gconstpointer b);

gboolean g_char_not_equal (gconstpointer a, gconstpointer b);

gboolean g_ptr_array_equal (gconstpointer a,
                            gconstpointer b);

gboolean g_ptr_array_equal_with_equal_func (gconstpointer a,
                                            gconstpointer b,
                                            GEqualFunc    equal_func);

gint g_compare_strings (gconstpointer a, gconstpointer b);

gpointer g_ptr_array_bsearch (GPtrArray     *ptr_array,
                              GCompareFunc   compare_func,
                              gconstpointer  key);

void g_ptr_array_add_range (GPtrArray *destination,
                            GPtrArray *source,
                            GRefFunc   ref_func);

void g_ptr_array_add_range_distinct (GPtrArray    *destination,
                                     GPtrArray    *source,
                                     GEqualFunc    equal_func);

void g_ptr_array_add_multiple (GPtrArray *destination,
                               ...);

void g_ptr_array_add_if_not_exists (GPtrArray    *ptr_array,
                                    gpointer      data,
                                    GEqualFunc    equal_func);

gboolean g_ptr_array_has_items (GPtrArray *ptr_array);

gboolean g_array_has_items (GArray *array);

gboolean g_hash_table_has_items (GHashTable *hash_table);

GQueue *g_queue_copy_g_objects (GQueue *queue);

void g_queue_free_g_objects (GQueue *queue);

void g_node_unref_g_objects (GNode *node);

#endif /* REGEXPERIENCE_CORE_HELPERS_H */
