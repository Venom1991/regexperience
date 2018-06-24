#ifndef REGEXPERIENCE_HELPERS_H
#define REGEXPERIENCE_HELPERS_H

#include <glib.h>

#define DELIMITER    "-|-"
#define END_OF_INPUT "EOI"

typedef gpointer (*GRefFunc) (gpointer data);

gboolean   g_char_any                        (gconstpointer   a,
                                              gconstpointer   b);

gboolean   g_char_equal                      (gconstpointer   a,
                                              gconstpointer   b);

gboolean   g_char_not_equal                  (gconstpointer   a,
                                              gconstpointer   b);

gboolean   g_ptr_array_equal                 (gconstpointer   a,
                                              gconstpointer   b);

gboolean   g_ptr_array_equal_with_equal_func (gconstpointer   a,
                                              gconstpointer   b,
                                              GEqualFunc      equal_func);

gint       g_compare_strings                 (gconstpointer   a,
                                              gconstpointer   b);

gint       g_compare_transitions             (gconstpointer   a,
                                              gconstpointer   b);

gpointer   g_ptr_array_bsearch               (GPtrArray      *ptr_array,
                                              GCompareFunc    compare_func,
                                              gconstpointer   key);

void       g_ptr_array_add_range             (GPtrArray      *destination,
                                              GPtrArray      *source,
                                              GRefFunc        ref_func);

void       g_ptr_array_add_range_distinct    (GPtrArray      *destination,
                                              GPtrArray      *source,
                                              GEqualFunc      equal_func,
                                              GRefFunc        ref_func);

void       g_ptr_array_add_multiple          (GPtrArray      *destination,
                                              ...);

void       g_ptr_array_add_if_not_exists     (GPtrArray      *ptr_array,
                                              gpointer        data,
                                              GEqualFunc      equal_func,
                                              GRefFunc        ref_func);

gboolean   g_ptr_array_has_items             (GPtrArray      *ptr_array);

gboolean   g_array_has_items                 (GArray         *array);


gboolean   g_queue_has_items                 (GQueue         *queue);

gboolean   g_hash_table_has_items            (GHashTable     *hash_table);

GPtrArray *g_hash_table_values_to_ptr_array  (GHashTable     *hash_table,
                                              GDestroyNotify  free_func);

void       g_queue_unref_g_objects           (GQueue         *queue);

void       g_node_unref_g_objects            (GNode          *node);

#endif /* REGEXPERIENCE_HELPERS_H */
