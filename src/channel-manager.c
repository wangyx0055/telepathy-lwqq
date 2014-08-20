#include "channel-manager.h"
#include "connection.h"
#include "channel.h"

static void
channel_manager_iface_init (gpointer g_iface,
        gpointer data G_GNUC_UNUSED);

G_DEFINE_TYPE_WITH_CODE (LwqqChannelManager, lwqq_channel_manager,
      G_TYPE_OBJECT, 
      G_IMPLEMENT_INTERFACE(TP_TYPE_CHANNEL_MANAGER,
         channel_manager_iface_init)
      );

#define LWQQ_CHANNEL_MANAGER_GET_PRIVATE(o)\
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), LWQQ_CHANNEL_MANAGER_TYPE, LwqqChannelManagerPrivate))

struct _LwqqChannelManagerPrivate {
   LwqqConnection* conn;
   GHashTable* channels;
   unsigned status_changed_id;
};

enum
{
    PROP_CONNECTION = 1,
    N_PROPS
};

static const gchar * const fixed_properties[] = {
    TP_PROP_CHANNEL_CHANNEL_TYPE,
    TP_PROP_CHANNEL_TARGET_HANDLE_TYPE,
    NULL
};

static const gchar * const allowed_properties[] = {
    TP_PROP_CHANNEL_TARGET_HANDLE,
    TP_PROP_CHANNEL_TARGET_ID,
    NULL
};

static void
close_all_channel (LwqqChannelManager* self)
{
   if (self->priv->channels != NULL) {
      GHashTable *tmp = self->priv->channels;

      self->priv->channels = NULL;
      g_hash_table_destroy (tmp);
   }

   if (self->priv->status_changed_id != 0) {
      g_signal_handler_disconnect (self->priv->conn,
            self->priv->status_changed_id);
      self->priv->status_changed_id = 0;
   }

   /*
   if (self->priv->message_received_id != 0) {
      g_signal_handler_disconnect (self,
            self->priv->message_received_id);
      self->priv->message_received_id = 0;
   }*/
}
static void
lwqq_channel_manager_dispose (LwqqChannelManager *self)
{
   close_all_channel(self);
}

static void
status_changed_cb (TpBaseConnection *conn,
        guint status,
        guint reason,
        LwqqChannelManager* self)
{
    if (status == TP_CONNECTION_STATUS_DISCONNECTED)
        close_all_channel(self);
}


