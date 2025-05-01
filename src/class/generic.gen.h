#ifndef _GENERIC_H_
#define _GENERIC_H_
#ifdef INCLUDE_PRIVATE
typedef struct _PhidgetGeneric *PhidgetGenericHandle;

/* Methods */
API_PRETURN_HDR PhidgetGeneric_create(PhidgetGenericHandle *ch);
API_PRETURN_HDR PhidgetGeneric_delete(PhidgetGenericHandle *ch);

/* Properties */

/* Events */

#endif /* INCLUDE_PRIVATE */
#endif /* _GENERIC_H_ */
