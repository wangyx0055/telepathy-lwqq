#include "aliasing.h"

static void
get_alias_flags (TpSvcConnectionInterfaceAliasing *aliasing,
        DBusGMethodInvocation *context)
{
    TpBaseConnection *base = TP_BASE_CONNECTION (aliasing);

    TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED (base, context);
    tp_svc_connection_interface_aliasing_return_from_get_alias_flags (context,
            TP_CONNECTION_ALIAS_FLAG_USER_SET);
}

static void
get_aliases (TpSvcConnectionInterfaceAliasing *aliasing,
        const GArray *contacts, DBusGMethodInvocation *context)
{
    LwqqConnection *self =
        LWQQ_CONNECTION (aliasing);
    TpBaseConnection *base = TP_BASE_CONNECTION (aliasing);
    TpHandleRepoIface *contact_repo = tp_base_connection_get_handles (base,
            TP_HANDLE_TYPE_CONTACT);
    LwqqClient* lc = self->lc;
    GHashTable *result;
    GError *error = NULL;
    guint i;

    TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED (base, context);

    if (!tp_handles_are_valid (contact_repo, contacts, FALSE, &error))
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
        return;
    }

    result = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, NULL);

    for (i = 0; i < contacts->len; i++)
    {
        TpHandle contact = g_array_index (contacts, TpHandle, i);
        const gchar *alias;
        if (contact == base->self_handle){
            alias = lc->myself->nick;
        }else{
           const char* qq = tp_handle_inspect(contact_repo, contact);
           LwqqBuddy* buddy = lwqq_buddy_find_buddy_by_qqnumber(lc, qq);
           if(buddy == NULL)
              continue;

           alias = buddy->markname?:buddy->nick?:buddy->qqnumber;
        }

        g_hash_table_insert (result, GUINT_TO_POINTER (contact),
                (gchar *) alias);
    }

    tp_svc_connection_interface_aliasing_return_from_get_aliases (context,
          result);
    g_hash_table_destroy (result);
}

static void
request_aliases (TpSvcConnectionInterfaceAliasing *aliasing,
        const GArray *contacts, DBusGMethodInvocation *context)
{
    LwqqConnection *self =
        LWQQ_CONNECTION (aliasing);
    TpBaseConnection *base = TP_BASE_CONNECTION (aliasing);
    TpHandleRepoIface *contact_repo = tp_base_connection_get_handles (base,
            TP_HANDLE_TYPE_CONTACT);
    LwqqClient* lc = self->lc;
    GPtrArray *result;
    gchar **strings;
    GError *error = NULL;
    guint i;

    TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED (base, context);

    if (!tp_handles_are_valid (contact_repo, contacts, FALSE, &error))
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
        return;
    }

    result = g_ptr_array_sized_new (contacts->len + 1);

    for (i = 0; i < contacts->len; i++)
    {
       TpHandle contact = g_array_index (contacts, TpHandle, i);
       const gchar *alias ;
       if (contact == base->self_handle){
          alias = lc->myself->nick;
       }else{
           const char* qq = tp_handle_inspect(contact_repo, contact);
           LwqqBuddy* buddy = lwqq_buddy_find_buddy_by_qqnumber(lc, qq);
           if(buddy == NULL)
              continue;

           alias = buddy->markname?:buddy->nick?:buddy->qqnumber;
       }

       g_ptr_array_add (result, (gchar *) alias);
    }

    g_ptr_array_add (result, NULL);
    strings = (gchar **) g_ptr_array_free (result, FALSE);
    tp_svc_connection_interface_aliasing_return_from_request_aliases (context,
            (const gchar **) strings);
    g_free (strings);
}

