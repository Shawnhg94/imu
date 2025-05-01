#ifndef _MANAGER_H_
#define _MANAGER_H_
typedef struct _PhidgetManager *PhidgetManagerHandle;

/* Methods */
API_PRETURN_HDR PhidgetManager_create(PhidgetManagerHandle *man);
API_PRETURN_HDR PhidgetManager_delete(PhidgetManagerHandle *man);
API_PRETURN_HDR PhidgetManager_close(PhidgetManagerHandle man);
API_PRETURN_HDR PhidgetManager_open(PhidgetManagerHandle man);

/* Properties */

/* Events */
typedef void (CCONV *PhidgetManager_OnAttachCallback)(PhidgetManagerHandle man, void *ctx,
  PhidgetHandle channel);

API_PRETURN_HDR PhidgetManager_setOnAttachHandler(PhidgetManagerHandle man,
  PhidgetManager_OnAttachCallback fptr, void *ctx);
typedef void (CCONV *PhidgetManager_OnDetachCallback)(PhidgetManagerHandle man, void *ctx,
  PhidgetHandle channel);

API_PRETURN_HDR PhidgetManager_setOnDetachHandler(PhidgetManagerHandle man,
  PhidgetManager_OnDetachCallback fptr, void *ctx);

#endif /* _MANAGER_H_ */
