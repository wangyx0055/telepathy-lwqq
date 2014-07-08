#include "channel.h"
#include "connection.h"
#include <telepathy-glib/telepathy-glib.h>

G_DEFINE_TYPE_WITH_CODE (LwqqChannel, 
      lwqq_channel, 
      TP_TYPE_BASE_CHANNEL,
      G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CHANNEL_TYPE_TEXT,
         tp_message_mixin_text_iface_init);
      G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CHANNEL_INTERFACE_MESSAGES,
         tp_message_mixin_messages_iface_init)
      );

#define LWQQ_CHANNEL_GET_PRIVATE(o)\
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), LWQQ_CHANNEL_TYPE, LwqqChannelPrivate))

struct _LwqqChannelPrivate {
};

static const char * im_channel_interfaces[] = {
   TP_IFACE_CHANNEL_INTERFACE_MESSAGES,
   NULL };

static void
lwqq_channel_finalize (LwqqChannel *self)
{
   tp_message_mixin_finalize(G_OBJECT(self));
}

/** start send message **/
static void
send_message (GObject *object,
        TpMessage *message,
        TpMessageSendingFlags flags)
{
    LwqqChannel* self = LWQQ_CHANNEL(object);
    TpBaseChannel *base = TP_BASE_CHANNEL (self);
    if (tp_asv_get_string (tp_message_peek (message, 0), "interface") != NULL)
    {
        /* this message is interface-specific - let's not echo it */
        goto finally;
    }

    LwqqConnection *conn =
        LWQQ_CONNECTION(tp_base_channel_get_connection(base));

    LwqqClient* lc = conn->lc;

    TpHandle to = tp_base_channel_get_target_handle (base);
    const GHashTable *input = tp_message_peek (message, 1);
    const gchar *send_message = tp_asv_get_string (input, "content");
    LwqqBuddy* buddy = lwqq_find_buddy_by_handle(conn, to);
    lwqq_msg_send_simple(lc, LWQQ_MS_BUDDY_MSG, buddy->uin, send_message);

finally:
    /* "OK, we've sent the message" (after calling this, message must not be
* dereferenced) */
    tp_message_mixin_sent (object, message, flags, "", NULL);
}

static void
lwqq_channel_init (LwqqChannel *self)
{
   LwqqChannelPrivate *priv;

   priv = LWQQ_CHANNEL_GET_PRIVATE (self);
   self->priv = priv;

   TpBaseChannel* base = TP_BASE_CHANNEL(self);
   GObject* obj = G_OBJECT(self);

   static TpChannelTextMessageType const types[] = {
      TP_CHANNEL_TEXT_MESSAGE_TYPE_NORMAL
   };

   static const char* const content_types[] = {"text/plain", NULL};

   tp_base_channel_register(base);

   tp_message_mixin_init(G_OBJECT(self), G_STRUCT_OFFSET(LwqqChannel, text),
         tp_base_channel_get_connection(base));

   tp_message_mixin_implement_sending(obj, send_message, G_N_ELEMENTS(types),
         types, 0, TP_DELIVERY_REPORTING_SUPPORT_FLAG_RECEIVE_FAILURES,
         content_types);
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
fill_immutable_properties (TpBaseChannel *chan,
        GHashTable *properties)
{
    TpBaseChannelClass *klass = TP_BASE_CHANNEL_CLASS (
            lwqq_channel_parent_class);

    klass->fill_immutable_properties (chan, properties);

    tp_dbus_properties_mixin_fill_properties_hash (
            G_OBJECT (chan), properties,
            TP_IFACE_CHANNEL_INTERFACE_MESSAGES, "MessagePartSupportFlags",
            TP_IFACE_CHANNEL_INTERFACE_MESSAGES, "DeliveryReportingSupport",
            TP_IFACE_CHANNEL_INTERFACE_MESSAGES, "SupportedContentTypes",
            TP_IFACE_CHANNEL_INTERFACE_MESSAGES, "MessageTypes",
            NULL);
}
static void
lwqq_channel_class_init (LwqqChannelClass *self_class)
{
   GObjectClass *object_class = G_OBJECT_CLASS (self_class);
   TpBaseChannelClass* base_class = TP_BASE_CHANNEL_CLASS(self_class);

   g_type_class_add_private (self_class, sizeof (LwqqChannelPrivate));
   object_class->finalize = (void (*) (GObject *object)) lwqq_channel_finalize;

   base_class->channel_type = TP_IFACE_CHANNEL_TYPE_TEXT;
   base_class->target_handle_type = TP_HANDLE_TYPE_CONTACT;
   base_class->interfaces = im_channel_interfaces;
   base_class->close = im_channel_close;
   base_class->fill_immutable_properties = fill_immutable_properties;

   tp_message_mixin_init_dbus_properties(object_class);
}

