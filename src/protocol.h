/*
 * protocol.h - LwqqProtocol
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

#ifndef LWQQ_PROTOCOL_H
#define LWQQ_PROTOCOL_H

#include <glib-object.h>
#include <telepathy-glib/base-protocol.h>

G_BEGIN_DECLS

typedef struct _LwqqProtocol LwqqProtocol;
typedef struct _LwqqProtocolPrivate LwqqProtocolPrivate;
typedef struct _LwqqProtocolClass LwqqProtocolClass;
typedef struct _LwqqProtocolClassPrivate LwqqProtocolClassPrivate;

struct _LwqqProtocolClass {
    TpBaseProtocolClass parent_class;

    LwqqProtocolClassPrivate *priv;
};

struct _LwqqProtocol {
    TpBaseProtocol parent;

    LwqqProtocolPrivate *priv;
};

GType lwqq_protocol_get_type (void);

#define LWQQ_TYPE_PROTOCOL (lwqq_protocol_get_type ())
#define LWQQ_PROTOCOL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), LWQQ_TYPE_PROTOCOL,  LwqqProtocol))
#define LWQQ_PROTOCOL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), LWQQ_TYPE_PROTOCOL, LwqqProtocolClass))
#define LWQQ_IS_PROTOCOL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LWQQ_TYPE_PROTOCOL))
#define LWQQ_PROTOCOL_GET_CLASS(klass) (G_TYPE_INSTANCE_GET_CLASS ((obj), LWQQ_TYPE_PROTOCOL, LwqqProtocolClass))

G_END_DECLS

#endif
