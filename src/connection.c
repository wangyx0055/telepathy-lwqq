/*
 * This file is part of telepathy-lwqq
 *
 * Copyright (C) 2006-2007 Collabora Limited
 * Copyright (C) 2006-2007 Nokia Corporation
 * Copyright (C) 2011      Debarshi Ray <rishi@gnu.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"

#include "connection.h"
#include "im-manager.h"
#include "contact-list.h"
#include "presence.h"
#include "aliasing.h"
#include "avatar.h"

#include <string.h>
#include <time.h>

#include <dbus/dbus-glib.h>

#include <telepathy-glib/dbus.h>
#include <telepathy-glib/enums.h>
#include <telepathy-glib/errors.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/simple-password-manager.h>
#include <telepathy-glib/handle-repo-dynamic.h>
#include <telepathy-glib/svc-connection.h>
#include <telepathy-glib/channel-manager.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/util.h>

#include <lwqq/lwqq.h>
#include <lwqq/lwdb.h>

G_DEFINE_TYPE_WITH_CODE(LwqqConnection,
    lwqq_connection,
    TP_TYPE_BASE_CONNECTION,
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CONNECTION_INTERFACE_ALIASING,
       aliasing_iface_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CONNECTION_INTERFACE_AVATARS,
       avatars_iface_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_DBUS_PROPERTIES,
       tp_dbus_properties_mixin_iface_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CONNECTION_INTERFACE_CONTACTS,
       tp_contacts_mixin_iface_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CONNECTION_INTERFACE_CONTACT_LIST,
       tp_base_contact_list_mixin_list_iface_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CONNECTION_INTERFACE_CONTACT_GROUPS,
       tp_base_contact_list_mixin_groups_iface_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CONNECTION_INTERFACE_CONTACT_BLOCKING,
       tp_base_contact_list_mixin_blocking_iface_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CONNECTION_INTERFACE_PRESENCE,
       tp_presence_mixin_iface_init);
    G_IMPLEMENT_INTERFACE (TP_TYPE_SVC_CONNECTION_INTERFACE_SIMPLE_PRESENCE,
       tp_presence_mixin_simple_presence_iface_init)
    );

enum {
	PROP_USERNAME = 1,
	PROP_PASSWORD,
	LAST_PROPERTY_ENUM
};


struct _LwqqConnectionPrivate {
   char* username;
   char* password;

   LwdbUserDB* db;

   TpHandleRepoIface* contact_repo;
   TpHandleSet* contacts;

   /* so we can pop up a SASL channel asking for the password */
   TpSimplePasswordManager *password_manager;
};

static const gchar * interfaces_always_present[] = {
	TP_IFACE_CONNECTION_INTERFACE_CONTACTS,
   TP_IFACE_CONNECTION_INTERFACE_CONTACT_LIST,
   TP_IFACE_CONNECTION_INTERFACE_PRESENCE,
   TP_IFACE_CONNECTION_INTERFACE_SIMPLE_PRESENCE,
	TP_IFACE_CONNECTION_INTERFACE_ALIASING,
   TP_IFACE_CONNECTION_INTERFACE_AVATARS,
	NULL};

const gchar * const *lwqq_connection_get_implemented_interfaces (void) {
	/* we don't have any conditionally-implemented interfaces yet */
	return interfaces_always_present;
}

static TpDBusPropertiesMixinIfaceImpl prop_interfaces[] = {
   { TP_IFACE_CONNECTION_INTERFACE_AVATARS,
      avatars_properties_getter,
      NULL,
      NULL,
   },
   { NULL }
};

enum
{
  ALIAS_UPDATED,
  PRESENCE_UPDATED,
  AVATAR_UPDATED,
  N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0 };

static gchar*
_contact_normalize (TpHandleRepoIface *repo,
                    const gchar *id,
                    gpointer context,
                    GError **error)
{
    printf("%s\n",id);
    return g_strdup(id);
    /*LwqqConnection *conn = LWQQ_CONNECTION (context);
    LwqqClient* lc = conn->lc;
    if(lc==NULL) return g_strdup(id);
    return g_strdup (lc->myself->nick);*/
}

