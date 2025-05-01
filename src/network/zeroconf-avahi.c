/*
 * This file is part of libphidget22
 *
 * Copyright (c) 2015-2024 Phidgets Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _PHIDGET_NETWORKCODE

#include "phidgetbase.h"
#include "zeroconf.h"

#include "avahi-client/client.h"
#include "avahi-client/publish.h"
#include "avahi-common/error.h"
#include "avahi-common/malloc.h"
#include "avahi-common/thread-watch.h"

#include "avahi-client/lookup.h"
#include "avahi-common/alternative.h"

#include <dlfcn.h>

static AvahiThreadedPoll *threaded_poll;
static AvahiClient *client;

static int initialized;
static int zcstarted;
static void *libHandle;
static int avstate;

static void client_callback(AvahiClient *, AvahiClientState, AVAHI_GCC_UNUSED void *);

static void listener_dispatch_thread(void *arg);

static void register_service(ZeroconfPublishHandle hdl);
static void deregister_service(ZeroconfPublishHandle hdl);
static void register_services(void);
static void deregister_services(void);

static void register_listener(ZeroconfListenerHandle hdl);
static void deregister_listener(ZeroconfListenerHandle hdl);
static void register_listeners(void);
static void deregister_listeners(void);

typedef struct _wrapper {
	PhidgetReturnCode res;
	int more;
	ZeroconfListenerHandle handle;
	void *arg0;
	void *arg1;
	uint16_t port;
	mos_mutex_t lock;
	mos_cond_t cond;
} wrapper_t;

typedef struct zcdisp {
	ZeroconfListener_t listener;
	void *handle;
	void *ctx;
	int added;
	int interface;
	Zeroconf_Protocol protocol;
	char *name;
	char *host;
	char *type;
	char *domain;
	MSLIST_ENTRY(zcdisp)
	link;
} zcdisp_t;

MSLIST_HEAD(displisthead, zcdisp)
displist = MSLIST_HEAD_INITIALIZER(displist);
static int disprunning;
static mos_mutex_t displock;
static mos_cond_t dispcond;

MSLIST_HEAD(publishlisthead, _ZeroconfPublish)
publishlist = MSLIST_HEAD_INITIALIZER(publishlist);
static mos_mutex_t publishlistlock;

MSLIST_HEAD(listenerlisthead, _ZeroconfListener)
listenerlist = MSLIST_HEAD_INITIALIZER(listenerlist);
static mos_mutex_t listenerlistlock;

#ifdef ZEROCONF_RUNTIME_LINKING
typedef AvahiClient *(*avahi_client_new_t)(const AvahiPoll *, AvahiClientFlags, AvahiClientCallback, void *,
										   int *);
typedef void (*avahi_client_free_t)(AvahiClient *);
typedef const char *(*avahi_client_get_host_name_t)(AvahiClient *);
typedef AvahiServiceBrowser *(*avahi_service_browser_new_t)(AvahiClient *, AvahiIfIndex, AvahiProtocol,
															const char *, const char *, AvahiLookupFlags, AvahiServiceBrowserCallback, void *);
typedef int (*avahi_service_browser_free_t)(AvahiServiceBrowser *);
typedef AvahiServiceResolver *(*avahi_service_resolver_new_t)(AvahiClient *, AvahiIfIndex, AvahiProtocol,
															  const char *, const char *, const char *, AvahiProtocol, AvahiLookupFlags, AvahiServiceResolverCallback,
															  void *);
typedef int (*avahi_service_resolver_free_t)(AvahiServiceResolver *);
typedef AvahiRecordBrowser *(*avahi_record_browser_new_t)(AvahiClient *, AvahiIfIndex, AvahiProtocol,
														  const char *, uint16_t, uint16_t, AvahiLookupFlags, AvahiRecordBrowserCallback, void *);
typedef int (*avahi_record_browser_free_t)(AvahiRecordBrowser *);
typedef int (*avahi_service_name_join_t)(char *, size_t, const char *, const char *, const char *);
typedef const char *(*avahi_strerror_t)(int);
typedef int (*avahi_client_errno_t)(AvahiClient *);
typedef AvahiThreadedPoll *(*avahi_threaded_poll_new_t)(void);
typedef const AvahiPoll *(*avahi_threaded_poll_get_t)(AvahiThreadedPoll *);
typedef void (*avahi_threaded_poll_free_t)(AvahiThreadedPoll *);
typedef int (*avahi_threaded_poll_start_t)(AvahiThreadedPoll *);
typedef int (*avahi_threaded_poll_stop_t)(AvahiThreadedPoll *);
typedef int (*avahi_threaded_poll_quit_t)(AvahiThreadedPoll *);
typedef void (*avahi_threaded_poll_lock_t)(AvahiThreadedPoll *);
typedef void (*avahi_threaded_poll_unlock_t)(AvahiThreadedPoll *);
typedef const char *(*avahi_client_get_version_string_t)(AvahiClient *);
typedef void (*avahi_free_t)(void *p);
typedef AvahiStringList *(*avahi_string_list_new_t)(const char *, ...);
typedef void (*avahi_string_list_free_t)(AvahiStringList *);
typedef AvahiStringList *(*avahi_string_list_get_next_t)(AvahiStringList *);
typedef int (*avahi_string_list_get_pair_t)(AvahiStringList *, char **, char **, size_t *);
typedef AvahiStringList *(*avahi_string_list_add_pair_t)(AvahiStringList *, const char *, const char *);
typedef AvahiEntryGroup *(*avahi_entry_group_new_t)(AvahiClient *, AvahiEntryGroupCallback, void *);
typedef int (*avahi_entry_group_free_t)(AvahiEntryGroup *);
typedef int (*avahi_entry_group_commit_t)(AvahiEntryGroup *);
typedef int (*avahi_entry_group_reset_t)(AvahiEntryGroup *);
typedef AvahiClient *(*avahi_entry_group_get_client_t)(AvahiEntryGroup *);
typedef int (*avahi_entry_group_get_state_t)(AvahiEntryGroup *);
typedef int (*avahi_entry_group_is_empty_t)(AvahiEntryGroup *);
typedef int (*avahi_entry_group_add_service_t)(AvahiEntryGroup *, AvahiIfIndex, AvahiProtocol,
											   AvahiPublishFlags, const char *, const char *, const char *, const char *, uint16_t, ...);
typedef int (*avahi_entry_group_add_service_strlst_t)(AvahiEntryGroup *, AvahiIfIndex, AvahiProtocol,
													  AvahiPublishFlags, const char *, const char *, const char *, const char *, uint16_t, AvahiStringList *);
typedef char *(*avahi_alternative_service_name_t)(const char *);

static avahi_service_browser_new_t _service_browser_new;
static avahi_service_browser_free_t _service_browser_free;
static avahi_service_resolver_new_t _service_resolver_new;
static avahi_service_resolver_free_t _service_resolver_free;
static avahi_record_browser_new_t _record_browser_new;
static avahi_record_browser_free_t _record_browser_free;
static avahi_service_name_join_t _service_name_join;
static avahi_client_new_t _client_new;
static avahi_client_free_t _client_free;
static avahi_strerror_t _strerror;
static avahi_client_errno_t _client_errno;
static avahi_threaded_poll_new_t _threaded_poll_new;
static avahi_threaded_poll_get_t _threaded_poll_get;
static avahi_threaded_poll_free_t _threaded_poll_free;
static avahi_threaded_poll_start_t _threaded_poll_start;
static avahi_threaded_poll_stop_t _threaded_poll_stop;
static avahi_threaded_poll_quit_t _threaded_poll_quit;
static avahi_threaded_poll_lock_t _threaded_poll_lock;
static avahi_threaded_poll_unlock_t _threaded_poll_unlock;
static avahi_client_get_version_string_t _client_get_version_string;
static avahi_free_t _free;
static avahi_string_list_new_t _string_list_new;
static avahi_string_list_free_t _string_list_free;
static avahi_string_list_get_next_t _string_list_get_next;
static avahi_string_list_get_pair_t _string_list_get_pair;
static avahi_string_list_add_pair_t _string_list_add_pair;
static avahi_entry_group_new_t _entry_group_new;
static avahi_entry_group_free_t _entry_group_free;
static avahi_entry_group_commit_t _entry_group_commit;
static avahi_entry_group_reset_t _entry_group_reset;
static avahi_entry_group_add_service_t _entry_group_add_service;
static avahi_entry_group_add_service_strlst_t _entry_group_add_service_strlst;
static avahi_entry_group_get_client_t _entry_group_get_client;
static avahi_entry_group_get_state_t _entry_group_get_state;
static avahi_entry_group_is_empty_t _entry_group_is_empty;

static PhidgetReturnCode
ZeroconfLoad() {

	if (initialized)
		return (EPHIDGET_OK);

	#define CK(stmt)                                          \
		do {                                                  \
			if ((stmt) == NULL) {                             \
				logwarn("'%s' failed: %s", #stmt, dlerror()); \
				return (EPHIDGET_UNEXPECTED);                 \
			}                                                 \
		} while (0)

	libHandle = dlopen("libavahi-client.so", RTLD_LAZY);
	if (!libHandle) {
		libHandle = dlopen("libavahi-client.so.3", RTLD_LAZY);
		if (!libHandle) {
			logwarn("dlopen() failed: %s", dlerror());
			logwarn("Zeroconf is not supported");
			return (EPHIDGET_UNSUPPORTED);
		}
	}

	CK(_service_browser_new = (avahi_service_browser_new_t)dlsym(libHandle, "avahi_service_browser_new"));
	CK(_service_browser_free = (avahi_service_browser_free_t)dlsym(libHandle, "avahi_service_browser_free"));
	CK(_service_resolver_new = (avahi_service_resolver_new_t)dlsym(libHandle, "avahi_service_resolver_new"));
	CK(_service_resolver_free = (avahi_service_resolver_free_t)dlsym(libHandle, "avahi_service_resolver_free"));
	CK(_record_browser_new = (avahi_record_browser_new_t)dlsym(libHandle, "avahi_record_browser_new"));
	CK(_record_browser_free = (avahi_record_browser_free_t)dlsym(libHandle, "avahi_record_browser_free"));
	CK(_service_name_join = (avahi_service_name_join_t)dlsym(libHandle, "avahi_service_name_join"));
	CK(_client_new = (avahi_client_new_t)dlsym(libHandle, "avahi_client_new"));
	CK(_client_free = (avahi_client_free_t)dlsym(libHandle, "avahi_client_free"));
	CK(_strerror = (avahi_strerror_t)dlsym(libHandle, "avahi_strerror"));
	CK(_client_errno = (avahi_client_errno_t)dlsym(libHandle, "avahi_client_errno"));
	CK(_threaded_poll_new = (avahi_threaded_poll_new_t)dlsym(libHandle, "avahi_threaded_poll_new"));
	CK(_threaded_poll_get = (avahi_threaded_poll_get_t)dlsym(libHandle, "avahi_threaded_poll_get"));
	CK(_threaded_poll_free = (avahi_threaded_poll_free_t)dlsym(libHandle, "avahi_threaded_poll_free"));
	CK(_threaded_poll_start = (avahi_threaded_poll_start_t)dlsym(libHandle, "avahi_threaded_poll_start"));
	CK(_threaded_poll_stop = (avahi_threaded_poll_stop_t)dlsym(libHandle, "avahi_threaded_poll_stop"));
	CK(_threaded_poll_quit = (avahi_threaded_poll_quit_t)dlsym(libHandle, "avahi_threaded_poll_quit"));
	CK(_threaded_poll_lock = (avahi_threaded_poll_lock_t)dlsym(libHandle, "avahi_threaded_poll_lock"));
	CK(_threaded_poll_unlock = (avahi_threaded_poll_lock_t)dlsym(libHandle, "avahi_threaded_poll_unlock"));
	CK(_client_get_version_string = (avahi_client_get_version_string_t)dlsym(libHandle, "avahi_client_get_version_string"));
	CK(_free = (avahi_free_t)dlsym(libHandle, "avahi_free"));
	CK(_string_list_new = (avahi_string_list_new_t)dlsym(libHandle, "avahi_string_list_new"));
	CK(_string_list_free = (avahi_string_list_free_t)dlsym(libHandle, "avahi_string_list_free"));
	CK(_string_list_get_next = (avahi_string_list_get_next_t)dlsym(libHandle, "avahi_string_list_get_next"));
	CK(_string_list_get_pair = (avahi_string_list_get_pair_t)dlsym(libHandle, "avahi_string_list_get_pair"));
	CK(_string_list_add_pair = (avahi_string_list_add_pair_t)dlsym(libHandle, "avahi_string_list_add_pair"));
	CK(_entry_group_new = (avahi_entry_group_new_t)dlsym(libHandle, "avahi_entry_group_new"));
	CK(_entry_group_free = (avahi_entry_group_free_t)dlsym(libHandle, "avahi_entry_group_free"));
	CK(_entry_group_commit = (avahi_entry_group_commit_t)dlsym(libHandle, "avahi_entry_group_commit"));
	CK(_entry_group_reset = (avahi_entry_group_reset_t)dlsym(libHandle, "avahi_entry_group_reset"));
	CK(_entry_group_add_service = (avahi_entry_group_add_service_t)dlsym(libHandle, "avahi_entry_group_add_service"));
	CK(_entry_group_add_service_strlst = (avahi_entry_group_add_service_strlst_t)dlsym(libHandle, "avahi_entry_group_add_service_strlst"));
	CK(_entry_group_get_client = (avahi_entry_group_get_client_t)dlsym(libHandle, "avahi_entry_group_get_client"));
	CK(_entry_group_get_state = (avahi_entry_group_get_state_t)dlsym(libHandle, "avahi_entry_group_get_state"));
	CK(_entry_group_is_empty = (avahi_entry_group_is_empty_t)dlsym(libHandle, "avahi_entry_group_is_empty"));

	return (EPHIDGET_OK);
}

#else /* ZEROCONF_RUNTIME_LINKING */

	#define _service_browser_new			avahi_service_browser_new
	#define _service_browser_free			avahi_service_browser_free
	#define _service_resolver_new			avahi_service_resolver_new
	#define _service_resolver_free			avahi_service_resolver_free
	#define _record_browser_new				avahi_record_browser_new
	#define _record_browser_free			avahi_record_browser_free
	#define _service_name_join				avahi_service_name_join
	#define _client_new						avahi_client_new
	#define _client_free					avahi_client_free
	#define _strerror						avahi_strerror
	#define _client_errno					avahi_client_errno
	#define _threaded_poll_new				avahi_threaded_poll_new
	#define _threaded_poll_get				avahi_threaded_poll_get
	#define _threaded_poll_free				avahi_threaded_poll_free
	#define _threaded_poll_start			avahi_threaded_poll_start
	#define _threaded_poll_stop				avahi_threaded_poll_stop
	#define _threaded_poll_quit				avahi_threaded_poll_quit
	#define _threaded_poll_lock				avahi_threaded_poll_lock
	#define _threaded_poll_unlock			avahi_threaded_poll_unlock
	#define _client_get_version_string		avahi_client_get_version_string
	#define _free							avahi_free
	#define _string_list_new				avahi_string_list_new
	#define _string_list_free				avahi_string_list_free
	#define _string_list_get_next			avahi_string_list_get_next
	#define _string_list_get_pair			avahi_string_list_get_pair
	#define _string_list_add_pair			avahi_string_list_add_pair
	#define _entry_group_new				avahi_entry_group_new
	#define _entry_group_free				avahi_entry_group_free
	#define _entry_group_commit				avahi_entry_group_commit
	#define _entry_group_get_client			avahi_entry_group_get_client
	#define _entry_group_get_state			avahi_entry_group_get_state
	#define _entry_group_is_empty			avahi_entry_group_is_empty
	#define _entry_group_reset				avahi_entry_group_reset
	#define _entry_group_add_service		avahi_entry_group_add_service
	#define _entry_group_add_service_strlst avahi_entry_group_add_service_strlst

