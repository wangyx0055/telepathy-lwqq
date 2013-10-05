/*
 * protocol.c - LwqqProtocol
 * Copyright © 2007-2010 Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include "protocol.h"

#include <dbus/dbus-glib.h>
#include <dbus/dbus-protocol.h>
#include <telepathy-glib/base-connection-manager.h>
#include <telepathy-glib/telepathy-glib.h>

#include "connection.h"
/*#include "lwqq-handles.h"
#include "lwqq-im-manager.h"
#include "lwqq-muc-manager.h"*/

#define PROTOCOL_NAME "webqq"
#define ICON_NAME "im-" PROTOCOL_NAME
#define ENGLISH_NAME "WEBQQ"
#define VCARD_FIELD_NAME "x-" PROTOCOL_NAME

G_DEFINE_TYPE (LwqqProtocol, lwqq_protocol, TP_TYPE_BASE_PROTOCOL)

static const TpCMParamSpec lwqq_params[] = {
    { "username", DBUS_TYPE_STRING_AS_STRING, G_TYPE_STRING, 
      TP_CONN_MGR_PARAM_FLAG_REQUIRED },
    { "password", DBUS_TYPE_STRING_AS_STRING, G_TYPE_STRING,
      TP_CONN_MGR_PARAM_FLAG_SECRET|TP_CONN_MGR_PARAM_FLAG_REQUIRED },
    /*
    { "fullname", DBUS_TYPE_STRING_AS_STRING, G_TYPE_STRING, 0 },
    { "charset", DBUS_TYPE_STRING_AS_STRING, G_TYPE_STRING,
      TP_CONN_MGR_PARAM_FLAG_HAS_DEFAULT, "UTF-8" },
    { "keepalive-interval", DBUS_TYPE_UINT32_AS_STRING, G_TYPE_UINT,
      TP_CONN_MGR_PARAM_FLAG_HAS_DEFAULT,
      GUINT_TO_POINTER (DEFAULT_KEEPALIVE_INTERVAL) },
    { "quit-message", DBUS_TYPE_STRING_AS_STRING, G_TYPE_STRING, 0 },
    { "use-ssl", DBUS_TYPE_BOOLEAN_AS_STRING, G_TYPE_BOOLEAN,
      TP_CONN_MGR_PARAM_FLAG_HAS_DEFAULT, GINT_TO_POINTER (FALSE) },
    { "password-prompt", DBUS_TYPE_BOOLEAN_AS_STRING, G_TYPE_BOOLEAN,
      TP_CONN_MGR_PARAM_FLAG_HAS_DEFAULT, GINT_TO_POINTER (FALSE) },*/
    { NULL, NULL, 0, 0, NULL, 0 }
};

static void
lwqq_protocol_init (LwqqProtocol *self)
{
}

static const TpCMParamSpec *
get_parameters (TpBaseProtocol *self G_GNUC_UNUSED)
{
  return lwqq_params;
}

static TpBaseConnection *
new_connection (TpBaseProtocol *protocol G_GNUC_UNUSED,
                GHashTable *params,
                GError **error G_GNUC_UNUSED)
{


  return g_object_new (LWQQ_TYPE_CONNECTION,
      "protocol", PROTOCOL_NAME,
      "username", tp_asv_get_string (params, "username"),
      "password", tp_asv_get_string (params, "password"),
      //"nickname", tp_asv_get_string (params, "account"),
      //"realname", tp_asv_get_string (params, "fullname"),
      NULL);
}
/*
static gchar *
normalize_contact (TpBaseProtocol *self G_GNUC_UNUSED,
                   const gchar *contact,
                   GError **error)
{
  return lwqq_normalize_nickname (contact, error);
}

static gchar *
identify_account (TpBaseProtocol *self G_GNUC_UNUSED,
    GHashTable *asv,
    GError **error)
{
  gchar *nick = lwqq_normalize_nickname (tp_asv_get_string (asv, "account"),
      error);
  gchar *server;
  gchar *nick_at_server;

  if (nick == NULL)
    return NULL;

  server = g_ascii_strdown (tp_asv_get_string (asv, "server"), -1);

  nick_at_server = g_strdup_printf ("%s@%s", nick, server);
  g_free (server);
  g_free (nick);
  return nick_at_server;
}

static GStrv
get_interfaces (TpBaseProtocol *self)
{
  return g_new0 (gchar *, 1);
}


static GStrv
dup_authentication_types (TpBaseProtocol *base)
{
  const gchar * const types[] = {
    TP_IFACE_CHANNEL_INTERFACE_SASL_AUTHENTICATION,
    NULL,
  };

  return g_strdupv ((GStrv) types);
}

*/
static void
get_connection_details (TpBaseProtocol *self,
    GStrv *connection_interfaces,
    GType **channel_managers,
    gchar **icon_name,
    gchar **english_name,
    gchar **vcard_field)
{
  if (connection_interfaces != NULL)
    {
      *connection_interfaces = g_strdupv (
          (GStrv) lwqq_connection_get_implemented_interfaces ());
    }

  if (channel_managers != NULL)
    {
      GType types[] = {
          G_TYPE_INVALID };

      *channel_managers = g_memdup (types, sizeof(types));
    }

  if (icon_name != NULL)
    {
      *icon_name = g_strdup (ICON_NAME);
    }

  if (vcard_field != NULL)
    {
      *vcard_field = g_strdup (VCARD_FIELD_NAME);
    }

  if (english_name != NULL)
    {
      *english_name = g_strdup (ENGLISH_NAME);
    }
}
static void
lwqq_protocol_class_init (LwqqProtocolClass *klass)
{
  TpBaseProtocolClass *base_class = (TpBaseProtocolClass *) klass;

  base_class->get_parameters = get_parameters;
  base_class->get_connection_details = get_connection_details;
  base_class->new_connection = new_connection;
  /*base_class->normalize_contact = normalize_contact;
  base_class->identify_account = identify_account;
  base_class->get_interfaces = get_interfaces;
  base_class->dup_authentication_types = dup_authentication_types;
  */
}

TpBaseProtocol *
lwqq_protocol_new (void)
{
  return g_object_new (LWQQ_TYPE_PROTOCOL,
      "name", PROTOCOL_NAME,
      NULL);
}
