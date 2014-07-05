#ifndef __LWQQ_CHANNEL_H_
#define __LWQQ_CHANNEL_H_
#include <telepathy-glib/telepathy-glib.h>
G_BEGIN_DECLS

typedef struct _LwqqChannelPrivate LwqqChannelPrivate;

typedef struct {
   TpBaseChannelClass parent_class;
} LwqqChannelClass;

typedef struct {
   TpBaseChannel parent;
   TpMessageMixin text;

   LwqqChannelPrivate *priv;
} LwqqChannel;


#define LWQQ_CHANNEL_TYPE (lwqq_channel_get_type ())
#define LWQQ_CHANNEL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), LWQQ_CHANNEL_TYPE, LwqqChannel))
#define LWQQ_CHANNEL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), LWQQ_CHANNEL_TYPE, LwqqChannelClass))
#define IS_LWQQ_CHANNEL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LWQQ_CHANNEL_TYPE))
#define IS_LWQQ_CHANNEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LWQQ_CHANNEL_TYPE))
#define LWQQ_CHANNEL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), LWQQ_CHANNEL_TYPE, LwqqChannelClass))



G_END_DECLS
#endif
