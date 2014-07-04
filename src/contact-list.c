#include "contact-list.h"
#include "connection.h"

#include <lwqq/lwqq.h>


struct _LwqqContactListPrivate {
    LwqqConnection *conn;
    int status_changed_id;

    TpHandleRepoIface* contact_repo;

    TpHandleSet* contacts;

    /* Maps TpHandle to PublishRequestData, corresponding to the handles on
     * publish's local_pending.
     */
    GHashTable *pending_publish_requests;

    /* Contacts whose publish requests we've accepted or declined. */
    TpHandleSet *publishing_to;
    TpHandleSet *not_publishing_to;

};



static const TpPresenceStatusSpec _statuses[] = {
      { "logout", TP_CONNECTION_PRESENCE_TYPE_UNSET, FALSE, NULL },
      { "available", TP_CONNECTION_PRESENCE_TYPE_AVAILABLE, TRUE, NULL },
      { "offline", TP_CONNECTION_PRESENCE_TYPE_OFFLINE, FALSE, NULL },
      { "away", TP_CONNECTION_PRESENCE_TYPE_AWAY, TRUE, NULL },
      { "hidden", TP_CONNECTION_PRESENCE_TYPE_HIDDEN, FALSE, NULL },
      { "busy", TP_CONNECTION_PRESENCE_TYPE_BUSY, FALSE, NULL },
      { "callme", TP_CONNECTION_PRESENCE_TYPE_AVAILABLE, FALSE, NULL },
      { "slient", TP_CONNECTION_PRESENCE_TYPE_BUSY, FALSE, NULL },
      { NULL }
};

const TpPresenceStatusSpec* lwqq_contact_list_presence_statuses()
{
    return _statuses;
}

static void contact_list_mutable_init (TpMutableContactListInterface *iface);

G_DEFINE_TYPE_WITH_CODE(LwqqContactList,
    lwqq_contact_list,
    TP_TYPE_BASE_CONTACT_LIST,
    G_IMPLEMENT_INTERFACE (TP_TYPE_MUTABLE_CONTACT_LIST,
      contact_list_mutable_init)
    /*G_IMPLEMENT_INTERFACE (TP_TYPE_CONTACT_GROUP_LIST,
      contact_list_groups_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_MUTABLE_CONTACT_GROUP_LIST,
      contact_list_mutable_groups_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_BLOCKABLE_CONTACT_LIST,
      lwqq_contact_list_blockable_init)*/
      )

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

    while (tp_intset_fast_iter_next (&iter, &handle)){
        lwqq_contact_list_request_subscription (self, handle, message);
    }

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

void
contact_list_remove_contact (LwqqContactList *self,
    TpHandle handle)
{
#if 0
  PurpleAccount *account = self->priv->conn->account;
  const gchar *bname = haze_connection_handle_inspect (self->priv->conn,
      TP_HANDLE_TYPE_CONTACT, handle);
  GSList *buddies, *l;

  buddies = purple_find_buddies (account, bname);
  /* buddies may be NULL, but that's a perfectly reasonable GSList */

  /* Removing a buddy from subscribe entails removing it from all
   * groups since you can't have a buddy without groups in libpurple.
   */
  for (l = buddies; l != NULL; l = l->next)
    {
      PurpleBuddy *buddy = (PurpleBuddy *) l->data;
      PurpleGroup *group = purple_buddy_get_group (buddy);

      purple_account_remove_buddy (account, buddy, group);
      purple_blist_remove_buddy (buddy);
    }

  /* Also decline any publication requests we might have had */
  haze_contact_list_reject_publish_request (self, handle);

  g_slist_free (buddies);
#endif
}

