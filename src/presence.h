#ifndef __LWQQ_PRESENCE_H_
#define __LWQQ_PRESENCE_H_
#include "connection.h"

G_BEGIN_DECLS

void presence_class_init(LwqqConnectionClass* klass);

void presence_init(GObject* obj);

G_END_DECLS

#endif
