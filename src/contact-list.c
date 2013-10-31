#include "contact-list.h"
#include "connection.h"

#include <lwqq/lwqq.h>


struct _LwqqContactListPrivate {
    LwqqConnection *conn;

    /* Maps TpHandle to PublishRequestData, corresponding to the handles on
     * publish's local_pending.
     */
    GHashTable *pending_publish_requests;

    /* Contacts whose publish requests we've accepted or declined. */
    TpHandleSet *publishing_to;
    TpHandleSet *not_publishing_to;

};

void
lwqq_contact_list_request_subscription (LwqqContactList *self,
    TpHandle handle,
    const gchar *message)
{
  /*const gchar *bname = lwqq_connection_handle_inspect (self->priv->conn,
      TP_HANDLE_TYPE_CONTACT, handle);
  PurpleBuddy *buddy;*/

}
static void
contact_list_request_subscription_async (TpBaseContactList *cl,
    TpHandleSet *contacts,
    const gchar *message,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  LwqqContactList *self = LWQQ_CONTACT_LIST (cl);
  TpIntsetFastIter iter;
  TpHandle handle;

  tp_intset_fast_iter_init (&iter, tp_handle_set_peek (contacts));

  while (tp_intset_fast_iter_next (&iter, &handle))
    lwqq_contact_list_request_subscription (self, handle, message);

  tp_simple_async_report_success_in_idle ((GObject *) self, callback,
      user_data, contact_list_request_subscription_async);
}

void
lwqq_contact_list_accept_publish_request (LwqqContactList *self,
    TpHandle handle)
{
  gpointer key = GUINT_TO_POINTER (handle);
  /*PublishRequestData *request_data = g_hash_table_lookup (
      self->priv->pending_publish_requests, key);
  const gchar *bname = lwqq_connection_handle_inspect (self->priv->conn,
      TP_HANDLE_TYPE_CONTACT, handle);

  if (request_data == NULL)
    return;

  DEBUG ("allowing publish request for %s", bname);
  request_data->allow(request_data->data);

  tp_handle_set_add (self->priv->publishing_to, handle);
  remove_pending_publish_request (self, handle);

  tp_base_contact_list_one_contact_changed ((TpBaseContactList *) self,
      handle);
      */
}
static void
contact_list_authorize_publication_async (TpBaseContactList *cl,
    TpHandleSet *contacts,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
  LwqqContactList *self = LWQQ_CONTACT_LIST (cl);
  TpIntsetFastIter iter;
  TpHandle handle;

  tp_intset_fast_iter_init (&iter, tp_handle_set_peek (contacts));

  while (tp_intset_fast_iter_next (&iter, &handle))
    lwqq_contact_list_accept_publish_request (self, handle);

  tp_simple_async_report_success_in_idle ((GObject *) self, callback,
      user_data, contact_list_authorize_publication_async);
}

static void
contact_list_mutable_init (TpMutableContactListInterface *vtable)
{
  /* we use the default _finish functions, which assume a GSimpleAsyncResult */
  vtable->request_subscription_async =
    contact_list_request_subscription_async;
  vtable->authorize_publication_async =
    contact_list_authorize_publication_async;
  //vtable->remove_contacts_async = lwqq_contact_list_remove_contacts_async;
  /* this is about the best we can do for unsubscribe/unpublish */
  //vtable->unsubscribe_async = lwqq_contact_list_remove_contacts_async;
  //vtable->unpublish_async = lwqq_contact_list_remove_contacts_async;
  //vtable->store_contacts_async = lwqq_contact_list_store_contacts_async;
  /* assume defaults: can change the contact list, and requests use the
   * message */
}

G_DEFINE_TYPE_WITH_CODE(LwqqContactList,
    lwqq_contact_list,
    TP_TYPE_BASE_CONTACT_LIST,
    G_IMPLEMENT_INTERFACE (TP_TYPE_MUTABLE_CONTACT_LIST,
      contact_list_mutable_init)
    /*G_IMPLEMENT_INTERFACE (TP_TYPE_CONTACT_GROUP_LIST,
      lwqq_contact_list_groups_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_MUTABLE_CONTACT_GROUP_LIST,
      lwqq_contact_list_mutable_groups_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_BLOCKABLE_CONTACT_LIST,
      lwqq_contact_list_blockable_init)*/
      )