static void 
contact_list_remove_contacts_async(TpBaseContactList* cl,TpHandleSet*
        contacts,GAsyncReadyCallback callback,gpointer data)
{

  LwqqContactList *self = LWQQ_CONTACT_LIST (cl);
  TpIntsetFastIter iter;
  TpHandle handle;

  tp_intset_fast_iter_init (&iter, tp_handle_set_peek (contacts));

  while (tp_intset_fast_iter_next (&iter, &handle))
    contact_list_remove_contact (self, handle);

  tp_simple_async_report_success_in_idle ((GObject *) self, callback,
      data, contact_list_remove_contacts_async);
}
static void
contact_list_store_contacts_async (TpBaseContactList *cl,
    TpHandleSet *contacts,
    GAsyncReadyCallback callback,
    gpointer user_data)
{
   LwqqContactList* self = LWQQ_CONTACT_LIST(cl);
   tp_simple_async_report_success_in_idle((GObject*)self, callback, user_data,
         contact_list_store_contacts_async);
}
static void
contact_list_mutable_init (TpMutableContactListInterface *iface)
{
    iface->can_change_contact_list = tp_base_contact_list_true_func;
    iface->get_request_uses_message = tp_base_contact_list_true_func;

    /* we use the default _finish functions, which assume a GSimpleAsyncResult */
    iface->request_subscription_async  = contact_list_request_subscription_async;
    iface->authorize_publication_async = contact_list_authorize_publication_async;
    iface->store_contacts_async        = contact_list_store_contacts_async;
    iface->remove_contacts_async       = contact_list_remove_contacts_async;

    iface->unsubscribe_async           = contact_list_remove_contacts_async;
    iface->unpublish_async             = contact_list_remove_contacts_async;
    /* this is about the best we can do for unsubscribe/unpublish */
    //vtable->store_contacts_async = lwqq_contact_list_store_contacts_async;
    /* assume defaults: can change the contact list, and requests use the
     * message */
}

static void 
received_all_info(LwqqClient* lc)
{
   TpHandle handle;
   LwqqConnection* conn = lc->data;
   TpHandleRepoIface* contact_repo = 
      tp_base_connection_get_handles(TP_BASE_CONNECTION(conn), TP_HANDLE_TYPE_CONTACT);

   tp_base_contact_list_set_list_received(TP_BASE_CONTACT_LIST(conn->contact_list));

   LwqqBuddy* buddy;
   LwqqGroup* group;
   LwdbUserDB* db = lwqq_connection_get_db(conn);
   lwdb_userdb_begin(db);
   LIST_FOREACH(buddy,&lc->friends,entries) {
      if(buddy->last_modify == -1 || buddy->last_modify == 0)
         lwdb_userdb_insert_buddy_info(db, &buddy);
      handle = tp_handle_ensure(contact_repo, buddy->uin, NULL, NULL);
      //tp_base_contact_list_one_contact_changed(&self->parent, handle);
      g_message("test:%s\n", buddy->uin);
      //g_signal_emit_by_name(conn, "presence-update", 1, handle);
   }
   LIST_FOREACH(group,&lc->groups,entries){
      if(group->last_modify == -1 || group->last_modify == 0)
         lwdb_userdb_insert_group_info(db, &group);
   }
   lwdb_userdb_commit(db);

   // wait all download finished, start msg pool, to get all qqnumber
   lwqq_msglist_poll(lc->msg_list, 0);
}