static void _iface_create_handle_repos(TpBaseConnection *self,
        TpHandleRepoIface *repos[NUM_TP_HANDLE_TYPES]) 
{
    repos[TP_HANDLE_TYPE_CONTACT] =
        tp_dynamic_handle_repo_new (TP_HANDLE_TYPE_CONTACT, _contact_normalize,
                                    self);
}



static GPtrArray *_iface_create_channel_managers(TpBaseConnection *base) {
	LwqqConnection *self = LWQQ_CONNECTION (base);
	LwqqConnectionPrivate *priv = self->priv;
	GPtrArray *managers = g_ptr_array_sized_new(1);
	GObject *manager;

	manager = g_object_new(LWQQ_TYPE_IM_MANAGER, "connection", self, NULL);
	g_ptr_array_add(managers, manager);

	/*
	manager = g_object_new(LWQQ_TYPE_MUC_MANAGER, "connection", self, NULL);
	g_ptr_array_add(managers, manager);
	*/

   priv->password_manager = tp_simple_password_manager_new(base);
   g_ptr_array_add(managers, priv->password_manager);
   self->contact_list =
      LWQQ_CONTACT_LIST(g_object_new(LWQQ_TYPE_CONTACT_LIST,"connection",self,NULL));
   g_ptr_array_add(managers, self->contact_list);


	/*
	manager = g_object_new(IDLE_TYPE_ROOMLIST_MANAGER, "connection", self, NULL);
	g_ptr_array_add(managers, manager);

	priv->tls_manager = g_object_new (IDLE_TYPE_SERVER_TLS_MANAGER,
		"connection", self,
                NULL);
	g_ptr_array_add(managers, priv->tls_manager);
	*/

	return managers;
}


static void _iface_shut_down(TpBaseConnection *base) {
#if 0
	IdleConnection *self = IDLE_CONNECTION (base);
	IdleConnectionPrivate *priv = self->priv;

	if (priv->quitting)
		return;

	/* we never got around to actually creating the connection
	 * iface object because we were still trying to connect, so
	 * don't try to send any traffic down it */
	if (priv->conn == NULL) {
		g_idle_add(_finish_shutdown_idle_func, self);
	} else if (!priv->sconn_connected) {
		IDLE_DEBUG("cancelling connection");
		g_cancellable_cancel (priv->connect_cancellable);
	} else {
		idle_server_connection_disconnect_async(priv->conn, NULL, NULL, NULL);
	}
#endif
}

