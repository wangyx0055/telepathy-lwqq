#ifndef __LWQQ_AVATARS_H__
#define __LWQQ_AVATARS_H__

#include "connection.h"

extern TpDBusPropertiesMixinPropImpl *avatars_properties;

G_BEGIN_DECLS

void avatars_iface_init (gpointer g_iface, gpointer iface_data);
void avatars_fill_contact_attributes (GObject *obj,
        const GArray *contacts, GHashTable *attributes_hash);
void avatars_properties_getter (GObject *object, GQuark interface,
        GQuark name, GValue *value, gpointer getter_data);
void avatars_init(GObject *object);

G_END_DECLS

#endif