#endif /* ZEROCONF_RUNTIME_LINKING */

void ZeroconfInit() {

	mos_glock((void *)1);
	if (initialized) {
		mos_gunlock((void *)1);
		return;
	}

	mos_mutex_init(&displock);
	mos_cond_init(&dispcond);
	mos_mutex_init(&publishlistlock);
	mos_mutex_init(&listenerlistlock);

#ifdef ZEROCONF_RUNTIME_LINKING
	if (ZeroconfLoad() != 0) {
		mos_gunlock((void *)1);
		goto bad;
	}
#endif

	initialized = 1;
	mos_gunlock((void *)1);
	return;

#ifdef ZEROCONF_RUNTIME_LINKING
bad:
	mos_mutex_destroy(&displock);
	mos_cond_destroy(&dispcond);
	mos_mutex_destroy(&publishlistlock);
	mos_mutex_destroy(&listenerlistlock);
#endif
}

void ZeroconfFini() {

	ZeroconfStop();

	mos_glock((void *)1);
	if (!initialized || zcstarted) {
		mos_gunlock((void *)1);
		return;
	}
	mos_gunlock((void *)1);

	initialized = 0;

	mos_mutex_destroy(&displock);
	mos_cond_destroy(&dispcond);
	mos_mutex_destroy(&publishlistlock);
	mos_mutex_destroy(&listenerlistlock);

#ifdef ZEROCONF_RUNTIME_LINKING
	dlclose(libHandle);
#endif
}