static void login_into_server(LwqqClient* lc,LwqqErrorCode* err)
{
    LwqqConnection* conn = lc->data;
    LwqqBuddy* buddy;

    tp_base_connection_change_status(&conn->parent,
            TP_CONNECTION_STATUS_CONNECTED,
            TP_CONNECTION_STATUS_REASON_REQUESTED);
    /*LIST_FOREACH(buddy, &lc->friends, entries){
        lwqq_contact_list_add_buddy(lc, buddy);
    }*/
    /*qq_account* ac = lwqq_client_userdata(lc);

    lwdb_userdb_flush_buddies(ac->db, 5,5);
    lwdb_userdb_flush_groups(ac->db, 1,10);

    if(ac->flag&QQ_USE_QQNUM){
        lwdb_userdb_query_qqnumbers(ac->db,lc);
    }

    //make sure all qqnumber is setup,then make state connected
    purple_connection_set_state(purple_account_get_connection(ac->account),PURPLE_CONNECTED);

    if(!purple_account_get_alias(ac->account))
        purple_account_set_alias(ac->account,lc->myself->nick);
    if(purple_buddy_icons_find_account_icon(ac->account)==NULL){
        LwqqAsyncEvent* ev=lwqq_info_get_friend_avatar(lc,lc->myself);
        lwqq_async_add_event_listener(ev,_C_(2p,friend_avatar,ac,lc->myself));
    }

    LwqqAsyncEvent* ev = NULL;

    //we must put buddy and group clean before any add operation.
    GSList* ptr = purple_blist_get_buddies();
    while(ptr){
        PurpleBuddy* buddy = ptr->data;
        if(buddy->account == ac->account){
            const char* qqnum = purple_buddy_get_name(buddy);
            //if it isn't a qqnumber,we should delete it whatever.
            if(lwqq_buddy_find_buddy_by_qqnumber(lc,qqnum)==NULL){
                purple_blist_remove_buddy(buddy);
            }
        }
        ptr = ptr->next;
    }

    //clean extra duplicated node
    all_reset(ac,RESET_GROUP_SOFT|RESET_DISCU_SOFT);

    LwqqAsyncEvset* set = lwqq_async_evset_new();
    
    LwqqBuddy* buddy;
    LIST_FOREACH(buddy,&lc->friends,entries) {
        lwdb_userdb_query_buddy(ac->db, buddy);
        if((ac->flag& QQ_USE_QQNUM)&& ! buddy->qqnumber){
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
        if(buddy->last_modify != -1 && buddy->last_modify != 0)
            friend_come(lc,buddy);
    }
    //friend_come(lc,create_system_buddy(lc));
    
    LwqqGroup* group;
    LIST_FOREACH(group,&lc->groups,entries) {
        //LwqqAsyncEvset* set = NULL;
        lwdb_userdb_query_group(ac->db, group);
        if((ac->flag && QQ_USE_QQNUM)&& ! group->account){
            ev = lwqq_info_get_group_qqnumber(lc,group);
            lwqq_async_evset_add_event(set, ev);
        }
        if(group->last_modify == -1 || group->last_modify == 0){
            ev = lwqq_info_get_group_memo(lc, group);
            lwqq_async_evset_add_event(set, ev);
        }
        //because group avatar less changed.
        //so we dont reload it.
        if(group->last_modify != -1 && group->last_modify != 0)
            group_come(lc,group);
    }

    LwqqGroup* discu;
    LIST_FOREACH(discu,&lc->discus,entries){
        if(!discu->account||discu->last_modify==-1){
            ev = lwqq_info_get_discu_detail_info(lc, discu);
            lwqq_async_evset_add_event(set, ev);
        }
        if(discu->last_modify!=-1)
            discu_come(lc,discu);
    }
    lwqq_async_add_evset_listener(set, _C_(p,login_stage_f,lc));


    ac->state = LOAD_COMPLETED;

    LwqqPollOption flags = POLL_AUTO_DOWN_DISCU_PIC|POLL_AUTO_DOWN_GROUP_PIC|POLL_AUTO_DOWN_BUDDY_PIC;
    if(ac->flag& REMOVE_DUPLICATED_MSG)
        flags |= POLL_REMOVE_DUPLICATED_MSG;
    if(ac->flag& NOT_DOWNLOAD_GROUP_PIC)
        flags &= ~POLL_AUTO_DOWN_GROUP_PIC;

    lwqq_msglist_poll(lc->msg_list, flags);*/
}


static gboolean local_do_dispatch(gpointer data)
{
    LwqqCommand* cmd = data;
    vp_do(*cmd, NULL);
    free(cmd);
    return 0;
}
static void local_dispatch(LwqqCommand cmd, unsigned long timeout)
{
   LwqqCommand* cmd_ = s_malloc0(sizeof(*cmd_));
   *cmd_ = cmd;
   g_timeout_add(timeout, local_do_dispatch, cmd_);
}
static void register_events(LwqqClient* lc)
{
   lwqq_add_event(lc->events->login_complete, 
         _C_(2p, login_into_server, lc, &lc->args->login_ec));
}
/******START CONNECTION TO SERVER*****************/
static gboolean _iface_start_connecting(TpBaseConnection *self, GError **error) {
	LwqqConnection *conn = LWQQ_CONNECTION(self);
	LwqqConnectionPrivate *priv = conn->priv;
   TpHandleRepoIface *contact_repo =
      tp_base_connection_get_handles (self, TP_HANDLE_TYPE_CONTACT);

    self->self_handle = tp_handle_ensure (contact_repo,
        priv->username, NULL, NULL);
    if (!self->self_handle)
        return FALSE;
    conn->priv->contact_repo = contact_repo;
    conn->priv->contacts = tp_handle_set_new(contact_repo);

	//g_assert(priv->nickname != NULL);

	if (conn->lc != NULL) {
		//verbose connection already open
		g_set_error(error, TP_ERROR, TP_ERROR_NOT_AVAILABLE, "connection already open!");
		return FALSE;
	}

	/*
	if (priv->password_prompt) {
		tp_simple_password_manager_prompt_async(priv->password_manager, _password_prompt_cb, conn);
	} else {
		//_start_connecting_continue(conn);
	}
	*/


   tp_base_connection_change_status(self, TP_CONNECTION_STATUS_CONNECTING,
         TP_CONNECTION_STATUS_REASON_REQUESTED);

	LwqqClient* lc = lwqq_client_new(priv->username,priv->password);
   if(!lc) return FALSE;
	lwqq_log_set_level(4);
   register_events(lc);
   lc->dispatch = local_dispatch;
   lc->data = conn;

   LwdbUserDB* db = lwdb_userdb_new(lc->myself->qqnumber, NULL, 0);
   if(!db) return FALSE;
   const char* last_hash = lwdb_userdb_read(db, "last_hash");
   if(last_hash) lwqq_hash_set_beg(lc, last_hash);
   

   lwqq_login(lc, LWQQ_STATUS_ONLINE, NULL);
	conn->lc = lc;
   conn->priv->db = db;
	return TRUE;
}

