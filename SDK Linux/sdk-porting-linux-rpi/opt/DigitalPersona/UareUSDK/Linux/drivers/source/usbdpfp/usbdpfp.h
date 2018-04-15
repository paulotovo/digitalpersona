/* usbdpfp.h - internal driver data structure 
 *
 * Copyright 1996-2011 DigitalPersona, Inc.  All rights reserved.
 * 
 */

#ifndef __USBDPFP_H
#define __USBDPFP_H


// System Headers
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kref.h>
#include <linux/usb.h>
#include <asm/uaccess.h>
//#include <linux/smp_lock.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/mempool.h>


#include "usbdpfpi.h"

// DigitalPersona product information
#define DP_VID          (0x05BA)
#define URU4000         (0x0007)
#define URU4000KB       (0x0008)
#define URU4000B        (0x000A)
#define DRAGONFLY       (0x0010)


//Supports Maximum of 3 channels per Device.
#define USBDPFP_MAX_CHANNELS               3

#define USBDPFP_CHANNEL_CONFIGURED         1
#define USBDPFP_CHANNEL_NOT_CONFIGURED     0

#define USBDPFP_FRAME_INVALID              0  //data has been copied or initial value
#define USBDPFP_FRAME_VALID                1  //contain new data
#define USBDPFP_FRAME_ERROR                2  //error

#define USBDPFP_MAX_FRAME_SIZE             (120*1024) /*TODO fix to a realistic value*/


struct usbdpfp_frame {
     unsigned char *buffer;          	//allocated on demand in the call to read.
     unsigned int   bulk_read_count; 	//set by the callback handler. 
     int            bulk_read_status;	//status of the read data.
     int            short_packet_detected;
     int            valid;           	//is valid when the frame is stored and read.
};

struct usbdpfp_channel_config {

     int ch_id;  // channel id can be implicitly taken from the index of the channels;

     unsigned int max_frames;  // maximum # of frame to be streamed ( < USBDPFP_MAX_FRAMES)
     unsigned int max_bytes_per_frame; // max # of bytes per frame ( < USBDPFP_MAX_FRAME_SIZE)
     struct usbdpfp_frame frame[USBDPFP_MAX_FRAMES]; 
     int valid; // ( USBDPFP_CHANNEL_CONFIGURED or USBDPFP_CHANNEL_NOT_CONFIGURED )

	  // frame buffer circular queue implementation
	  // empty condition: if (frame_write_index - frame_read_index == 0)
	  // full condition: if (frame_write_index - frame_read_index == USBDPFP_MAX_FRAMES-1)	  
     unsigned int frame_write_index;     	// write index (was cur_frame_index)
     unsigned int frame_read_index;     	// current frame index
};

////////////////////////////////////////////////////////////////////////////////////

/**
 * struct usbdpfp_device - Device specific information
 * @sem: probe 
 * @minor: disconnect
 * @isopen: open
 * @udev: get device specific info
 * @interface: set data to driver
 * @inq: get data from driver
 * @bulk_in_ep: delete 
 * @bulk_in_urb: urb
 * @int_in_ep: int in ep
 * @int_in_urb: int in urb
 * @int_in_int: interval
 * @ctrl_out_urb: ctrl urb
 * @ctrl_setup: setup token
 * @private_data: private data
 *
 * Description of structure members
 */
struct usbdpfp_device { 
  
    struct kref kref;

    struct semaphore sem;		/* mutual exclusion semaphore */
    unsigned char minor;	   /* the starting minor number for this device */
    int isopen;		         /* locking for preventing multiple open */

    struct usb_device *udev;	/* save off the usb device pointer */
    struct usb_interface *interface;	/* the interface for this device */

    wait_queue_head_t inq;		/* read queue */
    int disconnected;
   
    /* bulk pipe */
    unsigned char bulk_in_ep;
    struct urb *bulk_in_urb;
    struct semaphore bulk_sem;	  	/* thread-safe access urb*/

    struct usbdpfp_channel_config channel[USBDPFP_MAX_CHANNELS]; 
    struct usbdpfp_channel_config *active_channel; 
    int do_streaming_read;
    atomic_t cancelable_bulk_urb;  	/* flag indicating if the bulk read urb is cancelable */ 
    atomic_t suspended;            	/* was suspended by PM. */

