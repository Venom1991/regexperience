#ifndef REGEXPERIENCE_H
#define REGEXPERIENCE_H

#include "errors.h"
#include "match.h"

#include <glib-object.h>

G_BEGIN_DECLS

#define CORE_TYPE_REGEXPERIENCE (regexperience_get_type ())
#define regexperience_new(...) (g_object_new (CORE_TYPE_REGEXPERIENCE, ##__VA_ARGS__, NULL))

G_DECLARE_FINAL_TYPE (Regexperience, regexperience, CORE, REGEXPERIENCE, GObject)

void     regexperience_compile (Regexperience  *self,
                                const gchar    *expression,
                                GError        **error);

gboolean regexperience_match   (Regexperience  *self,
                                const gchar    *input,
                                GPtrArray     **matches,
                                GError        **error);

G_END_DECLS

#endif /* REGEXPERIENCE_H */
