#include <glib.h>
#include <telepathy-glib/telepathy-glib.h>


G_BEGIN_DECLS

typedef struct _LwqqCaptchaPrivate LwqqCaptchaPrivate;

typedef struct {
    TpBaseChannel parent;

    LwqqCaptchaPrivate *priv;
} LwqqCaptcha;

typedef struct {
    TpBaseChannelClass parent;
} LwqqCaptchaClass;

#define LWQQ_CAPTCHA_TYPE (lwqq_captcha_get_type ())
#define LWQQ_CAPTCHA(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), LWQQ_CAPTCHA_TYPE, LwqqCaptcha))
#define LWQQ_CAPTCHA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), LWQQ_CAPTCHA_TYPE, LwqqCaptchaClass))
#define IS_LWQQ_CAPTCHA(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LWQQ_CAPTCHA_TYPE))
#define IS_LWQQ_CAPTCHA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LWQQ_CAPTCHA_TYPE))
#define LWQQ_CAPTCHA_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), LWQQ_CAPTCHA_TYPE, LwqqCaptchaClass))



G_END_DECLS