void ZeroconfStart() {
	mos_task_t task;
	int error;

	mos_glock((void *)1);
	if (!initialized || zcstarted) {
		mos_gunlock((void *)1);
		return;
	}
	zcstarted = 1;
	mos_gunlock((void *)1);

	threaded_poll = _threaded_poll_new();
	if (threaded_poll == NULL) {
		logerr("Failed to create Avahi poll object");
		goto bad;
	}

	/*
	 * Locking shouldn't be required here, but we are being paranoid with Avahi.
	 */
	_threaded_poll_lock(threaded_poll);
	client = _client_new(_threaded_poll_get(threaded_poll), AVAHI_CLIENT_NO_FAIL, client_callback, NULL, &error);
	_threaded_poll_unlock(threaded_poll);
	if (client == NULL) {
		logerr("Failed to create Avahi client: %s", _strerror(error));
		goto bad;
	}

	if (_threaded_poll_start(threaded_poll)) {
		logerr("Failed to start threaded_poll");
		goto bad;
	}

	disprunning = 1;
	if (mos_task_create(&task, listener_dispatch_thread, NULL) != 0) {
		logerr("Failed to create mDNS listener dispatch thread");
		goto bad;
	}

	return;

bad:
	mos_glock((void *)1);
	if (threaded_poll) {
		_threaded_poll_stop(threaded_poll);
		if (client) {
			_client_free(client);
			client = NULL;
		}
		_threaded_poll_free(threaded_poll);
		threaded_poll = NULL;
	}
	disprunning = 0;
	zcstarted = 0;
	mos_gunlock((void *)1);
}