LwdbUserDB* lwqq_connection_get_db(LwqqConnection* conn)
{
   if(!conn) return NULL;
   return conn->priv->db;
}

//=================CONNECTION CLASS DEFINE======================//
static void lwqq_connection_init(LwqqConnection *obj) {
	LwqqConnectionPrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE (obj, LWQQ_TYPE_CONNECTION, LwqqConnectionPrivate);

   obj->priv = priv;
   /*
      priv->sconn_connected = FALSE;
      priv->msg_queue = g_queue_new();

      tp_contacts_mixin_init ((GObject *) obj, G_STRUCT_OFFSET (LwqqConnection, contacts));
      tp_base_connection_register_with_contacts_mixin ((TpBaseConnection *) obj);
      */
}
static void lwqq_connection_finalize(GObject* obj)
{
    LwqqConnection* self = LWQQ_CONNECTION(obj);

    tp_contacts_mixin_finalize (obj);

    G_OBJECT_CLASS (lwqq_connection_parent_class)->finalize (
            obj);
}

static void
lwqq_connection_constructed (GObject *object)
{
    TpBaseConnection* base = TP_BASE_CONNECTION(object);

    void (*chain_up) (GObject *) =
        G_OBJECT_CLASS (lwqq_connection_parent_class)->constructed;

    if (chain_up != NULL)
        chain_up (object);


    tp_contacts_mixin_init (object,
            G_STRUCT_OFFSET (LwqqConnection, contacts));

    tp_base_connection_register_with_contacts_mixin (base);
    tp_base_contact_list_mixin_register_with_contacts_mixin (base);

    presence_init(object);
    aliasing_init(object);
    avatars_init(object);
}

