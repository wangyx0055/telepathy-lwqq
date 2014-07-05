#ifndef _LWQQ_CHANNEL_MANAGER_H_
#define _LWQQ_CHANNEL_MANAGER_H_
#include <gobject/gobject.h>
G_BEGIN_DECLS

typedef struct _LwqqChannelManagerPrivate LwqqChannelManagerPrivate;

typedef struct {
   GObject parent;

   LwqqChannelManagerPrivate *priv;
} LwqqChannelManager;

typedef struct {
   GObjectClass parent;
} LwqqChannelManagerClass;

#define LWQQ_CHANNEL_MANAGER_TYPE (lwqq_channel_manager_get_type ())
#define LWQQ_CHANNEL_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), LWQQ_CHANNEL_MANAGER_TYPE, LwqqChannelManager))
#define LWQQ_CHANNEL_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), LWQQ_CHANNEL_MANAGER_TYPE, LwqqChannelManagerClass))
#define IS_LWQQ_CHANNEL_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LWQQ_CHANNEL_MANAGER_TYPE))
#define IS_LWQQ_CHANNEL_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LWQQ_CHANNEL_MANAGER_TYPE))
#define LWQQ_CHANNEL_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), LWQQ_CHANNEL_MANAGER_TYPE, LwqqChannelManagerClass))



G_END_DECLS
#endif