void ZeroconfStop() {

	mos_glock((void *)1);
	if (!initialized || !zcstarted) {
		mos_gunlock((void *)1);
		return;
	}
	zcstarted = 0;
	mos_gunlock((void *)1);

	if (threaded_poll) {
		_threaded_poll_stop(threaded_poll);
		if (client) {
			_client_free(client);
			client = NULL;
		}
		_threaded_poll_free(threaded_poll);
		threaded_poll = NULL;
	}

	mos_mutex_lock(&displock);
	if (disprunning == 1) {
		disprunning = 0;
		while (disprunning != -1)
			mos_cond_wait(&dispcond, &displock);
	}
	mos_mutex_unlock(&displock);
}

static void
client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void *userdata) {
	int err;

	assert(c);

	/* Called whenever the client or server state changes */

	avstate = state;

	switch (state) {
	case AVAHI_CLIENT_S_RUNNING:
		/* The server has startup successfully and registered its host
		 * name on the network. Now we can register services.
		 */
		logdebug("Avahi server running");
		register_listeners();
		register_services();
		break;
	case AVAHI_CLIENT_FAILURE:
		/* Some kind of error happened on the client side */
		err = _client_errno(c);

		if (err == AVAHI_ERR_DISCONNECTED) {
			logwarn("Avahi server disconnection - creating replacement client");

			deregister_listeners();
			deregister_services();

			// We must free and recreate the client
			_client_free(client);
			client = _client_new(_threaded_poll_get(threaded_poll), AVAHI_CLIENT_NO_FAIL, client_callback, NULL, &err);

			if (!client) {
				// Networking must be restarted to recover from this
				logerr("Failed to create replacement Avahi client : %s", _strerror(err));
				_threaded_poll_quit(threaded_poll);
			}
		} else {
			// Networking must be restarted to recover from this
			logerr("Avahi client failure: %s", _strerror(err));
			_threaded_poll_quit(threaded_poll);
		}
		break;
	case AVAHI_CLIENT_S_COLLISION:
		/* Let's drop our registered services. When the server is back
		 * in AVAHI_SERVER_RUNNING state we will register them
		 * again with the new host name.
		 */
		logwarn("Avahi server hostname collision");
		deregister_services();
		break;
	case AVAHI_CLIENT_S_REGISTERING:
		/* The server records are now being established. This
		 * might be caused by a host name change. We need to wait
		 * until the host name is properly established before
		 * registering our own services.
		 */
		logdebug("Avahi server registering");
		deregister_services();
		break;
	case AVAHI_CLIENT_CONNECTING:
		/* We're still connecting. This state is only entered when AVAHI_CLIENT_NO_FAIL
		 * has been passed to avahi_client_new() and the daemon is not yet available.
		 */
		logdebug("Avahi client connecting - waiting for server");
		break;
	}
}