static void 
request_detaile_info(LwqqClient* lc)
{
   LwqqConnection* conn = lc->data;
   LwdbUserDB* db = lwqq_connection_get_db(conn);

	lwdb_userdb_flush_buddies(db, 5, 5);
	lwdb_userdb_flush_groups(db, 1, 10);

   lwdb_userdb_query_qqnumbers(db,lc);

   LwqqAsyncEvent* ev;
	LwqqAsyncEvset* set = lwqq_async_evset_new();

	LwqqBuddy* buddy;
	LIST_FOREACH(buddy,&lc->friends,entries) {
		lwdb_userdb_query_buddy(db, buddy);
		if(!buddy->qqnumber){
			ev = lwqq_info_get_friend_qqnumber(lc,buddy);
			lwqq_async_evset_add_event(set, ev);
		}
		if(buddy->last_modify == 0 || buddy->last_modify == -1) {
			ev = lwqq_info_get_single_long_nick(lc, buddy);
			lwqq_async_evset_add_event(set, ev);
			ev = lwqq_info_get_level(lc, buddy);
			lwqq_async_evset_add_event(set, ev);
			//if buddy is unknow we should update avatar in friend_come
			//for better speed in first load
			if(buddy->last_modify == LWQQ_LAST_MODIFY_RESET){
				ev = lwqq_info_get_friend_avatar(lc,buddy);
				lwqq_async_evset_add_event(set, ev);
			}
		}
	}

	LwqqGroup* group;
	LIST_FOREACH(group,&lc->groups,entries) {
		//LwqqAsyncEvset* set = NULL;
		lwdb_userdb_query_group(db, group);
		if(!group->account){
			ev = lwqq_info_get_group_qqnumber(lc,group);
			lwqq_async_evset_add_event(set, ev);
		}
		if(group->last_modify == -1 || group->last_modify == 0){
			ev = lwqq_info_get_group_memo(lc, group);
			lwqq_async_evset_add_event(set, ev);
		}
	}

	LwqqGroup* discu;
	LIST_FOREACH(discu,&lc->discus,entries){
		if(discu->last_modify == LWQQ_LAST_MODIFY_UNKNOW)
			// discu is imediately date, doesn't need get info from server, we can
			// directly write it into database
			lwdb_userdb_insert_discu_info(db, &discu);
	}

	lwqq_async_add_evset_listener(set, _C_(p,received_all_info,lc));
}

static void 
request_other_list(LwqqAsyncEvent* event,LwqqClient* lc)
{
   if(!lwqq_client_valid(lc)) return;
   LwqqConnection* conn = lc->data;
	if(event->result != LWQQ_EC_OK){
      tp_base_connection_change_status(TP_BASE_CONNECTION(conn), TP_CONNECTION_STATUS_DISCONNECTED, TP_CONNECTION_STATUS_REASON_NETWORK_ERROR);
		/*
		qq_account* ac = lc->data;
      purple_connection_error_reason(ac->gc, 
				PURPLE_CONNECTION_ERROR_NETWORK_ERROR, 
				_("Get Group List Failed"));*/
		return;
	}
	LwqqAsyncEvset* set = lwqq_async_evset_new();
	LwqqAsyncEvent* ev;
	ev = lwqq_info_get_discu_name_list(lc);
	lwqq_async_evset_add_event(set,ev);
	ev = lwqq_info_get_online_buddies(lc,NULL);
	lwqq_async_evset_add_event(set,ev);
   ev = lwqq_info_get_friend_detail_info(lc,lc->myself);
   lwqq_async_evset_add_event(set,ev);

	lwqq_async_add_evset_listener(set,_C_(p,request_detaile_info,lc));
}

static void 
request_group_list(LwqqClient* lc)
{
   if(!lwqq_client_valid(lc)) return;
   LwqqConnection* conn = lc->data;
   LwdbUserDB* db = lwqq_connection_get_db(conn);

   const LwqqHashEntry* succ_hash = lwqq_hash_get_last(lc);
   lwdb_userdb_write(db, "last_hash", succ_hash->name);
   LwqqAsyncEvent* ev;
   ev = lwqq_info_get_group_name_list(lc, succ_hash->func, succ_hash->data);
   lwqq_async_add_event_listener(ev, _C_(2p, request_other_list, ev, lc));
}

static void
request_friend_list(LwqqClient* lc,LwqqErrorCode err)
{
    if(!lwqq_client_valid(lc)) return;
    LwqqConnection* conn = lc->data;

    LwqqAsyncEvent* ev = lwqq_info_get_friends_info(lc, NULL, NULL);
    lwqq_async_add_event_listener(ev, _C_(p,request_group_list,lc));
}