static void
set_aliases (TpSvcConnectionInterfaceAliasing *aliasing,
        GHashTable *aliases, DBusGMethodInvocation *context)
{
    LwqqConnection* self =
        LWQQ_CONNECTION (aliasing);
    TpBaseConnection *base = TP_BASE_CONNECTION (aliasing);
    TpHandleRepoIface *contact_repo = tp_base_connection_get_handles (base,
            TP_HANDLE_TYPE_CONTACT);
    LwqqClient* lc = self->lc;
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, aliases);

    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        GError *error = NULL;

        if (!tp_handle_is_valid (contact_repo, GPOINTER_TO_UINT (key),
                    &error))
        {
            dbus_g_method_return_error (context, error);
            g_error_free (error);
            return;
        }
    }

    g_hash_table_iter_init (&iter, aliases);

    while (g_hash_table_iter_next (&iter, &key, &value))
    {
        if(GPOINTER_TO_UINT(key) == base->self_handle){
        } else {
           const char* qq = tp_handle_inspect(contact_repo,
                 GPOINTER_TO_UINT(key));
           LwqqBuddy* buddy = lwqq_buddy_find_buddy_by_qqnumber(lc, qq);
           if(buddy == NULL)
              continue;
           lwqq_info_change_buddy_markname(lc, buddy, value);
        }
    }
    tp_svc_connection_interface_aliasing_return_from_set_aliases (context);
}

static void
aliasing_fill_contact_attributes (GObject *object,
        const GArray *contacts,
        GHashTable *attributes)
{
    LwqqConnection *self =
        LWQQ_CONNECTION (object);
    TpBaseConnection *base = TP_BASE_CONNECTION (object);
    TpHandleRepoIface * contact_repo = tp_base_connection_get_handles(base,
          TP_HANDLE_TYPE_CONTACT);
    LwqqClient* lc = self->lc;
    guint i;

    for (i = 0; i < contacts->len; i++)
    {
        TpHandle contact = g_array_index (contacts, guint, i);
        const gchar *name;
        if (contact == base->self_handle)
        {
            name = lc->myself->nick;
        }else
        {
           const char* qq = tp_handle_inspect(contact_repo, contact);
           LwqqBuddy* buddy = lwqq_buddy_find_buddy_by_qqnumber(lc, qq);
           if(buddy == NULL)
              continue;

           name = buddy->markname?:buddy->nick?:buddy->qqnumber;
        }

        tp_contacts_mixin_set_contact_attribute (attributes, contact,
                TP_TOKEN_CONNECTION_INTERFACE_ALIASING_ALIAS,
                tp_g_value_slice_new_string (name));
    }
}

void aliasing_iface_init(gpointer iface, gpointer iface_data G_GNUC_UNUSED)
{
   TpSvcConnectionInterfaceAliasingClass* klass = iface;
#define IMPLEMENT(x) tp_svc_connection_interface_aliasing_implement_##x (\
klass, x)
    IMPLEMENT(get_alias_flags);
    IMPLEMENT(request_aliases);
    IMPLEMENT(get_aliases);
    IMPLEMENT(set_aliases);
#undef IMPLEMENT
}

static void
alias_updated_cb( LwqqConnection *conn,
        TpHandle contact, gpointer user_data)
{
    TpBaseConnection *base = (TpBaseConnection *) conn;
    TpHandleRepoIface * contact_repo = tp_base_connection_get_handles(base,
          TP_HANDLE_TYPE_CONTACT);
    LwqqClient* lc = conn->lc;
    const gchar *name;
    if (contact == base->self_handle)
    {
        name = lc->myself->nick;
    }else
    {
       const char* qq = tp_handle_inspect(contact_repo, contact);
       LwqqBuddy* buddy = lwqq_buddy_find_buddy_by_qqnumber(lc, qq);
       if(buddy == NULL)
          return;

       name = buddy->markname?:buddy->nick?:buddy->qqnumber;
    }
    GPtrArray *aliases;

    GArray* pair = g_array_sized_new(FALSE, TRUE, 2, 2);
    g_array_append_val(pair, contact);
    g_array_append_val(pair, name);

    aliases = g_ptr_array_sized_new (1);
    g_ptr_array_add (aliases, pair);

    tp_svc_connection_interface_aliasing_emit_aliases_changed (conn, aliases);

    g_ptr_array_free (aliases, TRUE);
    g_array_free(pair, FALSE);
}

void
aliasing_init(GObject *object)
{
    g_signal_connect (object,
            "alias-updated", (GCallback) alias_updated_cb, NULL);
    tp_contacts_mixin_add_contact_attributes_iface (object,
            TP_IFACE_CONNECTION_INTERFACE_ALIASING,
            aliasing_fill_contact_attributes);
}
