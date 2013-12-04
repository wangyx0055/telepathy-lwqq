#ifndef __LWQQ_CONTACT_LIST_H__
#define __LWQQ_CONTACT_LIST_H__

#include <glib-object.h>

#include <telepathy-glib/telepathy-glib.h>
#include <lwqq/lwqq.h>

#define to_presence(lwqq_status) (lwqq_status/10)

G_BEGIN_DECLS

typedef struct _LwqqContactList LwqqContactList;
typedef struct _LwqqContactListClass LwqqContactListClass;
typedef struct _LwqqContactListPrivate LwqqContactListPrivate;

struct _LwqqContactListClass {
    TpBaseContactListClass parent_class;
};

struct _LwqqContactList {
    TpBaseContactList parent;
    LwqqContactListPrivate *priv;
};

GType lwqq_contact_list_get_type (void);

/* TYPE MACROS */
#define LWQQ_TYPE_CONTACT_LIST \
  (lwqq_contact_list_get_type ())
#define LWQQ_CONTACT_LIST(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), LWQQ_TYPE_CONTACT_LIST, \
                              LwqqContactList))
#define LWQQ_CONTACT_LIST_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), LWQQ_TYPE_CONTACT_LIST, \
                           LwqqContactListClass))
#define LWQQ_IS_CONTACT_LIST(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), LWQQ_TYPE_CONTACT_LIST))
#define LWQQ_IS_CONTACT_LIST_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), LWQQ_TYPE_CONTACT_LIST))
#define LWQQ_CONTACT_LIST_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), LWQQ_CONTACT_LIST, \
                              LwqqContactListClass))

G_END_DECLS

void lwqq_contact_list_add_buddy(LwqqClient* lc,LwqqBuddy* buddy);
const TpPresenceStatusSpec* lwqq_contact_list_presence_statuses();
guint lwqq_contact_list_get_presence(LwqqContactList* self,TpHandle contact);
#endif
