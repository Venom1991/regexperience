#ifndef REGEXPERIENCE_CORE_ACCEPTOR_RUNNABLE_H
#define REGEXPERIENCE_CORE_ACCEPTOR_RUNNABLE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define ACCEPTORS_TYPE_ACCEPTOR_RUNNABLE (acceptor_runnable_get_type ())

G_DECLARE_INTERFACE (AcceptorRunnable, acceptor_runnable, ACCEPTORS, ACCEPTOR_RUNNABLE, GObject)

struct _AcceptorRunnableInterface
{
    GTypeInterface parent_iface;

    void (*run) (AcceptorRunnable *self,
                 const gchar      *input);
    gboolean (*can_accept) (AcceptorRunnable  *self);
};

void acceptor_runnable_run (AcceptorRunnable *self,
                            const gchar      *input);

gboolean acceptor_runnable_can_accept (AcceptorRunnable *self);

G_END_DECLS

#endif /* REGEXPERIENCE_CORE_ACCEPTOR_RUNNABLE_H */
