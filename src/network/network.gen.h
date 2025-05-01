#ifndef _NET_H_
#define _NET_H_

/* Methods */
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR PhidgetNet_removeAllServers(void);
#endif
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR PhidgetNet_freeServerAddressList(const char **addressList, uint32_t count);
#endif
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR PhidgetNet_setProperty(const char *key, const char *property, ...);
#endif
API_PRETURN_HDR PhidgetNet_addServer(const char *serverName, const char *address, int port,
  const char *password, int flags);
API_PRETURN_HDR PhidgetNet_removeServer(const char *serverName);
API_PRETURN_HDR PhidgetNet_enableServer(const char *serverName);
API_PRETURN_HDR PhidgetNet_disableServer(const char *serverName, int flags);
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR PhidgetNet_getServerAddressList(const char *hostname, int addressFamily,
  const char **addressList, uint32_t *count);
#endif
API_PRETURN_HDR PhidgetNet_enableServerDiscovery(PhidgetServerType serverType);
API_PRETURN_HDR PhidgetNet_disableServerDiscovery(PhidgetServerType serverType);
API_PRETURN_HDR PhidgetNet_setServerPassword(const char *serverName, const char *password);
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR PhidgetNet_startServer(int flags, int addressFamily, const char *serverName,
  const char *address, int port, const char *password, PhidgetServer **server);
#endif
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR PhidgetNet_stopServer(PhidgetServer **server);
#endif

/* Properties */

/* Events */
typedef void (CCONV *PhidgetNet_OnServerAddedCallback)(void *ctx, PhidgetServer *server, void *kv);

API_PRETURN_HDR PhidgetNet_setOnServerAddedHandler(PhidgetNet_OnServerAddedCallback fptr, void *ctx);
typedef void (CCONV *PhidgetNet_OnServerRemovedCallback)(void *ctx, PhidgetServer *server);

API_PRETURN_HDR PhidgetNet_setOnServerRemovedHandler(PhidgetNet_OnServerRemovedCallback fptr,
  void *ctx);

#endif /* _NET_H_ */