static void
status_changed_cb (TpBaseConnection *conn,
    guint status,
    guint reason,
    LwqqContactList *self)
{
    switch(status){
        case TP_CONNECTION_STATUS_CONNECTED:
            tp_base_contact_list_set_list_pending(TP_BASE_CONTACT_LIST(self));
            request_friend_list(self->priv->conn->lc, 0);
            break;
        case TP_CONNECTION_STATUS_DISCONNECTED:
            break;
    }
}

static void contact_list_constructed(GObject* obj)
{
    LwqqContactList *self = LWQQ_CONTACT_LIST(obj);
    void (*chainup)(GObject* obj) = ((GObjectClass*)lwqq_contact_list_parent_class)->constructed;

    if(chainup)chainup(obj);

    self = LWQQ_CONTACT_LIST (obj);

    self->priv->conn = LWQQ_CONNECTION (tp_base_contact_list_get_connection (
        (TpBaseContactList *) self, NULL));
    g_assert (self->priv->conn != NULL);
    /* not reffed, for the moment */

    self->priv->contact_repo = tp_base_connection_get_handles (
        (TpBaseConnection *) self->priv->conn, TP_HANDLE_TYPE_CONTACT);
    self->priv->contacts = tp_handle_set_new(self->priv->contact_repo);

    //self->priv->publishing_to = tp_handle_set_new (contact_repo);
    //self->priv->not_publishing_to = tp_handle_set_new (contact_repo);

//    self->priv->pending_publish_requests = g_hash_table_new_full (NULL, NULL,
//        NULL, (GDestroyNotify) publish_request_data_free);
    self->priv->status_changed_id = g_signal_connect(self->priv->conn,
            "status-changed", (GCallback)status_changed_cb, self);
}

static void
contact_list_dispose (GObject *object)
{
    LwqqContactList *self = LWQQ_CONTACT_LIST (object);
    LwqqContactListPrivate *priv = self->priv;

    tp_clear_pointer(&priv->contacts, tp_handle_set_destroy);

    G_OBJECT_CLASS(lwqq_contact_list_parent_class)->dispose(object);
}


static void
lwqq_contact_list_finalize (GObject *object)
{
    G_OBJECT_CLASS (lwqq_contact_list_parent_class)->finalize (object);
}

static TpHandleSet* contact_list_dup_contacts(TpBaseContactList* base)
{
    LwqqContactList *self = LWQQ_CONTACT_LIST (base);
    TpBaseConnection *base_conn = TP_BASE_CONNECTION (self->priv->conn);
    TpHandleRepoIface* contact_repo = self->priv->contact_repo;
    LwqqClient* lc = self->priv->conn->lc;
    /* The list initially contains anyone we're definitely publishing to.
     * Because libpurple, that's only people whose request we accepted during
     * this session :-( */
    TpHandleSet *handles = tp_handle_set_new(contact_repo);
    /* Also include anyone on our buddy list */
    LwqqBuddy* buddy;
    LIST_FOREACH(buddy, &lc->friends, entries){
        TpHandle handle = tp_handle_ensure (contact_repo,
                buddy->uin, NULL, NULL);

        if (G_LIKELY (handle != 0)){
            g_message("Add Contact %s\n",buddy->uin);
            tp_handle_set_add (handles, handle);
        }
    }

    /* Also include anyone with an outstanding request */
#if 0
    GHashTableIter hash_iter;
    gpointer k;
    g_hash_table_iter_init (&hash_iter, self->priv->pending_publish_requests);

    while (g_hash_table_iter_next (&hash_iter, &k, NULL))
    {
        tp_handle_set_add (handles, GPOINTER_TO_UINT (k));
    }
#endif

    return handles;
}


