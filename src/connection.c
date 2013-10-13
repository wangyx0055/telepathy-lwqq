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

#include <string.h>
#include <time.h>

#include <dbus/dbus-glib.h>

#include <telepathy-glib/dbus.h>
#include <telepathy-glib/enums.h>
#include <telepathy-glib/errors.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/simple-password-manager.h>
#include <telepathy-glib/svc-connection.h>
#include <telepathy-glib/channel-manager.h>
#include <telepathy-glib/gtypes.h>
#include <telepathy-glib/util.h>

#include <lwqq.h>

enum {
	PROP_USERNAME = 1,
	PROP_PASSWORD,
	LAST_PROPERTY_ENUM
};


struct _LwqqConnectionPrivate {
	LwqqClient* lc;
    char* username;
    char* password;

	/* so we can pop up a SASL channel asking for the password */
	TpSimplePasswordManager *password_manager;
};

static const gchar * interfaces_always_present[] = {
	/*TP_IFACE_CONNECTION_INTERFACE_ALIASING,
	TP_IFACE_CONNECTION_INTERFACE_CONTACT_INFO,
	LWQQ_IFACE_CONNECTION_INTERFACE_RENAMING,
	TP_IFACE_CONNECTION_INTERFACE_REQUESTS,
	TP_IFACE_CONNECTION_INTERFACE_CONTACTS,*/
	NULL};

const gchar * const *lwqq_connection_get_implemented_interfaces (void) {
	/* we don't have any conditionally-implemented interfaces yet */
	return interfaces_always_present;
}


static void _iface_create_handle_repos(TpBaseConnection *self, TpHandleRepoIface **repos) {
	int i;
	for (i = 0; i < NUM_TP_HANDLE_TYPES; i++)
		repos[i] = NULL;

	idle_handle_repos_init(repos);
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


/******START CONNECTION TO SERVER*****************/
static gboolean _iface_start_connecting(TpBaseConnection *self, GError **error) {
	LwqqConnection *conn = LWQQ_CONNECTION(self);
	LwqqConnectionPrivate *priv = conn->priv;

	//g_assert(priv->nickname != NULL);

	if (priv->lc != NULL) {
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

	LwqqClient* lc = lwqq_client_new(priv->username,priv->password);
	lwqq_log_set_level(4);
	lwqq_login(lc, LWQQ_STATUS_ONLINE, NULL);
	priv->lc = lc;
	return TRUE;
}

//=================CONNECTION CLASS DEFINE======================//

G_DEFINE_TYPE (LwqqConnection, lwqq_connection, TP_TYPE_BASE_CONNECTION)

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
static void lwqq_connection_constructed(GObject* obj)
{
}
static void lwqq_connection_set_property(GObject *obj, guint prop_id, const GValue *value, GParamSpec *pspec) {
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

static void lwqq_connection_get_property(GObject *obj, guint prop_id, GValue *value, GParamSpec *pspec) {
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
	object_class->set_property = lwqq_connection_set_property;
	object_class->get_property = lwqq_connection_get_property;
    /*
	object_class->dispose = lwqq_connection_dispose;
	object_class->finalize = lwqq_connection_finalize;
    */

	parent_class->create_handle_repos = _iface_create_handle_repos;
	parent_class->create_channel_factories = NULL;
	parent_class->create_channel_managers = _iface_create_channel_managers;
	parent_class->shut_down = _iface_shut_down;
	parent_class->start_connecting = _iface_start_connecting;
    /*
	parent_class->get_unique_connection_name = _iface_get_unique_connection_name;
	parent_class->connecting = NULL;
	parent_class->connected = NULL;
	parent_class->disconnected = _iface_disconnected;
	parent_class->interfaces_always_present = interfaces_always_present;
    */
	param_spec = g_param_spec_string("username", "User name", "The username of the user connecting to IRC", NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
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

	tp_contacts_mixin_class_init (object_class, G_STRUCT_OFFSET (LwqqConnectionClass, contacts));
	lwqq_contact_info_class_init(klass);
    */

#if 0
	/* This is a hack to make the test suite run in finite time. */
	if (!tp_str_empty (g_getenv ("LWQQ_HTFU")))
		flush_queue_faster = TRUE;
#endif
}
//-----------------CONNECTION CLASS DEFINE----------------------//
