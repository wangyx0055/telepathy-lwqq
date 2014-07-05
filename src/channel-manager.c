#include "channel-manager.h"

G_DEFINE_TYPE (LwqqChannelManager, lwqq_channel_manager, G_TYPE_OBJECT);

#define LWQQ_CHANNEL_MANAGER_GET_PRIVATE(o)\
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), LWQQ_CHANNEL_MANAGER_TYPE, LwqqChannelManagerPrivate))

struct _LwqqChannelManagerPrivate {
};


static void
lwqq_channel_manager_dispose (LwqqChannelManager *self)
{
}

static void
lwqq_channel_manager_finalize (LwqqChannelManager *self)
{
}

static void
lwqq_channel_manager_init (LwqqChannelManager *self)
{
   LwqqChannelManagerPrivate *priv;

   priv = LWQQ_CHANNEL_MANAGER_GET_PRIVATE (self);
}

static void
lwqq_channel_manager_class_init (LwqqChannelManagerClass *self_class)
{
   GObjectClass *object_class = G_OBJECT_CLASS (self_class);

   g_type_class_add_private (self_class, sizeof (LwqqChannelManagerPrivate));
   object_class->dispose = (void (*) (GObject *object)) lwqq_channel_manager_dispose;
   object_class->finalize = (void (*) (GObject *object)) lwqq_channel_manager_finalize;
}

