/* test.h generated by valac 0.18.1, the Vala compiler, do not modify */


#ifndef __LWQQ_CONNECTION_MANAGER_H__
#define __LWQQ_CONNECTION_MANAGER_H__

#include <glib.h>
#include <telepathy-glib/telepathy-glib.h>

G_BEGIN_DECLS


#define LWQQ_TYPE_CONNECTION_MANAGER (lwqq_connection_manager_get_type ())
#define LWQQ_CONNECTION_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_LWQQ_CONNECTION_MANAGER, LwqqConnectionManager))
#define LWQQ_CONNECTION_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_LWQQ_CONNECTION_MANAGER, LwqqConnectionManagerClass))
#define IS_LWQQ_CONNECTION_MANAGER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_LWQQ_CONNECTION_MANAGER))
#define IS_LWQQ_CONNECTION_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_LWQQ_CONNECTION_MANAGER))
#define LWQQ_CONNECTION_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_LWQQ_CONNECTION_MANAGER, LwqqConnectionManagerClass))

typedef struct _LwqqConnectionManager LwqqConnectionManager;
typedef struct _LwqqConnectionManagerClass LwqqConnectionManagerClass;

struct _LwqqConnectionManager {
	TpBaseConnectionManager parent_instance;
};

struct _LwqqConnectionManagerClass {
	TpBaseConnectionManagerClass parent_class;
};


GType lwqq_connection_manager_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif