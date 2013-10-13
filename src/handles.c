#include "handles.h"

#include <glib.h>
#include <ctype.h>
#include <string.h>

#include <telepathy-glib/errors.h>
#include <telepathy-glib/handle-repo-dynamic.h>



gchar *lwqq_normalize_nickname (const gchar *id, GError **error) {
	/*
	gchar *normalized;

	if (!idle_nickname_is_valid(id, FALSE)) {
		g_set_error(error, TP_ERROR, TP_ERROR_INVALID_HANDLE, "invalid nickname");
		return NULL;
	}

	normalized = g_utf8_strdown(id, -1);
	*/

	return strdup(id);
}

static gchar *_nick_normalize_func(TpHandleRepoIface *repo, const gchar *id, gpointer ctx, GError **error) {
	return lwqq_normalize_nickname (id, error);
}

void idle_handle_repos_init(TpHandleRepoIface **handles) {
	g_assert(handles != NULL);

	handles[TP_HANDLE_TYPE_CONTACT] = (TpHandleRepoIface *) g_object_new(TP_TYPE_DYNAMIC_HANDLE_REPO,
			"handle-type", TP_HANDLE_TYPE_CONTACT,
			"normalize-function", _nick_normalize_func,
			"default-normalize-context", NULL,
			NULL);

	/*handles[TP_HANDLE_TYPE_ROOM] = (TpHandleRepoIface *) g_object_new(TP_TYPE_DYNAMIC_HANDLE_REPO,
			"handle-type", TP_HANDLE_TYPE_ROOM,
			"normalize-function", _channel_normalize_func,
			"default-normalize-context", NULL,
			NULL);
			*/
}
