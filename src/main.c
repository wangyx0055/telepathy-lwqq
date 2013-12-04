//#include "config.h"

#include <glib.h>
#include <telepathy-glib/run.h>
#include <telepathy-glib/debug.h>
#include <telepathy-glib/debug-sender.h>

#include <lwqq/logger.h>

#include "defines.h"
#include "connection-manager.h"


static void to_debug_sender(int l,const char* msg)
{
	TpDebugSender *dbg;
	GTimeVal now;

	dbg = tp_debug_sender_dup ();
	g_get_current_time (&now);
	tp_debug_sender_add_message (dbg, &now, "webqq",G_LOG_LEVEL_DEBUG, msg);
    
	g_object_unref (dbg);
}

void tp_lwqq_debug_init()
{
    tp_debug_set_flags("all");

	if (g_getenv("LWQQ_PERSIST") != NULL)
		tp_debug_set_persistent(TRUE);
}

static TpBaseConnectionManager* get_cm()
{
   return g_object_new(LWQQ_TYPE_CONNECTION_MANAGER, NULL);
}

int main(int argc,char **argv)
{
    int ret = 0;
    TpDebugSender* debug;
    g_set_prgname(UI_ID);

    tp_debug_divert_messages(g_getenv("LWQQ_LOGFILE"));
    tp_lwqq_debug_init();
    //lwqq_log_redirect(to_debug_sender);
    lwqq_log_set_level(3);

    ret = tp_run_connection_manager(UI_ID,PACKAGE_VERSION,get_cm,argc,argv);

    return ret;
}
