#include "im-manager.h"
#include "connection.h"

#include <telepathy-glib/channel-manager.h>
#include <telepathy-glib/interfaces.h>
#include <telepathy-glib/dbus.h>


#define LWQQ_IM_MANAGER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), LWQQ_TYPE_IM_MANAGER, LwqqIMManagerPrivate))

/* properties */
enum {
	PROP_CONNECTION = 1,
	LAST_PROPERTY_ENUM
};
typedef struct _LwqqIMManagerPrivate LwqqIMManagerPrivate;
struct _LwqqIMManagerPrivate {
	LwqqConnection *conn;
	GHashTable *channels;
	int status_changed_id;
	gboolean dispose_has_run;
};
//==============IM-MANAGER CLASS DEFINE=================//

static void _im_manager_iface_init(gpointer g_iface, gpointer iface_data) {
	TpChannelManagerIface *iface = g_iface;

	/*
	iface->foreach_channel = _im_manager_foreach;
	iface->type_foreach_channel_class = _im_manager_type_foreach_class;
	iface->request_channel = _im_manager_request_channel;
	iface->create_channel = _im_manager_create_channel;
	iface->ensure_channel = _im_manager_ensure_channel;
	*/
}

static void lwqq_im_manager_init(LwqqIMManager *obj) {
	LwqqIMManagerPrivate *priv = LWQQ_IM_MANAGER_GET_PRIVATE(obj);
	priv->channels = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_object_unref);
	priv->status_changed_id = 0;
	priv->dispose_has_run = FALSE;
}

static void lwqq_im_manager_class_init(LwqqIMManagerClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GParamSpec *param_spec;

	g_type_class_add_private(klass, sizeof(LwqqIMManagerPrivate));

	//object_class->constructed = _im_manager_constructed;
	//object_class->dispose = _im_manager_dispose;
	//object_class->get_property = lwqq_im_manager_get_property;
	//object_class->set_property = lwqq_im_manager_set_property;

	param_spec = g_param_spec_object("connection", "LwqqConnection object",
			"The LwqqConnection object that owns this IM channel manager object.",
			LWQQ_TYPE_CONNECTION, G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_READWRITE | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
	g_object_class_install_property(object_class, PROP_CONNECTION, param_spec);
}
G_DEFINE_TYPE_WITH_CODE(LwqqIMManager, lwqq_im_manager, G_TYPE_OBJECT,\
		G_IMPLEMENT_INTERFACE(TP_TYPE_CHANNEL_MANAGER,_im_manager_iface_init))
//--------------IM-MANAGER CLASS DEFINE-----------------//