static void set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec) {
	LwqqConnection *self = LWQQ_CONNECTION(obj);
	LwqqConnectionPrivate *priv = self->priv;

	switch (prop_id) {
		case PROP_USERNAME:
			g_free(priv->username);
			priv->username = g_value_dup_string(value);
			break;
		case PROP_PASSWORD:
			g_free(priv->password);
			priv->password = g_value_dup_string(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}

static void get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec) {
	LwqqConnection *self = LWQQ_CONNECTION(obj);
	LwqqConnectionPrivate *priv = self->priv;

	switch (prop_id) {
		case PROP_USERNAME:
            g_value_set_string(value, priv->username);
			break;
		case PROP_PASSWORD:
            g_value_set_string(value, priv->password);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
			break;
	}
}


static void lwqq_connection_class_init(LwqqConnectionClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	TpBaseConnectionClass *parent_class = TP_BASE_CONNECTION_CLASS(klass);
	GParamSpec *param_spec;

	g_type_class_add_private(klass, sizeof(LwqqConnectionPrivate));

   object_class->constructed = lwqq_connection_constructed;
	object_class->set_property = set_property;
	object_class->get_property = get_property;
	object_class->finalize = lwqq_connection_finalize;

	parent_class->create_handle_repos = _iface_create_handle_repos;
	parent_class->create_channel_managers = _iface_create_channel_managers;
	parent_class->shut_down = _iface_shut_down;
	parent_class->start_connecting = _iface_start_connecting;
   parent_class->interfaces_always_present = interfaces_always_present;
   /*
	parent_class->get_unique_connection_name = _iface_get_unique_connection_name;
	parent_class->connecting = NULL;
	parent_class->connected = NULL;
	parent_class->disconnected = _iface_disconnected;
	parent_class->interfaces_always_present = interfaces_always_present;
    */
	param_spec = g_param_spec_string("username", "User name", 
            "The username of the user connecting to WebQQ", NULL, 
            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(object_class, PROP_USERNAME, param_spec);

	param_spec = g_param_spec_string("password", "Server password", 
            "Password to authenticate to the server with", 
            NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(object_class, PROP_PASSWORD, param_spec);

    /*
	param_spec = g_param_spec_string("nickname", "IRC nickname", 
            "The nickname to be visible to others in IRC.", 
            NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(object_class, PROP_NICKNAME, param_spec);


	param_spec = g_param_spec_string("realname", "Real name", "The real name of the user connecting to IRC", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(object_class, PROP_REALNAME, param_spec);
	param_spec = g_param_spec_string("server", "Hostname or IP of the IRC server to connect to", 
            "The server used when establishing the connection.", 
            NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_property(object_class, PROP_SERVER, param_spec);

	param_spec = g_param_spec_uint("port", "IRC server port", "The destination port used when establishing the connection.", 0, G_MAXUINT16, 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);
	g_object_class_install_property(object_class, PROP_PORT, param_spec);

	param_spec = g_param_spec_string("charset", "Character set", "The character set to use to communicate with the outside world", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);
	g_object_class_install_property(object_class, PROP_CHARSET, param_spec);

	param_spec = g_param_spec_uint("keepalive-interval", "Keepalive interval", "Seconds between keepalive packets, or 0 to disable", 0, G_MAXUINT, DEFAULT_KEEPALIVE_INTERVAL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);
	g_object_class_install_property(object_class, PROP_KEEPALIVE_INTERVAL, param_spec);

	param_spec = g_param_spec_string("quit-message", "Quit message", "The quit message to send to the server when leaving IRC", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);
	g_object_class_install_property(object_class, PROP_QUITMESSAGE, param_spec);

	param_spec = g_param_spec_boolean("use-ssl", "Use SSL", "If the connection should use a SSL tunneled socket connection", FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);
	g_object_class_install_property(object_class, PROP_USE_SSL, param_spec);

	param_spec = g_param_spec_boolean("password-prompt", "Password prompt", "Whether the connection should pop up a SASL channel if no password is given", FALSE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT);
	g_object_class_install_property(object_class, PROP_PASSWORD_PROMPT, param_spec);

    */
	//lwqq_contact_info_class_init(klass);

   prop_interfaces[0].props = avatars_properties;
   klass->properties_class.interfaces = prop_interfaces;
   tp_dbus_properties_mixin_class_init(object_class,
         G_STRUCT_OFFSET(LwqqConnectionClass, properties_class));

   tp_contacts_mixin_class_init (object_class,
         G_STRUCT_OFFSET (LwqqConnectionClass, contacts_class));
   tp_base_contact_list_mixin_class_init (parent_class);
   presence_class_init(klass);

   signals[PRESENCE_UPDATED] = g_signal_new("presence-updated",
         G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
         g_cclosure_marshal_VOID__UINT,
         G_TYPE_NONE, 1,G_TYPE_UINT);
   signals[ALIAS_UPDATED] = g_signal_new ("alias-updated",
         G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
         g_cclosure_marshal_VOID__UINT, 
         G_TYPE_NONE, 1, G_TYPE_UINT);
   /* "avatar-updated" is used by system */
   signals[AVATAR_UPDATED] = g_signal_new ("lwqq-avatar-updated",
         G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST, 0, NULL, NULL,
         g_cclosure_marshal_VOID__UINT_POINTER, 
         G_TYPE_NONE, 2, G_TYPE_UINT,G_TYPE_POINTER);
}


//-----------------CONNECTION CLASS DEFINE----------------------//

const gchar *
lwqq_connection_handle_inspect (LwqqConnection *conn,
                                TpHandleType handle_type,
                                TpHandle handle)
{
    TpBaseConnection *base_conn = TP_BASE_CONNECTION (conn);
    TpHandleRepoIface *handle_repo =
        tp_base_connection_get_handles (base_conn, handle_type);
    g_assert (tp_handle_is_valid (handle_repo, handle, NULL));
    return tp_handle_inspect (handle_repo, handle);
}
