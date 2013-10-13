
#ifndef __LWQQ_IM_MANAGER_H__
#define __LWQQ_IM_MANAGER_H__

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _LwqqIMManager LwqqIMManager;
typedef struct _LwqqIMManagerClass LwqqIMManagerClass;

struct _LwqqIMManagerClass {
	GObjectClass parent_class;
};

struct _LwqqIMManager {
	GObject parent;
};

GType lwqq_im_manager_get_type (void);

#define LWQQ_TYPE_IM_MANAGER (lwqq_im_manager_get_type())
#define LWQQ_IM_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), LWQQ_TYPE_IM_MANAGER, LwqqIMManager))
#define LWQQ_IM_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), LWQQ_TYPE_IM_MANAGER, LwqqIMManagerClass))
#define LWQQ_IS_IM_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), LWQQ_TYPE_IM_MANAGER))
#define LWQQ_IS_IM_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), LWQQ_TYPE_IM_MANAGER))
#define LWQQ_IM_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), LWQQ_TYPE_IM_MANAGER, LwqqIMManagerClass))

G_END_DECLS

#endif /* #ifndef __LWQQ_IM_MANAGER_H__ */
