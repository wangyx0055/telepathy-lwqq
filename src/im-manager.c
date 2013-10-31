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
	GPtrArray *channels;
    int channel_index;
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


static void
lwqq_im_manager_init (LwqqIMManager *self)
{
  LwqqIMManagerPrivate *priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
      LWQQ_TYPE_IM_MANAGER, LwqqIMManagerPrivate);

  self->priv = priv;

  priv->channels = g_ptr_array_sized_new (1);
  priv->channel_index = 0;

  priv->conn = NULL;
  priv->dispose_has_run = FALSE;
}

G_DEFINE_TYPE_WITH_CODE(LwqqIMManager, lwqq_im_manager, G_TYPE_OBJECT,\
		G_IMPLEMENT_INTERFACE(TP_TYPE_CHANNEL_MANAGER,_im_manager_iface_init))

static void
lwqq_im_manager_constructed (GObject *object)
{
  void (*chain_up) (GObject *) =
      G_OBJECT_CLASS (lwqq_im_manager_parent_class)->constructed;
  LwqqIMManager *self = LWQQ_IM_MANAGER (object);
  LwqqIMManagerPrivate *priv = self->priv;

  if (chain_up != NULL)
    chain_up (object);

//  priv->status_changed_id = g_signal_connect (priv->conn,
//      "status-changed", (GCallback) connection_status_changed_cb, object);
}

static void
lwqq_im_manager_dispose (GObject *object)
{
  LwqqIMManager *self = LWQQ_IM_MANAGER(object);
  LwqqIMManagerPrivate *priv = self->priv;

  if (priv->dispose_has_run)
    return;

  //DEBUG ("dispose called");
  priv->dispose_has_run = TRUE;

  //haze_media_manager_close_all (self);
  g_assert (priv->channels->len == 0);
  g_ptr_array_free (priv->channels, TRUE);

  if (G_OBJECT_CLASS (lwqq_im_manager_parent_class)->dispose)
    G_OBJECT_CLASS (lwqq_im_manager_parent_class)->dispose (object);
}
static void
lwqq_im_manager_get_property (GObject    *object,
                                 guint       property_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  LwqqIMManager *self = LWQQ_IM_MANAGER(object);
  LwqqIMManagerPrivate *priv = self->priv;

  switch (property_id) {
    case PROP_CONNECTION:
      g_value_set_object (value, priv->conn);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void
lwqq_im_manager_set_property (GObject      *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  LwqqIMManager *self = LWQQ_IM_MANAGER(object);
  LwqqIMManagerPrivate *priv = self->priv;

  switch (property_id) {
    case PROP_CONNECTION:
      priv->conn = g_value_get_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

static void lwqq_im_manager_class_init(LwqqIMManagerClass *klass) {
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	GParamSpec *param_spec;

	g_type_class_add_private(klass, sizeof(LwqqIMManagerPrivate));

	object_class->constructed = lwqq_im_manager_constructed;
	object_class->dispose = lwqq_im_manager_dispose;
	object_class->get_property = lwqq_im_manager_get_property;
	object_class->set_property = lwqq_im_manager_set_property;

	param_spec = g_param_spec_object("connection", "LwqqConnection object",
			"The LwqqConnection object that owns this IM channel manager object.",
			LWQQ_TYPE_CONNECTION, G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_READWRITE | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB);
	g_object_class_install_property(object_class, PROP_CONNECTION, param_spec);
}
//--------------IM-MANAGER CLASS DEFINE-----------------//