static void
contact_list_dup_states (TpBaseContactList *cl,
    TpHandle contact,
    TpSubscriptionState *subscribe_out,
    TpSubscriptionState *publish_out,
    gchar **publish_request_out)
{
    LwqqContactList *self = LWQQ_CONTACT_LIST (cl);

    g_message("Add State %s\n",tp_handle_inspect(self->priv->contact_repo, contact));
    if(subscribe_out)*subscribe_out = TP_SUBSCRIPTION_STATE_YES;
    if(publish_out)*publish_out = TP_SUBSCRIPTION_STATE_YES;
    if(publish_request_out) *publish_request_out = g_strdup("");
#if 0
    LwqqContactList *self = LWQQ_CONTACT_LIST (cl);
    const gchar *bname = lwqq_connection_handle_inspect (self->priv->conn,
            TP_HANDLE_TYPE_CONTACT, contact);
    LwqqClient* lc = self->priv->conn->lc;
    LwqqBuddy* buddy = lc->find_buddy_by_uin(lc,bname);
    TpSubscriptionState pub = TP_SUBSCRIPTION_STATE_YES, sub;
    /*PublishRequestData *pub_req = g_hash_table_lookup (
      self->priv->pending_publish_requests, GUINT_TO_POINTER (contact));
      */

    if (publish_request_out != NULL)
        *publish_request_out = NULL;

    if (buddy != NULL)
    {
        /* Well, it's on the contact list. Are we subscribed to its presence?
         * Who knows? Let's assume we are. */
        sub = TP_SUBSCRIPTION_STATE_YES;
    }
    else
    {
        /* We're definitely not subscribed. */
        sub = TP_SUBSCRIPTION_STATE_NO;
    }

    /*if (pub_req != NULL)
      {
      pub = TP_SUBSCRIPTION_STATE_ASK;

      if (publish_request_out != NULL)
     *publish_request_out = g_strdup (pub_req->message);
     }
     else if (tp_handle_set_is_member (self->priv->publishing_to, contact))
     {
     pub = TP_SUBSCRIPTION_STATE_YES;
     }
     else if (tp_handle_set_is_member (self->priv->not_publishing_to, contact))
     {
     pub = TP_SUBSCRIPTION_STATE_NO;
     }
     else
     {
     pub = TP_SUBSCRIPTION_STATE_UNKNOWN;
     }
     */

    if (subscribe_out != NULL)
        *subscribe_out = sub;

    if (publish_out != NULL)
        *publish_out = pub;
#endif
}

static void
lwqq_contact_list_class_init (LwqqContactListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    TpBaseContactListClass *parent_class = TP_BASE_CONTACT_LIST_CLASS (klass);

    object_class->constructed = contact_list_constructed;

    object_class->dispose = contact_list_dispose;
    object_class->finalize = lwqq_contact_list_finalize;

    parent_class->dup_contacts = contact_list_dup_contacts;
    parent_class->dup_states = contact_list_dup_states;
    //parent_class->get_contact_list_persists = tp_base_contact_list_true_func;

    g_type_class_add_private (object_class,
                              sizeof(LwqqContactListPrivate));
}


static void
lwqq_contact_list_init (LwqqContactList *self)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, LWQQ_TYPE_CONTACT_LIST, LwqqContactListPrivate);
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
    LwqqFriendCategory* cate = NULL;
    cate = lwqq_category_find_by_id(lc, buddy->cate_index);
    const char *group_name = cate->name;

    tp_base_contact_list_one_contact_changed (
        (TpBaseContactList *) contact_list, handle);

    tp_base_contact_list_one_contact_groups_changed (
        (TpBaseContactList *) contact_list, handle, &group_name, 1, NULL, 0);
}

guint lwqq_contact_list_get_presence(LwqqContactList* self,TpHandle contact)
{
    LwqqClient* lc = self->priv->conn->lc;
    const char* uin = tp_handle_inspect(self->priv->contact_repo, contact);
    LwqqBuddy* b = lwqq_buddy_find_buddy_by_uin(lc, uin);
    if(b) return to_presence(b->stat);
    else return 0;
}
