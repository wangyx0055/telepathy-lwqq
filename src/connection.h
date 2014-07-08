/*
 * This file is part of telepathy-lwqq
 *
 * Copyright (C) 2006-2007 Collabora Limited
 * Copyright (C) 2006-2007 Nokia Corporation
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

#ifndef __LWQQ_CONNECTION_H__
#define __LWQQ_CONNECTION_H__

#include <glib-object.h>
#include <telepathy-glib/telepathy-glib.h>

#include "contact-list.h"
#include <lwqq/lwdb.h>

G_BEGIN_DECLS

typedef struct _LwqqConnection LwqqConnection;
typedef struct _LwqqConnectionClass LwqqConnectionClass;
typedef struct _LwqqConnectionPrivate LwqqConnectionPrivate;

struct _LwqqConnectionClass {
   TpBaseConnectionClass parent_class;
   TpContactsMixinClass contacts_class;
   TpPresenceMixinClass presence_class;
   TpDBusPropertiesMixinClass properties_class;
};

struct _LwqqConnection {
   TpBaseConnection parent;
   TpContactsMixin contacts;
   TpPresenceMixin presence;
	//LwqqParser *parser;
   GQueue *contact_info_requests;
   LwqqContactList* contact_list;

   LwqqClient* lc;

   LwqqConnectionPrivate *priv;
};


GType lwqq_connection_get_type(void);

/* TYPE MACROS */
#define LWQQ_TYPE_CONNECTION \
	(lwqq_connection_get_type())
#define LWQQ_CONNECTION(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), LWQQ_TYPE_CONNECTION, LwqqConnection))
#define LWQQ_CONNECTION_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), LWQQ_TYPE_CONNECTION, LwqqConnectionClass))
#define LWQQ_IS_CONNECTION(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), LWQQ_TYPE_CONNECTION))
#define LWQQ_IS_CONNECTION_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), LWQQ_TYPE_CONNECTION))
#define LWQQ_CONNECTION_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), LWQQ_TYPE_CONNECTION, LwqqConnectionClass))

/*void lwqq_connection_canon_nick_receive(LwqqConnection *conn, TpHandle handle, const gchar *canon_nick);
void lwqq_connection_emit_queued_aliases_changed(LwqqConnection *conn);
void lwqq_connection_send(LwqqConnection *conn, const gchar *msg);
gsize lwqq_connection_get_max_message_length(LwqqConnection *conn);
*/

G_END_DECLS


const gchar * const *lwqq_connection_get_implemented_interfaces (void);
const gchar * lwqq_connection_handle_inspect (LwqqConnection *conn,
        TpHandleType handle_type, TpHandle handle);
LwdbUserDB* lwqq_connection_get_db(LwqqConnection* conn);
LwqqBuddy* lwqq_find_buddy_by_handle(LwqqConnection* conn, TpHandle h);

#endif /* #ifndef __LWQQ_CONNECTION_H__*/
