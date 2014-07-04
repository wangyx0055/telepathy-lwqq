#include "presence.h"

static gboolean
status_available (GObject *object,
                  guint index_)
{
  TpBaseConnection *base = TP_BASE_CONNECTION (object);

  return tp_base_connection_check_connected (base, NULL);
}

static GHashTable *
get_contact_statuses (GObject *object,
                      const GArray *contacts,
                      GError **error)
{
    LwqqConnection *self = LWQQ_CONNECTION(object);
    TpBaseConnection *base = TP_BASE_CONNECTION (object);
    LwqqClient* lc = self->lc;
    guint i;
    GHashTable *result = g_hash_table_new_full (g_direct_hash, g_direct_equal,
            NULL, (GDestroyNotify) tp_presence_status_free);

    TpHandleRepoIface* contact_repo = tp_base_connection_get_handles(base, TP_HANDLE_TYPE_CONTACT);

    for (i = 0; i < contacts->len; i++)
    {
        TpHandle contact = g_array_index (contacts, guint, i);
        LwqqStatus presence;
        GHashTable *parameters;

        /* we get our own status from the connection, and everyone else's status
         * from the contact lists */
        if (contact == tp_base_connection_get_self_handle (base)) {
            presence = to_presence(lc->myself->stat);
            // presence_message = lc->myself->long_nick;
        
        } else {
            presence = lwqq_contact_list_get_presence(self->contact_list, contact);
            // presence_message = "hi";
        }

        parameters = g_hash_table_new_full (g_str_hash,
                g_str_equal, NULL, (GDestroyNotify) tp_g_value_slice_free);
        g_hash_table_insert (result, GUINT_TO_POINTER (contact),
                tp_presence_status_new (presence, parameters));
        g_hash_table_unref (parameters);
    }

    return result;
}

static gboolean
set_own_status (GObject *object,
                const TpPresenceStatus *status,
                GError **error)
{
    LwqqConnection* self = LWQQ_CONNECTION(object);
    TpBaseConnection *base = TP_BASE_CONNECTION (object);
    LwqqClient* lc = self->lc;
    GHashTable *presences;

    LwqqStatus state = status->index*10;

    LWQQ_SYNC_BEGIN(lc);
    lwqq_info_change_status(lc, state);
    LWQQ_SYNC_END(lc);

    presences = g_hash_table_new_full (g_direct_hash, g_direct_equal,
          NULL, NULL);
    g_hash_table_insert (presences,
          GUINT_TO_POINTER (tp_base_connection_get_self_handle (base)),
          (gpointer) status);
    tp_presence_mixin_emit_presence_update (object, presences);
    g_hash_table_unref (presences);
    return TRUE;
}

void presence_class_init(LwqqConnectionClass* klass)
{
   GObjectClass* object_class = (GObjectClass*) klass;
   TpPresenceMixinClass* mixin_class;
   tp_presence_mixin_class_init (object_class,
         G_STRUCT_OFFSET (LwqqConnectionClass, presence_class),
         status_available, get_contact_statuses, set_own_status,
         lwqq_contact_list_presence_statuses());
   tp_presence_mixin_simple_presence_init_dbus_properties (object_class);
}

static void
presence_updated_cb (LwqqConnection* conn,
                     TpHandle contact,
                     gpointer user_data)
{
  TpBaseConnection *base = (TpBaseConnection *) conn;
  TpPresenceStatus *status;
  LwqqClient* lc = conn->lc;
  guint presence;

  /* we ignore the presence indicated by the contact list for our own handle */
  if (contact == base->self_handle)
     return;

  LwqqContactList* contact_list = conn->contact_list;
  presence = lwqq_contact_list_get_presence(contact_list, contact);
  if(presence == 0) return;
  status = tp_presence_status_new (presence, NULL);
  tp_presence_mixin_emit_one_presence_update ((GObject *) base, contact,
          status);
  tp_presence_status_free (status);
}

void presence_init(GObject* object)
{
   g_signal_connect(object, "presence-updated", (GCallback)presence_updated_cb,
         NULL);
   tp_presence_mixin_init (object,
         G_STRUCT_OFFSET (LwqqConnection, presence));
   tp_presence_mixin_simple_presence_register_with_contacts_mixin (object);
}
