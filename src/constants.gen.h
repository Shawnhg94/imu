#ifndef _PHIDGETCONSTANTS_GEN_H_
#define _PHIDGETCONSTANTS_GEN_H_

#define PHIDGET_SERIALNUMBER_ANY -1	// Pass to <code>DeviceSerialNumber</code> to open any serial number.
#define PHIDGET_HUBPORT_ANY -1	// Pass to <code>HubPort</code> to open any hub port.
#define PHIDGET_CHANNEL_ANY -1	// Pass to <code>Channel</code> to open any channel.
#define PHIDGET_LABEL_ANY NULL	// Pass to <code>DeviceLabel</code> to open any label.
#define PHIDGET_TIMEOUT_INFINITE 0x0	// Pass to <code>Phidget_openWaitForAttachment()</code> for an infinite timeout.
#define PHIDGET_TIMEOUT_DEFAULT 0x3e8	// Pass to <code>Phidget_openWaitForAttachment()</code> for the default timeout.
#define PHIDGET_HUBPORTSPEED_AUTO 0x0	// Pass to <code>HubPortSpeed</code> to set the Hub Port speed automatically when supported by both the hub port and the VINT device.
#define PHIDGETSERVER_AUTHREQUIRED 0x1	// PhidgetServer flag indicating that the server requires a password to authenticate
#define IR_RAWDATA_LONGSPACE 0xffffffff	// The value for a long space in raw data
#define IR_MAX_CODE_BIT_COUNT 0x80	// Maximum bit count for sent / received data
#define IR_MAX_CODE_STR_LENGTH 0x21	// Maximum bit count for sent / received data

#endif /* _PHIDGETCONSTANTS_GEN_H_ */