/**********
 * Browse *
 **********/

static void
listener_dispatch_thread(void *arg) {
	zcdisp_t *dp;

	mos_mutex_lock(&displock);
	while (disprunning) {
		if (MSLIST_EMPTY(&displist)) {
			mos_cond_timedwait(&dispcond, &displock, MOS_SEC);
			continue;
		}

		dp = MSLIST_FIRST(&displist);
		MSLIST_REMOVE(&displist, dp, zcdisp, link);
		mos_mutex_unlock(&displock);
		dp->listener(dp->handle, dp->ctx, dp->added, dp->interface, dp->protocol, dp->name, dp->host,
					 dp->type, dp->domain);
		mos_free(dp->name, MOSM_FSTR);
		mos_free(dp->host, MOSM_FSTR);
		mos_free(dp->type, MOSM_FSTR);
		mos_free(dp->domain, MOSM_FSTR);
		mos_free(dp, sizeof(*dp));
		mos_mutex_lock(&displock);
	}
	disprunning = -1;
	mos_cond_broadcast(&dispcond);
	mos_mutex_unlock(&displock);
	mos_task_exit(0);
}

static void
dispatch(ZeroconfListener_t listener, void *handle, void *ctx, int added, int interface,
		 Zeroconf_Protocol proto, const char *name, const char *host, const char *type, const char *domain) {
	zcdisp_t *dp;

	dp = mos_malloc(sizeof(*dp));
	dp->listener = listener;
	dp->handle = handle;
	dp->ctx = ctx;
	dp->added = added;
	dp->interface = interface;
	dp->protocol = proto;
	dp->name = mos_strdup(name, NULL);
	dp->host = mos_strdup(host, NULL);
	dp->type = mos_strdup(type, NULL);
	dp->domain = mos_strdup(domain, NULL);

	mos_mutex_lock(&displock);
	MSLIST_INSERT_HEAD(&displist, dp, link);
	mos_cond_broadcast(&dispcond);
	mos_mutex_unlock(&displock);
}

static void
DNSServiceBrowse_Callback(AvahiServiceBrowser *b, AvahiIfIndex interface, AvahiProtocol protocol,
						  AvahiBrowserEvent event, const char *name, const char *type, const char *domain,
						  AVAHI_GCC_UNUSED AvahiLookupResultFlags flags, void *ctx) {
	ZeroconfListenerHandle handle;

	handle = ctx;

	switch (event) {
	case AVAHI_BROWSER_FAILURE:
		logwarn("Avahi browse failure: %s", _strerror(_client_errno(client)));
		return;

	case AVAHI_BROWSER_NEW:
		logdebug("AVAHI_BROWSER_NEW '%s.%s.%s' (interace %d) (proto %d)", name, type, domain, interface, protocol);
		dispatch(handle->listener, handle, handle->listenerctx, 1, interface, protocol, name, name,
				 type, domain);
		break;

	case AVAHI_BROWSER_REMOVE:
		logdebug("AVAHI_BROWSER_REMOVE '%s.%s.%s'", name, type, domain);
		dispatch(handle->listener, handle, handle->listenerctx, 0, interface, protocol, name, name,
				 type, domain);
		break;

	case AVAHI_BROWSER_ALL_FOR_NOW:
	case AVAHI_BROWSER_CACHE_EXHAUSTED:
		logverbose("%s", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
		break;
	}
}

// NOTE: must _threaded_poll_lock(threaded_poll) unless calling from an avahi callback
static void register_listener(ZeroconfListenerHandle hdl) {

	// Can only register when the server is running
	if (avstate != AVAHI_CLIENT_S_RUNNING)
		return;

	// Already registered
	if (hdl->sb)
		return;

	loginfo("Registering service browser '%s'", hdl->type);

	// For now, we are only looking up IPv4 addresses
	hdl->sb = _service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_INET, hdl->type, NULL, 0,
								   DNSServiceBrowse_Callback, hdl);

	if (hdl->sb == NULL) {
		logerr("Failed to create service browser '%s': %s", hdl->type, _strerror(_client_errno(client)));
	}
}

