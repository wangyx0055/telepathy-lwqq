#include "channel.h"
#include <telepathy-glib/telepathy-glib.h>

G_DEFINE_TYPE_WITH_CODE (LwqqChannel, 
      lwqq_channel, 
      TP_TYPE_BASE_CHANNEL,
      );

#define LWQQ_CHANNEL_GET_PRIVATE(o)\
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), LWQQ_CHANNEL_TYPE, LwqqChannelPrivate))

struct _LwqqChannelPrivate {
};

static const char * im_channel_interfaces[] = {
    NULL };

static void
lwqq_channel_dispose (LwqqChannel *self)
{
}

static void
lwqq_channel_finalize (LwqqChannel *self)
{
}

static void
lwqq_channel_init (LwqqChannel *self)
{
   LwqqChannelPrivate *priv;

   priv = LWQQ_CHANNEL_GET_PRIVATE (self);
}

static void
im_channel_close (TpBaseChannel *self)
{
    GObject *object = (GObject *) self;

    if (!tp_base_channel_is_destroyed (self)) {
        TpHandle first_sender;

        /* The manager wants to be able to respawn the channel if it has
         * pending messages. When respawned, the channel must have the
         * initiator set to the contact who sent us those messages (if it isn't
         * already), and the messages must be marked as having been rescued so
         * they don't get logged twice. */
        if (tp_message_mixin_has_pending_messages (object, &first_sender)) {
            tp_message_mixin_set_rescued (object);
            tp_base_channel_reopened (self, first_sender);
        } else {
            tp_base_channel_destroyed (self);
        }
    }
}

static void
lwqq_channel_class_init (LwqqChannelClass *self_class)
{
   GObjectClass *object_class = G_OBJECT_CLASS (self_class);
   TpBaseChannelClass* base_class = TP_BASE_CHANNEL_CLASS(self_class);

   g_type_class_add_private (self_class, sizeof (LwqqChannelPrivate));
   object_class->dispose = (void (*) (GObject *object)) lwqq_channel_dispose;
   object_class->finalize = (void (*) (GObject *object)) lwqq_channel_finalize;

   base_class->channel_type = TP_IFACE_CHANNEL_TYPE_TEXT;
   base_class->target_handle_type = TP_HANDLE_TYPE_CONTACT;
   base_class->interfaces = im_channel_interfaces;
   base_class->close = im_channel_close;
}

