#include "avatar.h"

#define AVATAR_MIN_PX 64
#define AVATAR_REC_PX 96
#define AVATAR_MAX_PX 120
#define AVATAR_MAX_BYTES 16384

static const char *mimetypes[] = {
    "image/png", "image/jpeg", "image/gif", NULL 
};

static TpDBusPropertiesMixinPropImpl props[] = {
    { "MinimumAvatarWidth"      ,GUINT_TO_POINTER (AVATAR_MIN_PX)    ,NULL }, 
    { "RecommendedAvatarWidth"  ,GUINT_TO_POINTER (AVATAR_REC_PX)    ,NULL }, 
    { "MaximumAvatarWidth"      ,GUINT_TO_POINTER (AVATAR_MAX_PX)    ,NULL }, 
    { "MinimumAvatarHeight"     ,GUINT_TO_POINTER (AVATAR_MIN_PX)    ,NULL }, 
    { "RecommendedAvatarHeight" ,GUINT_TO_POINTER (AVATAR_REC_PX)    ,NULL }, 
    { "MaximumAvatarHeight"     ,GUINT_TO_POINTER (AVATAR_MAX_PX)    ,NULL }, 
    { "MaximumAvatarBytes"      ,GUINT_TO_POINTER (AVATAR_MAX_BYTES) ,NULL }, 
    /* special-cased - it's the only one with a non-guint value */
    { "SupportedAvatarMIMETypes", NULL, NULL },
    { NULL }
};

TpDBusPropertiesMixinPropImpl *avatars_properties = props;

static void
avatars_get_avatar_requirements (TpSvcConnectionInterfaceAvatars *iface,
        DBusGMethodInvocation *context)
{

    TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED (TP_BASE_CONNECTION (iface),
            context);

    tp_svc_connection_interface_avatars_return_from_get_avatar_requirements (
            context, mimetypes, AVATAR_MIN_PX, AVATAR_MIN_PX,
            AVATAR_MAX_PX, AVATAR_MAX_PX, AVATAR_MAX_BYTES);
}

static void
avatars_get_avatar_tokens (TpSvcConnectionInterfaceAvatars *iface,
      const GArray *contacts, DBusGMethodInvocation *invocation)
{
   LwqqConnection *self =
      LWQQ_CONNECTION (iface);
   TpBaseConnection *base = (TpBaseConnection *) self;
   LwqqClient* lc = self->lc;
   TpHandleRepoIface *contacts_repo = tp_base_connection_get_handles(base,
         TP_HANDLE_TYPE_CONTACT);

   guint i ;
   gchar **ret;
   GError *err = NULL;

   TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED (base, invocation);

   if (!tp_handles_are_valid (contacts_repo, contacts, FALSE, &err))
   {
      dbus_g_method_return_error (invocation, err);
      g_error_free (err);
      return;
   }

   ret = g_new0 (gchar *, contacts->len + 1);

   for (i = 0; i < contacts->len; i++)
   {
      TpHandle contact = g_array_index (contacts, TpHandle, i);
      if(contact == base->self_handle) {
         ret[i] = g_strdup(lc->myself->qqnumber);
      }else {
         const char* qq = tp_handle_inspect(contacts_repo, contact);
         ret[i] = g_strdup(qq);
      }
   }

   tp_svc_connection_interface_avatars_return_from_get_avatar_tokens (
         invocation, (const gchar **)ret);
   g_strfreev (ret);
}

static void
avatars_get_known_avatar_tokens (TpSvcConnectionInterfaceAvatars *iface,
      const GArray *contacts, DBusGMethodInvocation *invocation)
{
   LwqqConnection *self =
      LWQQ_CONNECTION (iface);
   TpBaseConnection *base = (TpBaseConnection *) self;
   LwqqClient* lc = self->lc;

   TpHandleRepoIface *contacts_repo = tp_base_connection_get_handles(base,
         TP_HANDLE_TYPE_CONTACT);

   guint i;
   GHashTable *ret;
   GError *err = NULL;

   TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED (base, invocation);

   if (!tp_handles_are_valid (contacts_repo, contacts, FALSE, &err))
   {
      dbus_g_method_return_error (invocation, err);
      g_error_free (err);
      return;
   }

   ret = g_hash_table_new_full (NULL, NULL, NULL, g_free);

   for (i = 0; i < contacts->len; i++)
   {
      TpHandle contact;

      contact = g_array_index (contacts, TpHandle, i);

      if(contact != base->self_handle) {
         const char* qq = tp_handle_inspect(contacts_repo, contact);
         g_hash_table_insert (ret, GUINT_TO_POINTER (contact),
               g_strdup (qq));
      }else{
         g_hash_table_insert (ret, GUINT_TO_POINTER (contact),
               g_strdup (lc->myself->qqnumber));
      }
   }

   tp_svc_connection_interface_avatars_return_from_get_known_avatar_tokens (
         invocation, ret);

   g_hash_table_destroy (ret);
}

static GArray* get_avatar(LwqqClient* lc, LwqqBuddy* buddy)
{
   if(buddy->avatar_len == 0){
      LWQQ_SYNC_BEGIN(lc);
      lwqq_info_get_friend_avatar(lc, buddy);
      LWQQ_SYNC_END(lc);
   }

   GArray* arr = g_array_new (FALSE, FALSE, sizeof (gchar));
   g_array_append_vals (arr, buddy->avatar, buddy->avatar_len);
   return arr;
}

