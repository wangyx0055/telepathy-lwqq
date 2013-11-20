#include "captcha.h"

#include <dbus/dbus-glib.h>

static void 
captcha_auth_iface_init( gpointer klass,gpointer G_GNUC_UNUSED);

G_DEFINE_TYPE_WITH_CODE(LwqqCaptcha, lwqq_captcha,
        TP_TYPE_BASE_CHANNEL,
        G_IMPLEMENT_INTERFACE (
            TP_TYPE_SVC_CHANNEL_TYPE_SERVER_AUTHENTICATION,NULL);
        G_IMPLEMENT_INTERFACE (
            TP_TYPE_SVC_CHANNEL_INTERFACE_CAPTCHA_AUTHENTICATION,
            captcha_auth_iface_init)
        );

#define LWQQ_CAPTCHA_GET_PRIVATE(o)\
    (G_TYPE_INSTANCE_GET_PRIVATE ((o), LWQQ_CAPTCHA_TYPE, LwqqCaptchaPrivate))

struct _LwqqCaptchaPrivate {
};


static void
lwqq_captcha_dispose (LwqqCaptcha *self)
{
}

static void
lwqq_captcha_finalize (LwqqCaptcha *self)
{
}

static void
lwqq_captcha_init (LwqqCaptcha *self)
{
    LwqqCaptchaPrivate *priv;

    priv = LWQQ_CAPTCHA_GET_PRIVATE (self);
}

static void
lwqq_captcha_class_init (LwqqCaptchaClass *self_class)
{
    GObjectClass *object_class = G_OBJECT_CLASS (self_class);

    g_type_class_add_private (self_class, sizeof (LwqqCaptchaPrivate));
    object_class->dispose = (void (*) (GObject *object)) lwqq_captcha_dispose;
    object_class->finalize = (void (*) (GObject *object)) lwqq_captcha_finalize;
}

static void lwqq_captcha_get_captchas(
        TpSvcChannelInterfaceCaptchaAuthentication* iface,
        DBusGMethodInvocation* context)
{
}

static void lwqq_captcha_get_captcha_data (
        TpSvcChannelInterfaceCaptchaAuthentication *self,
        guint in_ID,
        const gchar *in_Mime_Type,
        DBusGMethodInvocation *context)
{
}

static void lwqq_captcha_answer_captchas(
        TpSvcChannelInterfaceCaptchaAuthentication* self,
        GHashTable* in_Answers,
        DBusGMethodInvocation* context)
{
}

static void lwqq_captcha_cancel_captcha(
        TpSvcChannelInterfaceCaptchaAuthentication* self,
        guint in_Reason,
        const gchar* in_DebugMessage,
        DBusGMethodInvocation* context)
{
}

static void 
captcha_auth_iface_init( gpointer klass,gpointer unused)
{
#define IMPLEMENT(x) tp_svc_channel_interface_captcha_authentication_implement_##x (\
  klass, lwqq_captcha_##x)
  IMPLEMENT (get_captchas);
  IMPLEMENT (get_captcha_data);
  IMPLEMENT (answer_captchas);
  IMPLEMENT (cancel_captcha);
#undef IMPLEMENT
}
