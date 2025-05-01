#ifndef _LOG_H_
#define _LOG_H_

/* Methods */
API_PRETURN_HDR PhidgetLog_disable(void);
API_PRETURN_HDR PhidgetLog_enable(Phidget_LogLevel level, const char *destination);
API_PRETURN_HDR PhidgetLog_getLevel(Phidget_LogLevel *level);
API_PRETURN_HDR PhidgetLog_setLevel(Phidget_LogLevel level);
API_PRETURN_HDR PhidgetLog_log(Phidget_LogLevel level, const char *message, ...);
API_PRETURN_HDR PhidgetLog_loge(const char *file, int line, const char *function, const char *source,
  Phidget_LogLevel level, const char *message, ...);
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR PhidgetLog_loges(Phidget_LogLevel level, const char *source, const char *message);
#endif
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR PhidgetLog_logs(Phidget_LogLevel level, const char *message);
#endif
API_PRETURN_HDR PhidgetLog_rotate(void);
API_PRETURN_HDR PhidgetLog_isRotating(int *isrotating);
API_PRETURN_HDR PhidgetLog_getRotating(uint64_t *size, int *keepCount);
API_PRETURN_HDR PhidgetLog_setRotating(uint64_t size, int keepCount);
API_PRETURN_HDR PhidgetLog_enableRotating(void);
API_PRETURN_HDR PhidgetLog_disableRotating(void);
API_PRETURN_HDR PhidgetLog_addSource(const char *source, Phidget_LogLevel level);
API_PRETURN_HDR PhidgetLog_getSourceLevel(const char *source, Phidget_LogLevel *level);
API_PRETURN_HDR PhidgetLog_setSourceLevel(const char *source, Phidget_LogLevel level);
API_PRETURN_HDR PhidgetLog_getSources(const char **sources, uint32_t *count);

/* Properties */

/* Events */

#endif /* _LOG_H_ */
