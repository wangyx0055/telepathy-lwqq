
#ifndef __IDLE_CONNECTION_H__
#define __IDLE_CONNECTION_H__

#include <glib.h>
#include <telepathy-glib/handle-repo.h>

G_BEGIN_DECLS

void idle_handle_repos_init(TpHandleRepoIface **handles);

G_END_DECLS

#endif
