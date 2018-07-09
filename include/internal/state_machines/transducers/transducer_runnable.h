#ifndef REGEXPERIENCE_TRANSDUCER_RUNNABLE_H
#define REGEXPERIENCE_TRANSDUCER_RUNNABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define TRANSDUCERS_TYPE_TRANSDUCER_RUNNABLE (transducer_runnable_get_type ())

G_DECLARE_INTERFACE (TransducerRunnable, transducer_runnable, TRANSDUCERS, TRANSDUCER_RUNNABLE, GObject)

struct _TransducerRunnableInterface
{
  GTypeInterface parent_iface;

  void     (*reset) (TransducerRunnable *self);

  gpointer (*run)   (TransducerRunnable *self,
                     gchar               input);
};

void     transducer_runnable_reset (TransducerRunnable *self);

gpointer transducer_runnable_run   (TransducerRunnable *self,
                                    gchar               input);

G_END_DECLS

#endif /* REGEXPERIENCE_TRANSDUCER_RUNNABLE_H */