static void
avatars_request_avatar (TpSvcConnectionInterfaceAvatars *iface,
      guint contact, DBusGMethodInvocation *context)
{
   LwqqConnection *self =
      LWQQ_CONNECTION (iface);
   TpBaseConnection *base = (TpBaseConnection *) self;
   LwqqClient* lc = self->lc;
   LwqqBuddy* buddy = NULL;

   TpHandleRepoIface *contacts_repo = tp_base_connection_get_handles(base,
         TP_HANDLE_TYPE_CONTACT);

   GError *err = NULL;

   TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED (base, context);

   if (!tp_handle_is_valid (contacts_repo, contact, &err))
   {
      dbus_g_method_return_error (context, err);
      g_error_free (err);
      return;
   }

   if(contact != base->self_handle) {
      const char* qq = tp_handle_inspect(contacts_repo, contact);
      buddy = lwqq_buddy_find_buddy_by_qqnumber(lc, qq);
   }else{
      buddy = lc->myself;
   }

   if(buddy == NULL)
      return;

   GArray *arr = get_avatar(lc, buddy);
   tp_svc_connection_interface_avatars_return_from_request_avatar (
         context, arr, "");
   g_array_free (arr, TRUE);
}

static void
avatars_request_avatars (TpSvcConnectionInterfaceAvatars *iface,
        const GArray *contacts, DBusGMethodInvocation *context)
{
    LwqqConnection *self =
        LWQQ_CONNECTION (iface);
    TpBaseConnection *base = (TpBaseConnection *) self;
    LwqqClient* lc = self->lc;

   TpHandleRepoIface *contacts_repo = tp_base_connection_get_handles(base,
         TP_HANDLE_TYPE_CONTACT);

    GError *error = NULL;
    guint i;

    TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED (base, context);

    if (!tp_handles_are_valid (contacts_repo, contacts, FALSE, &error))
    {
        dbus_g_method_return_error (context, error);
        g_error_free (error);
        return;
    }

    for (i = 0; i < contacts->len; i++)
    {
        TpHandle contact = g_array_index (contacts, TpHandle, i);
        const char* qq = NULL;
        LwqqBuddy* buddy = NULL;

        if(contact != base->self_handle) {
           qq = tp_handle_inspect(contacts_repo, contact);
           buddy = lwqq_buddy_find_buddy_by_qqnumber(lc, qq);
        }else{
           qq = lc->myself->qqnumber;
           buddy = lc->myself;
        }

        if(buddy == NULL)
           return;

        GArray* arr = get_avatar(lc, buddy);
        tp_svc_connection_interface_avatars_emit_avatar_retrieved (iface, contact,
              qq, arr, mimetypes[1]);
        g_array_free (arr, TRUE);
    }

    tp_svc_connection_interface_avatars_return_from_request_avatars (context);
}

void
avatars_fill_contact_attributes (GObject *obj,
      const GArray *contacts, GHashTable *attributes_hash)
{
   guint i;

   LwqqConnection *self =
      LWQQ_CONNECTION (obj);
   TpBaseConnection *base = (TpBaseConnection *) self;
   LwqqClient* lc = self->lc;
   TpHandleRepoIface *contacts_repo = tp_base_connection_get_handles(base,
         TP_HANDLE_TYPE_CONTACT);

   for (i = 0; i < contacts->len; i++)
   {
      TpHandle contact = g_array_index (contacts, guint, i);
      GValue *val = tp_g_value_slice_new (G_TYPE_STRING);
      const char* qq = NULL;

      if(contact != base->self_handle) {
         qq = tp_handle_inspect(contacts_repo, contact);
      }else{
         qq = lc->myself->qqnumber;
      }
      g_value_set_string (val, qq);
      tp_contacts_mixin_set_contact_attribute (attributes_hash, contact,
            TP_IFACE_CONNECTION_INTERFACE_AVATARS"/token", val);
   }
}

void
avatars_properties_getter (GObject *object,
      GQuark interface, GQuark name,
      GValue *value, gpointer getter_data)
{
   GQuark q_mime_types = g_quark_from_static_string (
         "SupportedAvatarMIMETypes");

   if (name == q_mime_types)
   {
      g_value_set_static_boxed (value, mimetypes);
   }
   else
   {
      g_value_set_uint (value, GPOINTER_TO_UINT (getter_data));
   }
}


void
avatars_iface_init (gpointer g_iface, gpointer iface_data)
{
   TpSvcConnectionInterfaceAvatarsClass *klass = g_iface;

#define IMPLEMENT(x) tp_svc_connection_interface_avatars_implement_##x (\
      klass, avatars_##x)
   IMPLEMENT(get_avatar_requirements);
   IMPLEMENT(get_avatar_tokens);
   IMPLEMENT(get_known_avatar_tokens);
   IMPLEMENT(request_avatar);
   IMPLEMENT(request_avatars);
#undef IMPLEMENT
}

static void
avatar_updated_cb( LwqqConnection *conn,
      TpHandle contact,const gchar *shal, gpointer user_data)
{
   g_assert(shal != NULL);
   tp_svc_connection_interface_avatars_emit_avatar_updated(conn,contact,shal);
}

void
avatars_init(GObject *object)
{
    g_signal_connect (object,
            "lwqq-avatar-updated", (GCallback) avatar_updated_cb, NULL);
    tp_contacts_mixin_add_contact_attributes_iface (object,
            TP_IFACE_CONNECTION_INTERFACE_AVATARS,
            avatars_fill_contact_attributes);
}