// NOTE: must _threaded_poll_lock(threaded_poll) unless calling from an avahi callback
static void deregister_listener(ZeroconfListenerHandle hdl) {

	if (hdl->sb) {
		loginfo("Deregistering service browser '%s'", hdl->type);
		_service_browser_free(hdl->sb);
		hdl->sb = NULL;
	}
}

static void register_listeners(void) {
	ZeroconfListenerHandle hdl;

	mos_mutex_lock(&listenerlistlock);
	hdl = MSLIST_FIRST(&listenerlist);
	while (hdl != NULL) {
		register_listener(hdl);
		hdl = MSLIST_NEXT(hdl, link);
	}
	mos_mutex_unlock(&listenerlistlock);
}

static void deregister_listeners(void) {
	ZeroconfListenerHandle hdl;

	mos_mutex_lock(&listenerlistlock);
	hdl = MSLIST_FIRST(&listenerlist);
	while (hdl != NULL) {
		deregister_listener(hdl);
		hdl = MSLIST_NEXT(hdl, link);
	}
	mos_mutex_unlock(&listenerlistlock);
}

PhidgetReturnCode
Zeroconf_listen(ZeroconfListenerHandle *_handle, const char *type, ZeroconfListener_t fptr, void *ctx) {
	ZeroconfListenerHandle handle;

	if (client == NULL) {
		logerr("Avahi client is not initialized");
		return (EPHIDGET_UNEXPECTED);
	}

	handle = mos_malloc(sizeof(*handle));
	handle->flags = ZCL_RUN;
	handle->listener = fptr;
	handle->listenerctx = ctx;
	handle->type = mos_strdup(type, NULL);

	mos_mutex_lock(&listenerlistlock);
	MSLIST_INSERT_HEAD(&listenerlist, (handle), link);
	mos_mutex_unlock(&listenerlistlock);

	_threaded_poll_lock(threaded_poll);
	register_listener(handle);
	_threaded_poll_unlock(threaded_poll);

	*_handle = handle;

	return (EPHIDGET_OK);
}

void Zeroconf_listenclose(ZeroconfListenerHandle *_handle) {
	ZeroconfListenerHandle handle;

	if (_handle == NULL || *_handle == NULL)
		return;

	handle = *_handle;

	mos_mutex_lock(&listenerlistlock);
	MSLIST_REMOVE(&listenerlist, handle, _ZeroconfListener, link);
	mos_mutex_unlock(&listenerlistlock);

	_threaded_poll_lock(threaded_poll);
	deregister_listener(handle);
	_threaded_poll_unlock(threaded_poll);

	mos_free(handle->type, MOSM_FSTR);
	mos_free(handle, sizeof(*handle));

	*_handle = NULL;
}

/***********
 * Resolve *
 ***********/

static void
DNSServiceResolve_Callback(AvahiServiceResolver *r, AVAHI_GCC_UNUSED AvahiIfIndex interface,
						   AVAHI_GCC_UNUSED AvahiProtocol protocol, AvahiResolverEvent event, const char *name, const char *type,
						   const char *domain, const char *host_name, const AvahiAddress *address, uint16_t port, AvahiStringList *txt,
						   AvahiLookupResultFlags flags, void *ctx) {
	mos_sockaddr_list_t *addri, **next;
	mos_sockaddr_list_t **addrlist;
	AvahiStringList *cur;
	wrapper_t *wrapper;
	char *key, *val;
	kv_t **_kv, *kv;
	size_t size;

	wrapper = ctx;
	addrlist = wrapper->arg0;
	_kv = wrapper->arg1;

	mos_mutex_lock(&wrapper->lock);

	switch (event) {
	case AVAHI_RESOLVER_FAILURE:
		logwarn("Failed to resolve service '%s.%s.%s': %s", name, type, domain, _strerror(_client_errno(client)));
		wrapper->res = EPHIDGET_UNEXPECTED;
		break;
	case AVAHI_RESOLVER_FOUND:
		if (addrlist) {
			/* XXX - IPv6?
			 We may wish to pass along IPv6 addresses at some point..
			 but make sure we don't try to use them to connect to a server
			 unless we verify that it's version is new enough to support it
			 */
			if (address->proto != AVAHI_PROTO_INET) {
				wrapper->res = EPHIDGET_UNSUPPORTED;
				break;
			}
			addri = mos_malloc(sizeof(mos_sockaddr_list_t));
			memset(addri, 0, sizeof(mos_sockaddr_list_t));

			if (*addrlist == NULL) {
				*addrlist = addri;
			} else {
				next = &(*addrlist)->next;
				while (*next)
					next = &(*next)->next;
				*next = addri;
			}

			addri->family = AF_INET;
			addri->addr.s4.sin_family = AF_INET;
			addri->addr.s4.sin_addr.s_addr = address->data.ipv4.address;
			addri->addr.s4.sin_port = port;
		}
		if (_kv) {
			if (txt != NULL) {
				newkv(&kv);
				cur = txt;
				do {
					_string_list_get_pair(cur, &key, &val, &size);
					if (val)
						kvset(kv, MOS_IOP_IGNORE, key, val);
					_free(key);
					if (val)
						_free(val);
				} while ((cur = _string_list_get_next(cur)) != NULL);
				*_kv = kv;
			}
		}
		wrapper->port = port;
		wrapper->res = EPHIDGET_OK;
		break;
	default:
		wrapper->res = EPHIDGET_UNEXPECTED;
	}

	wrapper->more = 0;
	mos_cond_broadcast(&wrapper->cond);
	mos_mutex_unlock(&wrapper->lock);
}