    /* interrupt pipe */
    unsigned char int_in_ep;
    struct urb *int_in_urb;
    __u8 int_in_int;               	/* polling interval */
    struct usbdpfp_device_event* event_data; /* data buffer and more */
    struct semaphore event_sem;	  	/* thread-safe access */
    struct semaphore event_compl_sem;
    atomic_t cancelable_int_urb;    /* flag indicating if the int urb cancelable */
	 int abort_state;

    /* control pipe */
    struct urb *ctrl_out_urb;
    struct usb_ctrlrequest *ctrl_setup;
    void* ctldata;						/* preallocate a fixed size memory for device I/O */
    int   size_ctldata;					/* size of ctldata memory buffer for ctldata */
    
    void *private_data;	/* device type specific stuff can go here */

    /* Device Name and Serial number info stored at the time of creation 
        and can be used in disconnect callback
     */
    char dev_name[80];
    char serial_no[80];
};

#define PNP_NR_DEVS			1
#define PNP_MINOR_START		0
#define PNP_MAJOR_NUM      0

#define PNP_ACTIVE_BIT		0

#define PNP_MAX_NODES		32 
//2 nodes for each device.

#define PNP_STATE_TERMINATED	1
#define PNP_STATE_NOT_TERMINATED	0


/*!
@struct usbdpfp_pnp_node
@event		event on which the software device ioctl waits.
@minor		spinlock to serialize the access to event and event_data.
event_list	used for linked list management 
*/

struct usbdpfp_pnp_node {
    struct usbdpfp_device_pnp_event    event;
    int                                minor;
    struct list_head                   event_list;
};

/*!
@struct usbdpfp_pnp_device
@is_open	used to limit the no. of times this device is opened.
@new_events	list of event nodes that are not yet reported to the userspace.
@repored_events	list of connect events that are reported to the userspace driver.
@lock		spinlock to serialize the access to the event lists.
*/

struct usbdpfp_pnp_device {
    dev_t                           devno;
    unsigned long                   is_active;
    int                             terminated;
    struct list_head                new_events;
    struct list_head                reported_events;
    struct semaphore                event_lock;
    wait_queue_head_t               wait_queue;
    wait_queue_t                    wait;
    struct cdev                     cdev;	
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
    kmem_cache_t                    *cache;
#else
    struct kmem_cache               *cache;
#endif
    mempool_t                       *pool;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
    struct class_simple             *class;
#else
    struct class                    *class;
#endif
};

/* Configurable variables */

/* pnp char device major/minor */
extern int usbdpfp_pnp_major;
extern int usbdpfp_pnp_minor;

/* our own private debug macros */
extern int debug;
extern int mdebug;

#undef err
//#define err(format, arg...) 
#define err(format, arg...) printk(KERN_ERR __FILE__ ": **ERROR** %s: %d: " format "\n" , __func__, __LINE__, ## arg)

#undef info
//#define info(format, arg...) 
//#define info(format, arg...) printk(KERN_INFO __FILE__ ": %s: %d: " format "\n" , __func__ , __LINE__, ## arg)
#define info(format, arg...) printk(KERN_INFO": %s: %d: " format "\n" , __func__ , __LINE__, ## arg)

#undef dbg
//#define dbg(format, arg...) 
#define dbg(format, arg...) do { if (debug) info(format, ## arg); } while (0)

#undef mdbg
#define mdbg(format, arg...) do { if (mdebug) info(format, ## arg); } while (0)

// these macros are different from 2.4 in that ep is a pointer to the struct
// and not the struct itself
#define IS_EP_BULK(ep)  ((ep)->bmAttributes == USB_ENDPOINT_XFER_BULK ? 1 : 0)
#define IS_EP_BULK_IN(ep) (IS_EP_BULK(ep) && ((ep)->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
#define IS_EP_INTR(ep) ((ep)->bmAttributes == USB_ENDPOINT_XFER_INT ? 1 : 0)

extern void *usbdpfp_kmalloc(size_t, int);
extern void usbdpfp_kfree(const void *);


#endif // __USBDPFP_H
