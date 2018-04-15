/* usbdpfpi.h - driver interface to client
 *
 * Copyright 1996-2011 DigitalPersona, Inc.  All rights reserved.
 * 
 */

#ifndef __USBDPFPI_H
#define __USBDPFPI_H

#define USBDPFP_IOC_MAGIC 'U'
#define USBDPFP_MAX_EVENT_SIZE 64
#define USBDPFP_NAME_BUFF_SIZE 80
#define USBDPFP_MAX_FRAMES  8


/* USB Char driver (usbdpfp[n])
 *    DigitalPersona fingerprint device
 */

/* CONTROL PIPE: set / get device register */
struct usbdpfp_device_data {
   int          type;             /* NOT USED                             */
   unsigned int address;          /* [IN] device IO map address or offset */
   int          length;           /* [IN] number of btyes xfer            */
   void         *data;            /* [IN or OUT] data number              */
};

/* INTERRUPT PIPE: data stream */
struct usbdpfp_device_event {
   int    size_requested;               /* [IN] numer of bytes requested (64 bytes)*/
   char   data[USBDPFP_MAX_EVENT_SIZE]; /* [OUT] raw data                          */
   int    size_returned;                /* [OUT] size in return                    */
   int    status;                       /* [OUT] urb status in return (0=no error) */
};

/* Image streaming buffer configuration */
struct usbdpfp_channel_info {
   unsigned int ch_id;
   unsigned int max_frames;
   unsigned int bytes_per_frame;
};

/* DEVICE INFORMATION */
struct usbdpfp_device_info {
   unsigned int   idVendor;       /* [OUT] DP vendor ID: 0x5ba                                */
   unsigned int   idProduct;      /* [OUT] Product ID: 0x0007, 000A, ...                      */
   unsigned int   bcdDevice;      /* [OUT] HW Revision: 0x100, 0x101, ...                     */
   char           nameDevice[80]; /* [OUT] Device name: U.are.U4000B Fingerprint Reader       */
   char           snDevice[80];   /* [OUT] Serial number string (converted 8-bit char string):*/
                                  /* e.g "{7C265680-0056-FFFF-680D-A7033B09615}"              */
};

/* IOCTL CODE for device node usbdpfp[n] */
#define USBDPFP_IOCTL_GET_INFO   _IOR(USBDPFP_IOC_MAGIC,  0x20, struct usbdpfp_device_info)
#define USBDPFP_IOCTL_SET_DATA   _IOW(USBDPFP_IOC_MAGIC,  0x21, struct usbdpfp_device_data)
#define USBDPFP_IOCTL_GET_DATA   _IOWR(USBDPFP_IOC_MAGIC, 0x22, struct usbdpfp_device_data)
#define USBDPFP_IOCTL_WAIT_EVENT _IOR(USBDPFP_IOC_MAGIC,  0x23, struct usbdpfp_device_event)
#define USBDPFP_IOCTL_ABORT_WAIT_EVENT    _IO(USBDPFP_IOC_MAGIC,   0x24)
#define USBDPFP_IOCTL_CONFIG_CHANNEL      _IOW(USBDPFP_IOC_MAGIC,  0x25, struct usbdpfp_channel_info)
#define USBDPFP_IOCTL_SET_ACTIVE_CHANNEL  _IOW(USBDPFP_IOC_MAGIC,  0x26, int)
#define USBDPFP_IOCTL_ABORT_BULK_READ     _IO(USBDPFP_IOC_MAGIC,   0x27 ) 


/* Char driver (usbdpfpPnp): 
 *    Internal PnP event notification to user mode FP device manager 
 */

/* PnP EVENT ID */
#define PNP_STATE_ATTACH		0
#define PNP_STATE_DETACH		1
#define PM_STATE_UP				2
#define PM_STATE_DOWN			3

/* PnP EVENT INFORMATION */
struct usbdpfp_device_pnp_event {
   char  dev_file_name[USBDPFP_NAME_BUFF_SIZE];   /* Name of the device file     */
   int   detach_state;                            /* Device inserted or removed  */
   struct usbdpfp_device_info dev_info;           /* Device information          */
};

/* IOCTL CODE for the device node usbdpfpPnp */
#define USBDPFP_IOCTL_WAIT_PNP_EVENT 	_IOR( USBDPFP_IOC_MAGIC, 0x10, struct usbdpfp_device_pnp_event) 
#define USBDPFP_IOCTL_CANCEL_WAIT_PNP_EVENT   _IO( USBDPFP_IOC_MAGIC, 0x11)

#endif