PhidgetReturnCode
Zeroconf_addr_lookup(const char *host, Zeroconf_Protocol proto, mos_sockaddr_list_t **addrlist) {

	// XXX - maybe support at some point
	return (EPHIDGET_UNSUPPORTED);
}

PhidgetReturnCode
Zeroconf_lookup(ZeroconfListenerHandle handle, int interface, Zeroconf_Protocol proto, const char *name,
				const char *host, const char *type, const char *domain, Zeroconf_Protocol reqproto, mos_sockaddr_list_t **addrlist,
				uint16_t *port, kv_t **kv) {
	AvahiServiceResolver *r;
	PhidgetReturnCode res;
	wrapper_t wrapper;

	if (client == NULL) {
		logerr("Avahi client is not initialized");
		return (EPHIDGET_UNEXPECTED);
	}

	_threaded_poll_lock(threaded_poll);
	if (avstate != AVAHI_CLIENT_S_RUNNING) {
		logerr("Avahi client is not connected to server");
		_threaded_poll_unlock(threaded_poll);
		return (EPHIDGET_UNEXPECTED);
	}

	if (addrlist == NULL && kv == NULL)
		return (EPHIDGET_INVALIDARG);
		 
	if (addrlist)
		*addrlist = NULL;

	if (kv)
		*kv = NULL;

	wrapper.handle = handle;
	wrapper.arg0 = addrlist;
	wrapper.arg1 = kv;
	wrapper.more = 1;
	mos_mutex_init(&wrapper.lock);
	mos_cond_init(&wrapper.cond);

	r = _service_resolver_new(client, interface, proto, host, type, domain, reqproto, 0,
							  DNSServiceResolve_Callback, &wrapper);
	_threaded_poll_unlock(threaded_poll);
	if (r == NULL) {
		logerr("avahi_service_resolver_new() failed on service '%s.%s.%s': %s", name, type, domain, _strerror(_client_errno(client)));
		res = EPHIDGET_UNEXPECTED;
		goto error_exit;
	}

	// XXX do we need to worry the resolve just never finishing? Maybe add a timeout?
	mos_mutex_lock(&wrapper.lock);
	while (wrapper.more)
		mos_cond_wait(&wrapper.cond, &wrapper.lock);
	mos_mutex_unlock(&wrapper.lock);

	mos_mutex_destroy(&wrapper.lock);
	mos_cond_destroy(&wrapper.cond);

	// XXX Do we want to leave the resolver running to get notified of TXT updates?
	_threaded_poll_lock(threaded_poll);
	_service_resolver_free(r);
	_threaded_poll_unlock(threaded_poll);

	if (wrapper.res != EPHIDGET_OK) {
		res = wrapper.res;
		goto error_exit;
	}

	*port = wrapper.port;

	return (EPHIDGET_OK);

error_exit:

	if (addrlist && *addrlist) {
		mos_freeaddrlist(*addrlist);
		*addrlist = NULL;
	}

	return (res);
}

/***********
 * Publish *
 ***********/

static void
entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *ctx) {
	ZeroconfPublishHandle hdl;

	hdl = ctx;

	/* Called whenever the entry group state changes */
	switch (state) {
	case AVAHI_ENTRY_GROUP_ESTABLISHED:
		loginfo("Service registered: '%s.%s'", hdl->service_name, hdl->type);
		break;
	case AVAHI_ENTRY_GROUP_COLLISION:
		logwarn("Service collision on '%s.%s' - finding a new name", hdl->service_name, hdl->type);
		hdl->dupsrvname++;
		// NOTE: deregistration happens automatically on collision
		register_service(hdl);
		break;
	case AVAHI_ENTRY_GROUP_FAILURE:
		logerr("Service failure on '%s.%s': %s", hdl->service_name, hdl->type, _strerror(_client_errno(client)));
		break;
	case AVAHI_ENTRY_GROUP_UNCOMMITED:
		logdebug("entry group uncommited: '%s.%s'", hdl->service_name, hdl->type);
		break;
	case AVAHI_ENTRY_GROUP_REGISTERING:
		logdebug("entry group registering: '%s.%s'", hdl->service_name, hdl->type);
		break;
	}
}