static void
lwqq_channel_manager_init (LwqqChannelManager *self)
{
   LwqqChannelManagerPrivate *priv;

   priv = LWQQ_CHANNEL_MANAGER_GET_PRIVATE (self);
   priv->conn = NULL;
   priv->channels = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_object_unref);

   self->priv = priv;
}
static void
set_property (GObject *object,
        guint property_id,
        const GValue *value,
        GParamSpec *pspec)
{
    LwqqChannelManager* self = LWQQ_CHANNEL_MANAGER(object);

    switch (property_id)
    {
        case PROP_CONNECTION:
            /* We don't ref the connection, because it owns a reference to the
* channel manager, and it guarantees that the manager's lifetime is
* less than its lifetime */
            self->priv->conn = g_value_get_object (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

   static void
get_property (GObject *object,
      guint property_id,
      GValue *value,
      GParamSpec *pspec)
{
   LwqqChannelManager *self = LWQQ_CHANNEL_MANAGER(object);
   switch (property_id)
   {
      case PROP_CONNECTION:
         g_value_set_object (value, self->priv->conn);
         break;
      default:
         G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
   }
}
   static void
lwqq_channel_manager_class_init (LwqqChannelManagerClass *self_class)
{
   GObjectClass *object_class = G_OBJECT_CLASS (self_class);

   g_type_class_add_private (self_class, sizeof (LwqqChannelManagerPrivate));
   object_class->dispose = (void (*) (GObject *object)) lwqq_channel_manager_dispose;
   object_class->set_property = set_property;
   object_class->get_property = get_property;

   GParamSpec* param_spec = 
      g_param_spec_object ("connection", "Connection object",
         "The connection that owns this channel manager",
         TP_TYPE_BASE_CONNECTION,
         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
         G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
   g_object_class_install_property (object_class, PROP_CONNECTION, param_spec);
}

static void
channel_closed_cb (LwqqChannel* chan,
        LwqqChannelManager*self)
{
    tp_channel_manager_emit_channel_closed_for_object (self,
            TP_EXPORTABLE_CHANNEL (chan));

    if (self->priv->channels != NULL) {
        TpHandle handle;
        gboolean really_destroyed;

        g_object_get (chan,
                "handle", &handle,
                "channel-destroyed", &really_destroyed,
                NULL);

        /* Re-announce the channel if it's not yet ready to go away (pending
* messages) */
        if (really_destroyed) {
            g_hash_table_remove (self->priv->channels,
                    GUINT_TO_POINTER (handle));
        } else {
            tp_channel_manager_emit_new_channel (self,
                    TP_EXPORTABLE_CHANNEL (chan), NULL);
        }
    }
}

LwqqChannel*
new_channel (LwqqChannelManager* self,
        TpHandle handle,
        TpHandle initiator,
        gpointer request_token)
{
    LwqqChannel* chan;
    gchar *object_path;
    GSList *requests = NULL;

    g_return_val_if_fail (handle != 0, NULL);
    g_return_val_if_fail (initiator != 0, NULL);

    object_path = g_strdup_printf ("%s/lwqq-im-channel-%u",
            tp_base_connection_get_object_path(
               TP_BASE_CONNECTION(self->priv->conn))
            , handle);

    chan = g_object_new (LWQQ_CHANNEL_TYPE,
            "connection", self->priv->conn,
            "object-path", object_path,
            "handle", handle,
            "initiator-handle", initiator,
            NULL);

    g_free (object_path);

    g_signal_connect (chan, "closed", (GCallback) channel_closed_cb, self);

    g_hash_table_insert (self->priv->channels, GUINT_TO_POINTER (handle), chan);

    if (request_token != NULL)
        requests = g_slist_prepend (requests, request_token);

    tp_channel_manager_emit_new_channel (self, TP_EXPORTABLE_CHANNEL (chan),
            requests);
    g_slist_free (requests);

    return chan;
}

static gboolean
request_new_channel (LwqqChannelManager *self,
        gpointer request_token,
        GHashTable *request_properties,
        gboolean require_new)
{
   TpHandle handle;
   LwqqChannel *chan;
   GError *error = NULL;

   if (tp_strdiff (tp_asv_get_string (request_properties,
               TP_PROP_CHANNEL_CHANNEL_TYPE), TP_IFACE_CHANNEL_TYPE_TEXT)) {
      return FALSE;
   }

   if (tp_asv_get_uint32 (request_properties,
            TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, NULL) 
         != TP_HANDLE_TYPE_CONTACT) {
      return FALSE;
   }

   handle = tp_asv_get_uint32 (request_properties,
         TP_PROP_CHANNEL_TARGET_HANDLE, NULL);
   g_assert (handle != 0);

   if (tp_channel_manager_asv_has_unknown_properties (request_properties,
            fixed_properties, allowed_properties, &error)) {
      goto error;
   }

   chan = g_hash_table_lookup (self->priv->channels, GUINT_TO_POINTER (handle));

   if (chan == NULL) {
      new_channel (self, handle,
            tp_base_connection_get_self_handle(TP_BASE_CONNECTION(self->priv->conn)),
            request_token);
   } else if (require_new) {
      g_set_error (&error, TP_ERROR, TP_ERROR_NOT_AVAILABLE,
            "An fetion im channel to contact #%u already exists", handle);
      goto error;
   } else {
      tp_channel_manager_emit_request_already_satisfied (self,
            request_token, TP_EXPORTABLE_CHANNEL (chan));
   }

   return TRUE;

error:
   tp_channel_manager_emit_request_failed (self, request_token,
         error->domain, error->code, error->message);
   g_error_free (error);
   return TRUE;
}

//==================IFACE IMPLEMENT===========================//
static void
foreach_channel (TpChannelManager *iface,
        TpExportableChannelFunc callback,
        gpointer user_data)
{
    LwqqChannelManager* self = LWQQ_CHANNEL_MANAGER(iface);
    GHashTableIter iter;
    gpointer handle, channel;

    g_hash_table_iter_init (&iter, self->priv->channels);

    while (g_hash_table_iter_next (&iter, &handle, &channel))
    {
        callback (TP_EXPORTABLE_CHANNEL (channel), user_data);
    }
}

static void
foreach_channel_class (GType type,
      TpChannelManagerTypeChannelClassFunc func,
      gpointer user_data)
{
   GHashTable *table = tp_asv_new ( 
         TP_PROP_CHANNEL_CHANNEL_TYPE,
         G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_TEXT,
         TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, 
         G_TYPE_UINT, TP_HANDLE_TYPE_CONTACT, 
         NULL);

   func (type, table, allowed_properties, user_data);

   g_hash_table_destroy (table);
}

static gboolean
iface_create_channel (TpChannelManager *manager,
        gpointer request_token,
        GHashTable *request_properties)
{
    return request_new_channel(LWQQ_CHANNEL_MANAGER(manager),
            request_token, request_properties, TRUE);
}

static gboolean
iface_ensure_channel (TpChannelManager *manager,
        gpointer request_token,
        GHashTable *request_properties)
{
    return request_new_channel(LWQQ_CHANNEL_MANAGER(manager),
            request_token, request_properties, FALSE);
}

static void
channel_manager_iface_init (gpointer g_iface,
        gpointer data G_GNUC_UNUSED)
{
    TpChannelManagerIface *iface = g_iface;

    iface->foreach_channel = foreach_channel;
    iface->type_foreach_channel_class = foreach_channel_class;
    iface->create_channel = iface_create_channel;
    iface->ensure_channel = iface_ensure_channel;
    /* In this channel manager, Request has the same semantics as Ensure */
    iface->request_channel = iface_ensure_channel;
}