static GObject *
lwqq_contact_list_constructor (GType type, guint n_props,
                               GObjectConstructParam *props)
{
    GObject *obj;
    LwqqContactList *self;
    TpHandleRepoIface *contact_repo;

    obj = G_OBJECT_CLASS (lwqq_contact_list_parent_class)->
        constructor (type, n_props, props);

    self = LWQQ_CONTACT_LIST (obj);

    self->priv->conn = LWQQ_CONNECTION (tp_base_contact_list_get_connection (
        (TpBaseContactList *) self, NULL));
    g_assert (self->priv->conn != NULL);
    /* not reffed, for the moment */

    contact_repo = tp_base_connection_get_handles (
        (TpBaseConnection *) self->priv->conn, TP_HANDLE_TYPE_CONTACT);

    self->priv->publishing_to = tp_handle_set_new (contact_repo);
    self->priv->not_publishing_to = tp_handle_set_new (contact_repo);

//    self->priv->pending_publish_requests = g_hash_table_new_full (NULL, NULL,
//        NULL, (GDestroyNotify) publish_request_data_free);

    return obj;
}

static void
lwqq_contact_list_dispose (GObject *object)
{
    LwqqContactList *self = LWQQ_CONTACT_LIST (object);
    LwqqContactListPrivate *priv = self->priv;

    tp_clear_pointer (&priv->publishing_to, tp_handle_set_destroy);
    tp_clear_pointer (&priv->not_publishing_to, tp_handle_set_destroy);

    if (priv->pending_publish_requests)
    {
        g_assert (g_hash_table_size (priv->pending_publish_requests) == 0);
        g_hash_table_destroy (priv->pending_publish_requests);
        priv->pending_publish_requests = NULL;
    }

    if (G_OBJECT_CLASS (lwqq_contact_list_parent_class)->dispose)
        G_OBJECT_CLASS (lwqq_contact_list_parent_class)->dispose (object);
}


static void
lwqq_contact_list_finalize (GObject *object)
{
    G_OBJECT_CLASS (lwqq_contact_list_parent_class)->finalize (object);
}

static void
lwqq_contact_list_class_init (LwqqContactListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    TpBaseContactListClass *parent_class = TP_BASE_CONTACT_LIST_CLASS (klass);

    object_class->constructor = lwqq_contact_list_constructor;

    object_class->dispose = lwqq_contact_list_dispose;
    object_class->finalize = lwqq_contact_list_finalize;

    //parent_class->dup_contacts = lwqq_contact_list_dup_contacts;
    //parent_class->dup_states = lwqq_contact_list_dup_states;
    /* we assume the contact list does persist, which is the default */

    g_type_class_add_private (object_class,
                              sizeof(LwqqContactListPrivate));

    /*purple_signal_connect (purple_blist_get_handle(), "buddy-added",
                           klass, PURPLE_CALLBACK(buddy_added_cb), NULL);
    purple_signal_connect (purple_blist_get_handle(), "buddy-removed",
                           klass, PURPLE_CALLBACK(buddy_removed_cb), NULL);
    */
}


static void
lwqq_contact_list_init (LwqqContactList *self)
{
    LwqqContactListPrivate *priv =
        (G_TYPE_INSTANCE_GET_PRIVATE((self), LWQQ_TYPE_CONTACT_LIST,
                                     LwqqContactListPrivate));

    self->priv = priv;
}



void
lwqq_contact_list_add_buddy(LwqqClient* lc,LwqqBuddy* buddy)
{
    LwqqConnection *conn = lc->data;
    LwqqContactList *contact_list = conn->contact_list;
    TpBaseConnection *base_conn = TP_BASE_CONNECTION (conn);
    TpHandleRepoIface *contact_repo = tp_base_connection_get_handles (
        base_conn, TP_HANDLE_TYPE_CONTACT);
    const char* name = buddy->uin;
    TpHandle handle = tp_handle_ensure (contact_repo, name, NULL, NULL);
    LwqqFriendCategory* cate = lwqq_category_find_by_id(lc, buddy->cate_index);
    const char *group_name = cate->name;

    tp_base_contact_list_one_contact_changed (
        (TpBaseContactList *) contact_list, handle);

    tp_base_contact_list_one_contact_groups_changed (
        (TpBaseContactList *) contact_list, handle, &group_name, 1, NULL, 0);
}