// NOTE: must _threaded_poll_lock(threaded_poll) unless calling from an avahi callback
static void register_service(ZeroconfPublishHandle hdl) {
	int ret = 0;

	// Can only register when the server is running
	if (avstate != AVAHI_CLIENT_S_RUNNING)
		return;

	if (hdl->ref == NULL)
		hdl->ref = _entry_group_new(client, entry_group_callback, hdl);

	if (hdl->ref == NULL) {
		logerr("Failed to create entry group");
		return;
	}

	// Already registered
	if (!_entry_group_is_empty(hdl->ref))
		return;

	if (hdl->dupsrvname)
		mos_snprintf(hdl->service_name, sizeof(hdl->service_name), "%s (%d)", hdl->name, (hdl->dupsrvname + 1));
	else
		mos_snprintf(hdl->service_name, sizeof(hdl->service_name), "%s", hdl->name);

	loginfo("Registering service '%s.%s'", hdl->service_name, hdl->type);

	if (hdl->sl) {
		ret = _entry_group_add_service_strlst(hdl->ref, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0,
											  hdl->service_name, hdl->type, NULL, NULL, hdl->port, hdl->sl);
	} else {
		ret = _entry_group_add_service(hdl->ref, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0,
									   hdl->service_name, hdl->type, NULL, NULL, hdl->port, NULL);
	}

	if (ret != 0) {
		logerr("avahi_entry_group_add_service failed on '%s.%s'", hdl->service_name, hdl->type);
		return;
	}

	ret = _entry_group_commit(hdl->ref);
	if (ret != 0) {
		logerr("avahi_entry_group_commit failed on '%s.%s'", hdl->service_name, hdl->type);
		return;
	}
}

// NOTE: must _threaded_poll_lock(threaded_poll) unless calling from an avahi callback
static void deregister_service(ZeroconfPublishHandle hdl) {

	if (hdl->ref != NULL) {
		loginfo("Deregistering service '%s.%s'", hdl->service_name, hdl->type);
		_entry_group_free(hdl->ref);
		hdl->ref = NULL;
	}
}

static void register_services(void) {
	ZeroconfPublishHandle hdl;

	mos_mutex_lock(&publishlistlock);
	hdl = MSLIST_FIRST(&publishlist);
	while (hdl != NULL) {
		register_service(hdl);
		hdl = MSLIST_NEXT(hdl, link);
	}
	mos_mutex_unlock(&publishlistlock);
}

static void deregister_services(void) {
	ZeroconfPublishHandle hdl;

	mos_mutex_lock(&publishlistlock);
	hdl = MSLIST_FIRST(&publishlist);
	while (hdl != NULL) {
		deregister_service(hdl);
		hdl = MSLIST_NEXT(hdl, link);
	}
	mos_mutex_unlock(&publishlistlock);
}

PhidgetReturnCode
Zeroconf_publish(ZeroconfPublishHandle *hdl, const char *name, const char *host, const char *type,
				 int port, kv_t *txtRecords) {
	kvent_t *e;

	TESTPTR(hdl);
	TESTPTR(name);
	TESTPTR(type);

	if (client == NULL) {
		logerr("Avahi client is not initialized");
		return (EPHIDGET_UNEXPECTED);
	}

	*hdl = mos_zalloc(sizeof(**hdl));
	(*hdl)->name = mos_strdup(name, NULL);
	(*hdl)->type = mos_strdup(type, NULL);
	(*hdl)->port = port;
	(*hdl)->dupsrvname = 0;

	if (txtRecords) {
		KV_FOREACH(e, txtRecords)
		(*hdl)->sl = _string_list_add_pair((*hdl)->sl, e->key, e->val);
	}

	mos_mutex_lock(&publishlistlock);
	MSLIST_INSERT_HEAD(&publishlist, (*hdl), link);
	mos_mutex_unlock(&publishlistlock);

	_threaded_poll_lock(threaded_poll);
	register_service(*hdl);
	_threaded_poll_unlock(threaded_poll);

	return (EPHIDGET_OK);
}

PhidgetReturnCode
Zeroconf_unpublish(ZeroconfPublishHandle *hdl) {

	TESTPTR(hdl);

	mos_mutex_lock(&publishlistlock);
	MSLIST_REMOVE(&publishlist, (*hdl), _ZeroconfPublish, link);
	mos_mutex_unlock(&publishlistlock);

	_threaded_poll_lock(threaded_poll);
	deregister_service(*hdl);
	_threaded_poll_unlock(threaded_poll);

	if ((*hdl)->sl)
		_string_list_free((*hdl)->sl);

	mos_free((*hdl)->name, MOSM_FSTR);
	mos_free((*hdl)->type, MOSM_FSTR);
	mos_free(*hdl, sizeof(**hdl));
	*hdl = NULL;

	return (EPHIDGET_OK);
}
