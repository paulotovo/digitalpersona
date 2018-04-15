/* usbdpfp.c - DigitalPersona Fingerprint Sensors Device Driver
*
* Copyright 1996-2011 DigitalPersona, Inc.  All rights reserved.
* 
*
************
* Changelog:
************
* (March/2011)
* - Add 2.6.36 kernel support
*
* (July/2010)
* - Add 2.6.32 kernel support
*
* (March/2010)
* - Release image buffer on device file close.
* - Add mdbg macro and mdebug variable for memory tracing.
*
* (Jan/2010) 
* - Port to kernel 2.6.28 (for Ubuntu distribution)
* - add abort state to device.
*
* (03/08)
* - SlackWare supprot is added.
* - Replace class_dev with usb_dev and class_id with bus_id
*
* (07/07) 
* - Upgrade to kernel 2.6.18 (SUSE 10.2 and Slackware 11.0)
* - Replace class_simple_device with class_device.
*
* (06/07) PowerManagement:
* - Suspend handler implemented to support Standby and hibernate system states.
* - Changes to event IOCTL and Channel mechanism to support Power management.
*
* (05/07) Modified:
* - USBDPFP_IOCTL_ABORT_BULK_READ.
* - Aborts pending bulk URB and prevents further streaming with 
*   respect to channel mechanism.
* - Channel mechanism to support wide range of devices tranparently.
*   Basically, the channel specifies the size of the frame to expect
*   and the number of frames to be streamed while the channel is activated
* - USBDPFP_IOCTL_CONFIG_CHANNEL:
*   Configures a channel with No. of frames to capture and 
*   the size of the frames.
* - USBDPFP_IOCTL_SET_ACTIVE_CHANNEL:
*   Activates the specified channel to capture the image.
*   By default First channel with a single frame is activated.
* - Read entry point
*   First read operation allocates the kernel buffer for the images. Starts 
*   streaming if active channel is configured with frames > 1.  
* - Maximum of USBDPFP_MAX_CHANNELS is supported for each device.
* - Maximum No. of devices limited by Kernel's support for USB devices.
* - USBDPFP_IOCTL_SET_STREAMED_READ replaced with channel mechanism.
* 
* (04/07) Aborting of bulk pipe transfer from user mode:
* - USBDPFP_IOCTL_ABORT_BULK_READ:
*   Abort the submitted bulk URB from usermode driver supported.
* - Streaming fingerprint images:
*   Implemented Streaming image read and Patchprint image read.
*   A device independent implementation. 
*
* (03/07) Enhanced device information to user mode driver:
* USBDPFP_IOCTL_GET_INFO
*  Device name and Serial No. fields added into usbdpfp_
*  device_info struct.  
*
* (03/07) PnP event hook to user mode driver:
* - USBDPFP_IOCTL_WAIT_PNP_EVENT
*   Provides mechanism to notify usermode driver thread that device
*   Plug-in and plug-out has occured.
* - USBDPFP_IOCTL_CANCEL_WAIT_PNP_EVENT 
*   Provides mechanism to abort usermode driver's wait for the event.
*
*****************
* Kernel support:
***************** 
* The driver Supports Linux Kernel 2.6.5-2.6.18, 2.6.28-2.6.37.
*
**********************
* Future enhancements:
**********************
*
* Channel implementation could be enhanced to support 
* any no. of channels dynamically. 
*
***********************
* Areas of Improvement:
***********************
*
*
*************
* Known Bugs:
*************
* None
*
*/


#ifndef __KERNEL__
# error "The __KERNEL__ preprocessor symbol must be defined"
#endif
#ifndef MODULE
# error "This driver must be built as a module due to licensing issues"
#endif

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#include <linux/config.h>
#endif
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/completion.h>
#include <linux/delay.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
#include <linux/semaphore.h>
#endif
#include <asm/uaccess.h>        

#include "usbdpfp.h"


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
# error "This module needs kernel version 2.6.5 or greater"
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)
# ifndef init_MUTEX
#  define init_MUTEX(x) sema_init(x,1)
# endif
#endif

#define MODULE_NAME    "mod_usbdpfp"
#define DRIVER_AUTHOR  "DigitalPersona, Inc. <www.digitalpersona.com>"
#define DRIVER_DESC    "DigitalPersona Fingerprint Reader USB Driver"

// We get the DRIVER_VERSION macro from the main Makefile
#ifndef DRIVER_VERSION
#define DRIVER_VERSION "no version"
#endif

// USB char driver device minor for DigitalPersona fingerprint scanner: 176-191
// see www.LANANA.ORG
// The USB char driver major number is: 180
// For example
// To create a node for DP fingerprint char driver manually:
//    mknod -m 666 /dev/usbdpfp0 c 180 176
// To create a node for the PnP driver:
//    mknod -m 666 /dev/usbdpfpPnp c 254 0
#define USB_USBDPFP_MINOR_BASE	176

// USB driver entry points.
static int   usbdpfp_probe(struct usb_interface *interface, const struct usb_device_id *id);
static void  usbdpfp_disconnect(struct usb_interface *interface);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
static int   usbdpfp_suspend(struct usb_interface *interface, u32 state);
#else
static int   usbdpfp_suspend(struct usb_interface *interface, pm_message_t message);
#endif
static int   usbdpfp_resume(struct usb_interface *interface);

// Char driver file operations routines for the USB device.
static int     usbdpfp_open(struct inode *inode, struct file *file);
static int     usbdpfp_close(struct inode *inode, struct file *file);
static ssize_t usbdpfp_read(struct file *file, char *buffer, size_t count, loff_t * ppos);
static ssize_t usbdpfp_write(struct file *file, const char *buffer, size_t count, loff_t * ppos);
static loff_t  usbdpfp_llseek(struct file *filp, loff_t offset, int whence);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int     usbdpfp_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
#else
static long    usbdpfp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif


// Helper routines
static __u16 get_product_id(struct usbdpfp_device *dev);
static __u16 get_vendor_id(struct usbdpfp_device *dev);
static __u16 get_revision(struct usbdpfp_device *dev);
static int   get_device_info(struct usbdpfp_device *dev, struct usbdpfp_device_info *dev_info);


// USB pipe operations
static int  usbdpfp_control_pipe_write(struct usbdpfp_device *dev, unsigned long arg);
static int  usbdpfp_control_pipe_read(struct usbdpfp_device *dev, unsigned long arg);


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
static void usbdpfp_interrupt_callback(struct urb *urb);
#else
static void usbdpfp_interrupt_callback(struct urb *urb, struct pt_regs*);
#endif

static int  usbdpfp_interrupt_pipe_read(struct usbdpfp_device *dev, unsigned long arg);

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
static void usbdpfp_bulk_callback(struct urb *urb);
#else
static void usbdpfp_bulk_callback(struct urb *urb, struct pt_regs *dummy);
#endif
static int  usbdpfp_bulk_pipe_read(struct usbdpfp_device *dev, unsigned char *buf, size_t count, int nowait, 
                           int mem_flags);
// Forward reference for kref_init in usbdpfp_new.
// (kref require kernel 2.6.5-rc1 or later).
static inline  void  usbdpfp_delete(struct kref *kref);
static inline struct usbdpfp_device *usbdpfp_new(void);


// PNP event notification functions.
static int  usbdpfp_add_attach_event( struct usb_interface *interface );
static int  usbdpfp_handle_detach_event( struct usb_interface *interface );
static int  usbdpfp_add_resume_event( struct usb_interface *interface );


// USB device file operation structure.
static struct file_operations usbdpfp_fops = 
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16)
    .owner   = THIS_MODULE,
#endif
    .open    = usbdpfp_open,
    .release = usbdpfp_close,
    .read    = usbdpfp_read,
    .write   = usbdpfp_write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    .ioctl   = usbdpfp_ioctl,
#else
    .unlocked_ioctl = usbdpfp_ioctl,
#endif
    .llseek  = usbdpfp_llseek,
};


// Our device Id table
static struct usb_device_id usbdpfp_id_table[] = 
{
    {USB_DEVICE(DP_VID, URU4000B)},
    {USB_DEVICE(DP_VID, URU4000)},	  
    {.match_flags=USB_DEVICE_ID_MATCH_DEVICE|USB_DEVICE_ID_MATCH_INT_INFO,
    .idVendor = DP_VID, 
    .idProduct = URU4000KB,
    .bInterfaceClass = 0xff, 
    .bInterfaceSubClass = 0xff, 
    .bInterfaceProtocol = 0xff
    }, 
    {0}
};
MODULE_DEVICE_TABLE(usb, usbdpfp_id_table);


// USB driver entry points.
static struct usb_driver usbdpfp_usb_driver = 
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16)
    .owner      = THIS_MODULE,
#endif
    .name       = MODULE_NAME,
    .probe      = usbdpfp_probe,
    .disconnect = usbdpfp_disconnect,
    .id_table   = usbdpfp_id_table,
    .suspend    = usbdpfp_suspend,
    .resume     = usbdpfp_resume,
};


// USB class driver structure.
static struct usb_class_driver usbdpfp_class = {
    .name       = "usb/usbdpfp%d",
    .fops       = &usbdpfp_fops,
    //   .mode       = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH,
    .minor_base = USB_USBDPFP_MINOR_BASE,
};


// Some helper functions.

#define usbdpfp_kmalloc(size, flags) \
    ({ \
    void* p = kmalloc((size), (flags));  \
    mdbg("usbdpfp_kmalloc(%d, %d)=%p", (int)(size), (flags), p); \
    (void*)p; \
    })

#define usbdpfp_kfree(p) \
{ \
    mdbg("usbdpfp_kfree(%p)", (p)); \
    kfree(p); \
    }

inline void usbdpfp_kref_init(struct kref *kref, void (*release)(struct kref *kref))
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
    kref_init(kref, release);
#else
    kref_init(kref);
#endif
}

inline void usbdpfp_kref_put(struct kref *kref, void (*release)(struct kref *kref))
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
    kref_put(kref);
#else
    kref_put(kref, release);
#endif
}

static __u16 get_product_id(struct usbdpfp_device *dev)
{
    return dev->udev->descriptor.idProduct;
}

static __u16 get_vendor_id(struct usbdpfp_device * dev)
{
    return dev->udev->descriptor.idVendor;
}

static __u16 get_revision(struct usbdpfp_device * dev)
{
    return dev->udev->descriptor.bcdDevice;
}

// Copies the device name into the buffer from the dev struct, stored during probe 
static void get_device_name(struct usbdpfp_device *dev, char *buffer)
{
    if(!buffer)
        return;
    strncpy(buffer, dev->dev_name, 80);
    dev->dev_name[79]='\0';
}

// Copies the device serial number into the buffer from the dev struct, stored during probe 
static void get_device_serial_no(struct usbdpfp_device *dev, char *buffer)
{
    if(!buffer)
        return;
    strncpy(buffer, dev->serial_no, 80);
    dev->dev_name[79]='\0';
}

// Get all the device info and fill up the dev_info structure
static int get_device_info(struct usbdpfp_device *dev, struct usbdpfp_device_info *dev_info)
{
    if(!dev || !dev_info)
        return -EINVAL;
    dev_info->idProduct = (unsigned int) get_product_id(dev);
    dev_info->idVendor  = (unsigned int) get_vendor_id(dev);
    dev_info->bcdDevice = (unsigned int) get_revision(dev);
    get_device_name(dev, dev_info->nameDevice);
    get_device_serial_no(dev, dev_info->snDevice);
    return 0;
}

/*!
frame utilities:- 
These functions should be called with the bulk_sem held.
*/

/*!
Initialize a frame. Does not allocate the data buffer.
*/
static void inline init_frame(struct usbdpfp_frame *frame)
{
    frame->buffer = NULL;
    frame->bulk_read_status = 0;
    frame->bulk_read_count = 0;
    frame->short_packet_detected = 0;
    frame->valid = USBDPFP_FRAME_INVALID;     
}

/*!
Frees the previously allocated data buffers and initializes the frame
*/
static inline void cleanup_and_init_frame(struct usbdpfp_frame *frame)
{
    if(frame->buffer){
        usbdpfp_kfree(frame->buffer); //Free any allocated buffer 
    }
    frame->buffer = NULL;
    frame->bulk_read_status = 0;
    frame->bulk_read_count = 0;
    frame->short_packet_detected = 0;
    frame->valid = USBDPFP_FRAME_INVALID;     
}

/*!
Initializes all the frames of a channel.
*/
static inline void init_all_frames(struct usbdpfp_channel_config *channel)
{
    int index;
    for(index=0; index<USBDPFP_MAX_FRAMES; index++){
        init_frame(&(channel->frame[index]));
    }
}

/*!
Cleans-up an initializes the frames of a channel.
*/
static inline void cleanup_and_init_all_frames(struct usbdpfp_channel_config *channel)
{
    int index;
    for(index=0; index<USBDPFP_MAX_FRAMES; index++){
        cleanup_and_init_frame(&(channel->frame[index]));
    }
}

/*!
Invalidates all the frames of the channel. 
Does not allocate or deallocate the data buffer.
*/
static inline void invalidate_frames(struct usbdpfp_channel_config *channel)
{
    int index;
    for (index=0; index<channel->max_frames; index++) {
        channel->frame[index].valid=USBDPFP_FRAME_INVALID;
        channel->frame[index].bulk_read_status=0;
        channel->frame[index].bulk_read_count=0;
        channel->frame[index].short_packet_detected=0;          
    }
}

/*!
Channel Utilities.
These function should be called with 
the bulk_sem held;
*/

/*!
Initializes the channel for fresh use.
*/
static void init_channel(struct usbdpfp_channel_config *channel, unsigned int max_frames, 
                         unsigned max_bytes_per_frame)
{
    channel->valid = USBDPFP_CHANNEL_NOT_CONFIGURED;
    channel->max_frames = max_frames;         
    channel->max_bytes_per_frame = max_bytes_per_frame;
    channel->frame_write_index = 0;
    channel->frame_read_index = 0;     
    init_all_frames(channel);
}

/*!
Cleanup the channel.
If frame data buffers are allocated for the channel releases them.
*/
static inline void cleanup_channel(struct usbdpfp_channel_config *channel)
{
    channel->valid = USBDPFP_CHANNEL_NOT_CONFIGURED;
    channel->max_frames = 0;         //0 is an invalid value
    channel->max_bytes_per_frame = 0;//0 is an invalid value
    channel->frame_write_index=0;
    channel->frame_read_index = 0;          
    cleanup_and_init_all_frames(channel);
}

/*!
Cleanup all the channels for the device.
*/
static inline void init_all_channels(struct usbdpfp_device *dev)
{
    int index;
    for(index=0; index < USBDPFP_MAX_CHANNELS; index++) {
        init_channel(&dev->channel[index], 0, 0);
    }
}

/*!
allocate the frame buffers for the channel
succeeds in allocating all the frame buffers
or fails and allocates none.
*/
static int allocate_frame_buffers(struct usbdpfp_channel_config *active_channel)
{
    int alloc_index;
    unsigned long alloc_count;

    alloc_count=active_channel->max_bytes_per_frame;

    for(alloc_index=0; alloc_index < active_channel->max_frames; alloc_index++) 
    {
        //if there was a frame left out, free it
        if(active_channel->frame[alloc_index].buffer) {
            usbdpfp_kfree(active_channel->frame[alloc_index].buffer);
            active_channel->frame[alloc_index].buffer = NULL;
        }
        active_channel->frame[alloc_index].buffer=usbdpfp_kmalloc(alloc_count, GFP_KERNEL);

        dbg("frame # %d: address=%p", alloc_index, active_channel->frame[alloc_index].buffer);     

        if(!active_channel->frame[alloc_index].buffer)
            break;
    }
    if(alloc_index!=active_channel->max_frames) {//all frames not be allocated
        //free the allocated frame buffers here
        for(--alloc_index; alloc_index>=0; alloc_index--) {
            usbdpfp_kfree(active_channel->frame[alloc_index].buffer);
            active_channel->frame[alloc_index].buffer = NULL;
        }
    }
    return alloc_index;
}


/*!
Deallocate all the buffers of the channel.
Checks all the frames independent of how channel is configured.
May be redundant after 'max frames' but helps keep cleanups simple.
*/
static inline void deallocate_frame_buffers(struct usbdpfp_channel_config *
                                            active_channel)
{
    int index;
    for(index=0; index<USBDPFP_MAX_FRAMES; index++) 
        if(active_channel->frame[index].buffer) {
            usbdpfp_kfree(active_channel->frame[index].buffer);
            active_channel->frame[index].buffer=NULL;
        }
}

static inline int is_empty_frames(struct usbdpfp_channel_config *channel)
{
    int is_empty = 1;

    if (channel && (channel->frame_write_index != channel->frame_read_index)) {
        is_empty = 0;
    }

    return is_empty;		
}

static inline int is_full_frames(struct usbdpfp_channel_config *channel)
{
    int full_empty = 0;

    if (channel && 
        (channel->frame_write_index - channel->frame_read_index) == USBDPFP_MAX_FRAMES -1) 
    {
        full_empty = 1;
    }

    return full_empty;		
}

static inline void adv_read_frame(struct usbdpfp_channel_config *channel)
{
    if (channel) {
        channel->frame_read_index += 1;
        if (channel->frame_read_index >= channel->max_frames)
            channel->frame_read_index = 0;
    }
}

static inline void adv_write_frame(struct usbdpfp_channel_config *channel)
{
    if (channel) {
        channel->frame_write_index += 1;
        if (channel->frame_write_index >= channel->max_frames)
            channel->frame_write_index = 0;
    }
}

static inline void reset_frames(struct usbdpfp_channel_config *channel)
{
    if (channel) {
        //int i;
        /* These will cause the queue to be empty */
        channel->frame_write_index = 0;
        channel->frame_read_index = 0;

        /* These code will be done by invalidate_frames()
        for (i = 0; i < channel->max_frames; i++) {
        channel->frame[i].bulk_read_count = 0;		
        channel->frame[i].bulk_read_status = 0;
        channel->frame[i].short_packet_detected = 0;
        channel->frame[i].valid = USBDPFP_FRAME_INVALID;
        }	
        */
    }
}


/* USB pipes code */

// Synchronous io write to control pipe 
static int usbdpfp_control_pipe_write(struct usbdpfp_device *dev, unsigned long arg)
{
    int    result = 0;
    struct usbdpfp_device_data dev_data;
    struct usbdpfp_device_data* p = (struct usbdpfp_device_data*) arg;

    if (!dev || !dev->udev || !p) {
        err("bad parameter (dev=%p, arg=%p)", dev, p);
        return -EFAULT;
    }	

    if (!p->data || p->length <= 0 || 
        !access_ok(VERIFY_READ, (void __user*) p->data, p->length)) {
            err("device minor %d: bad parameter (.length=%d, .data=%p)", 
                dev->minor, p->length, p->data);
            return -EFAULT;
    }

    // all the user data seems to be ok, copy them to kernel space
    if (copy_from_user(&dev_data, (const void __user*)arg, 
        sizeof(struct usbdpfp_device_data))) {
            err("device minor %d: copy_from_user() 1 failed", dev->minor);
            return -EFAULT;
    }

    // Optimize frequent allocation for small memory 
    if (dev_data.length > dev->size_ctldata) {
        dev_data.data = usbdpfp_kmalloc(dev_data.length * sizeof(unsigned char), GFP_KERNEL);
    } else {
        // use the preallocated the buffer (must be used one thread at the time)
        dev_data.data = dev->ctldata;
    }

    if(dev_data.data == NULL) {
        err("device minor %d: Out of memory", dev->minor);
        return -EFAULT;		
    }
    if (copy_from_user(dev_data.data, (const void __user*)p->data, dev_data.length)) {	
        err("device minor %d: copy_from_user() 2 failed", dev->minor);
        result = -EFAULT;
        goto io_write_exit;	// free memory and exit
    }

    dbg("device minor %d: .type=%d, .address=0x%x, .length=%d, .data[0]=0x%02x",
        dev->minor, dev_data.type, dev_data.address, dev_data.length, 
        ((unsigned char*)dev_data.data)[0]);

    // issue urb to device - synchronous operation!!
    result = usb_control_msg(
        dev->udev,                    /* USB device to send  */
        usb_sndctrlpipe(dev->udev, 0),/* pipe, end point of USB device */
        0x04,                         /* USB request */
        USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_DIR_OUT, /* USB request type 0x40 */
        dev_data.address,             /* USB message value   */
        0,                            /* USB message index value */
        dev_data.data,                /* data buffer */
        dev_data.length,              /* size of buffer */
        HZ * 5                        /* amount of timeout */
        );

    if (result < dev_data.length) {
        err("device minor %d: %d out of %d bytes was written", 
            dev->minor, result, dev_data.length);
    } 
io_write_exit:
    if(dev_data.data && (dev_data.data != dev->ctldata)) {
        usbdpfp_kfree(dev_data.data);
        dev_data.data = NULL;
    }

    return result;
}


// Synchronous io read from control pipe 
static int usbdpfp_control_pipe_read(struct usbdpfp_device *dev, unsigned long arg)
{
    int    result = 0;
    struct usbdpfp_device_data dev_data;
    struct usbdpfp_device_data* p = (struct usbdpfp_device_data*) arg;
    if (!dev || !dev->udev || !p) {
        err("bad parameter (dev=%p, arg=%p)", dev, p);
        return -EFAULT;
    }	
    if (!p->data || p->length <= 0 || 
        !access_ok(VERIFY_WRITE, (void __user*) p->data, p->length)) {
            err("device minor %d: bad parameter (.length=%d, .data=%p)", 
                dev->minor, p->length, p->data);
            return -EFAULT;
    }

    // the user data seems to be ok, copy them to kernel space
    if (copy_from_user(&dev_data, (const void __user*)arg, 
        sizeof(struct usbdpfp_device_data))) {
            err("device minor %d: copy_from_user() failed", dev->minor);
            return -EFAULT;
    }

    // Optimize frequent allocation for small memory 
    if (dev_data.length > dev->size_ctldata) {
        dev_data.data = usbdpfp_kmalloc(dev_data.length * sizeof(unsigned char), GFP_KERNEL);
    } else {
        // use the preallocated the buffer (must be used one thread at the time)
        dev_data.data = dev->ctldata;
    }

    if(NULL == dev_data.data) {
        err("device minor %d: Out of memory", dev->minor);
        return -EFAULT;		
    }


    // issue urb to device - synchronous operation!!
    result = usb_control_msg(
        dev->udev,                    /* USB device to send     */
        usb_rcvctrlpipe(dev->udev, 0),/* end point of USB device*/
        0x04,                         /* USB request value      */
        USB_TYPE_VENDOR | USB_RECIP_DEVICE 
        | USB_DIR_IN,       /* USB request type 0xc0  */
        dev_data.address,             /* USB message value      */
        0,                            /* USB message index value*/
        dev_data.data,                /* data buffer            */
        dev_data.length,              /* size of buffer         */
        HZ * 5                        /* amount of timeout      */
        );

    if (result < dev_data.length) {
        err("device minor %d: only %d was read, but %d bytes was expected.", 
            dev->minor, result, dev_data.length);
        goto io_read_exit;
    } 
    else { // copy the kernel data to user space
        dbg("device minor %d: copy data to user space, .address=%x, .length=%d, .data[0]=0x%02x", 
            dev->minor, dev_data.address, result, ((unsigned char*)dev_data.data)[0]);
        if (copy_to_user((void __user*)p->data, dev_data.data, dev_data.length*sizeof(unsigned char))) {	
            err("device minor %d: copy_to_user() failed", dev->minor);
            result = -EFAULT;
            goto io_read_exit;	// free memory and exit on error
        }
    }
io_read_exit:
    if(dev_data.data && (dev_data.data != dev->ctldata)) {
        usbdpfp_kfree(dev_data.data);
        dev_data.data = NULL;
    }
    return result;
}

/*!
Callback routine for interrupt pipe urb.
May be called after the following:
When a URB is unlinked by the Suspend ]----> No process is waiting at this point.
When a URB is successful.  -------------| 
When a URB fails due to other reasons   |--->wake up the Sleeping process.
When a URB is unlinked by abort IOCTL---|
*/
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
static void usbdpfp_interrupt_callback(struct urb *urb)
#else
static void usbdpfp_interrupt_callback(struct urb *urb, struct pt_regs*dummy)
#endif
{		
    if (urb) {
        struct usbdpfp_device* dev = urb->context;		
        if (dev) {
            atomic_set(&dev->cancelable_int_urb, 0); // No urbs to cancel
            // The dev->event_data->data now carries the raw data from device
            // but we need to update other fields to reflect in the caller. 
            dev->event_data->status = urb->status;
            dev->event_data->size_returned = urb->actual_length;

            //URB was unlinked through suspend.
            //Already come out of waiting and restarted the IOCTL system cal.
            //No need to Up the cpmpletion sem.
            if(atomic_read(&dev->suspended)) {   
                atomic_set(&dev->suspended, 0);
                dbg("driver was suspended");
            }
            //End waiting for IOCTL completion for any thing other than suspend.
            else {               
                dbg("up completion sem in int callback");
                up(&dev->event_compl_sem);
            }
        }
        else err("device %d doesn't exist", dev->minor);
    }
}

static int usbdpfp_interrupt_pipe_read(struct usbdpfp_device *dev, unsigned long arg)
{
    int result = 0, i;
    struct usbdpfp_device_event* p = NULL;
    p = (struct usbdpfp_device_event*) arg;

    if (!dev || !dev->udev || !p) {
        err("bad parameter (dev=%p, arg=%p)", dev, p);
        result = -EFAULT;
        goto event_read_error;
    }

    /* allow one thread at a time */
    if (down_interruptible(&dev->event_sem)) {
        dbg("device minor %d: acquiring event->sem failed", dev->minor);
        result =  -ERESTARTSYS;
        goto event_read_error;
    }

    if (p->size_requested <= 0 || p->size_requested > USBDPFP_MAX_EVENT_SIZE || 
        !access_ok(VERIFY_WRITE, (void __user*) p, 
        sizeof(struct usbdpfp_device_event))) {
            err("device minor %d: bad parameter or inaccessible memory", dev->minor);
            result = -EFAULT;
            goto event_read_exit;
    }
    if(0==atomic_read(&dev->cancelable_int_urb)) { //interrupt URB is not pending  
        dev->event_data->size_requested = p->size_requested = USBDPFP_MAX_EVENT_SIZE;
        dev->event_data->size_returned = p->size_returned = 0;
        dev->event_data->status = -EINVAL;  /* set to non-zero please*/

        usb_fill_int_urb(
            dev->int_in_urb,                /* urb for this USB transaction*/
            dev->udev,                      /* USB device object           */
            usb_rcvintpipe(dev->udev, dev->int_in_ep),/* endpoint IN       */
            dev->event_data->data,          /* transfer buffer             */ 
            dev->event_data->size_requested,/* transfer buffer size        */
            (usb_complete_t)usbdpfp_interrupt_callback, /*completion routine       */
            dev,                            /* context                     */
            dev->int_in_int                 /* polling interval            */
            );
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)          
        dev->int_in_urb->transfer_flags |= URB_ASYNC_UNLINK;/*allow immediate abort*/
#endif
        result = usb_submit_urb(dev->int_in_urb, GFP_KERNEL);
        if (result) {
            err("device minor %d: usb_submit_urb failed (%d)", dev->minor, result);
            goto event_read_exit;
        }

        /* set the cancelable flag to true to signal a pending urb. 
        this flag is cleared in the urb's completion routine. */
        atomic_set(&dev->cancelable_int_urb, 1);			
    }
    /* Wait for event data to arrive in an interruptible manner.
    The dev->event_compl_sem should be, ideally, signaled by the completion 
    routine. If a signal has interrupted the wait, cleanup whatever is done
    and try to restart the system call.
    */  		
    if ((i=down_interruptible(&dev->event_compl_sem))) {
        dbg("wait event_compl_sem interrupted by %x", i);
        result =  -ERESTARTSYS;
        goto event_read_exit;
    }

    /* copy the data back to user regardless status condition, 
    but check for user space again before copying data to it 
    */
    if (!access_ok(VERIFY_WRITE, (void __user*) p, sizeof(struct usbdpfp_device_event))) {
        err("device minor %d: inaccessible memory, p=%p)", dev->minor, p);
        result = -EFAULT;
        goto event_read_exit;
    }
    if (copy_to_user((void __user*)arg, dev->event_data, sizeof(struct usbdpfp_device_event))) {
        err("device minor %d: copy_to_user failed", dev->minor);
        result = -EFAULT;
        goto event_read_exit;		
    }

    /* The dev->event_data->status should be updated by the completion routine.
    If not,something has gone wrong and it should contain the initial value, 
    which should be non-zero.
    */
    if (dev->event_data->status != 0) { // something went wrong 
        if (dev->event_data->status == -ENOENT) {  // was killed
            dbg("device minor %d: urb status error code -ENOENT)", 
                dev->minor); 
        } else if (dev->event_data->status == -ECONNRESET) { // was unlinked
            dbg("device minor %d: urb status error code -ECONNRESET)", 
                dev->minor); 
        }
        else {
            dbg("device minor %d: urb status error 0x%x or 0x%x)", dev->minor, 
                dev->event_data->status, -dev->event_data->status);
        }
        result = dev->event_data->status; // send the status back to IOCTL.
    }
event_read_exit:
    up(&dev->event_sem);
event_read_error:	
    dbg("device minor %d: result=0x%x", dev->minor, result);	
    return result;
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
static void usbdpfp_bulk_callback(struct urb *urb)
#else
static void usbdpfp_bulk_callback(struct urb *urb, struct pt_regs *dummy)
#endif
{
    // Data arrives or error occurs from bulk pipe in. Store the data into the 
    // active channel's current frame buffer, continue stream then copy data
    // to user buffer and wake up the caller of usbdpfp_read.

    struct usbdpfp_device *dev = NULL;
    struct usbdpfp_channel_config *active_channel=NULL;
    struct usbdpfp_frame *cur_frame=NULL;

    if (!urb) {
        err("invalid URB");
        goto callback_exit;
    }
    dev = urb->context;		
    if (!dev) {
        err("invalid Device");
        goto callback_exit;
    }
    atomic_set(&dev->cancelable_bulk_urb, 0); // URB is done, can NOT be cancelled
    active_channel=dev->active_channel;      

    dbg("device minor %d: callback status=0x%0x, actual_length=%d", 
        dev->minor, urb->status, urb->actual_length);

    // Locate the current frame
    cur_frame = &active_channel->frame[active_channel->frame_write_index];      
    cur_frame->bulk_read_status = urb->status;
    cur_frame->bulk_read_count = urb->actual_length;

    if (cur_frame->bulk_read_status == 0) { // Successful
        cur_frame->valid = USBDPFP_FRAME_VALID;	//mark valid data 
        if (urb->actual_length == 0) { 
            //we treat short packet or end of packet as non-error and valid frame
            cur_frame->short_packet_detected = 1;  
        }

        adv_write_frame(active_channel);
    }
    else { //error occured or urb aborted.
        cur_frame->valid = USBDPFP_FRAME_ERROR;      
    }	                                

    //	wake_up_interruptible(&dev->inq); // Trigger the waiting read to wakeup. 

    // Continue streaming if ...	
    if (cur_frame->bulk_read_status == 0 &&   // no urb error
        dev->do_streaming_read && 				// streaming flag						
        !is_full_frames(active_channel))      // has free frame
    { 
        dbg("device minor %d: resubmit bulk read from callback", dev->minor);

        if (usbdpfp_bulk_pipe_read(dev, 
            active_channel->frame[active_channel->frame_write_index].buffer, 
            active_channel->max_bytes_per_frame,  
            O_NONBLOCK, GFP_ATOMIC)) 
        {
            // Failed to stream, report this through frame's read status.
            // we cannot do much here, especially not to touch the unread data.
            err("device minor %d: failed to stream at frame # %d",
                dev->minor, active_channel->frame_write_index);               
        }
    }

    wake_up_interruptible(&dev->inq); // Trigger the waiting read to wakeup. 
callback_exit:
    return;
}

static int usbdpfp_bulk_pipe_read(struct usbdpfp_device *dev, unsigned char *kbuf, size_t count, 
                          int nowait, int mem_flags)
{
    int result = 0;

    if (!dev || !dev->udev || !kbuf || count <= 0) {
        err("bad parameter (dev=%p, buf=%p, count=%d)", dev, kbuf, (int)count);
        result = -EFAULT;
        goto bulk_read_error;
    }	

    dbg("device minor %d: kbuf=%p, count=%d", dev->minor, kbuf, (int)count);

    usb_fill_bulk_urb(
        dev->bulk_in_urb,                    /* urb for bulk pipe read */
        dev->udev,                           /* USB device object      */
        usb_rcvbulkpipe(dev->udev, dev->bulk_in_ep),/* endpoint bulk in*/
        kbuf,                                /* xfer buffer            */
        count,                               /* xfer buffer size       */
        (usb_complete_t)usbdpfp_bulk_callback,       /* completion routine     */
        dev                                  /* context                */
        );

    result = usb_submit_urb(dev->bulk_in_urb, mem_flags);
    if (result) {
        err("device minor %d:usb_submit_urb failed (%d)", dev->minor, result);
    } else {
        atomic_set(&dev->cancelable_bulk_urb, 1);
    }
bulk_read_error:	
    dbg("device minor %d: result=0x%x or %d", dev->minor, result, result);		
    return result;
}

static void abort_bulk_read(struct usbdpfp_device *dev)
{
    if (dev) {
        if (atomic_read(&dev->cancelable_bulk_urb) == 1) {
            //usb_kill_urb(dev->bulk_in_urb);	
            usb_unlink_urb(dev->bulk_in_urb);	//TODO: old way
            while (atomic_read(&dev->cancelable_bulk_urb) == 1)
                ;
            dev->do_streaming_read = 0;
            //atomic_set(&dev->cancelable_bulk_urb, 0);
        }
        reset_frames(dev->active_channel);  
    }
}

/*
*	usbdpfp_new
*/
static inline struct usbdpfp_device *usbdpfp_new(void)
{
    struct usbdpfp_device *dev = NULL;

    /* allocate memory for our device state and intialize it */
    dev = usbdpfp_kmalloc(sizeof(struct usbdpfp_device), GFP_KERNEL);
    if (dev == NULL) {
        err("Out of memory");
        goto exit;
    }
    memset(dev, 0, sizeof(struct usbdpfp_device));		
    usbdpfp_kref_init(&dev->kref, usbdpfp_delete);
    init_MUTEX(&dev->sem);
    init_MUTEX(&dev->event_sem);
    init_MUTEX(&dev->bulk_sem);

	// dev->event_compl_sem will be initialized in usbdpfp_open

    init_waitqueue_head(&dev->inq);
    dev->disconnected = 0;

    dev->bulk_in_urb = usb_alloc_urb(0, GFP_KERNEL);
    dev->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);
    dev->ctrl_out_urb = usb_alloc_urb(0, GFP_KERNEL);

    atomic_set(&dev->cancelable_int_urb, 0);

    dev->event_data = usbdpfp_kmalloc(sizeof(struct usbdpfp_device_event), GFP_KERNEL);
    dev->ctrl_setup = usbdpfp_kmalloc(sizeof(struct usb_ctrlrequest), GFP_ATOMIC);

    /* preallocate a fixed size memory for device I/O */
    dev->size_ctldata = 64;	// preallocate 64 bytes
    dev->ctldata = (void*) usbdpfp_kmalloc(dev->size_ctldata, GFP_KERNEL);

    if(!(dev->bulk_in_urb && dev->int_in_urb && dev->ctrl_out_urb && dev->ctrl_setup &&
        dev->event_data)) {
            err("Out of memory");

            // we clean up here manually, rather than relying on kref.
            if (dev->ctrl_setup) 	 { usbdpfp_kfree(dev->ctrl_setup);  dev->ctrl_setup=NULL; }
            if (dev->event_data) 	 { usbdpfp_kfree(dev->event_data);  dev->event_data=NULL; }
            if (dev->ctrl_out_urb) { usb_free_urb(dev->ctrl_out_urb); dev->ctrl_out_urb=NULL;}
            if (dev->int_in_urb) 	 { usb_free_urb(dev->int_in_urb);   dev->int_in_urb=NULL; }
            if (dev->bulk_in_urb)  { usb_free_urb(dev->bulk_in_urb);  dev->bulk_in_urb=NULL; }
            usbdpfp_kfree(dev); 
            dev = NULL;
            goto exit;
    }

    dev->event_data->size_requested = USBDPFP_MAX_EVENT_SIZE;

    atomic_set(&dev->cancelable_bulk_urb, 0);
    atomic_set(&dev->suspended, 0);
    dev->do_streaming_read=0;
    init_all_channels(dev);
    dev->active_channel=NULL;
    dev->abort_state = 0;
exit:
    return dev;
}


/**
*	usbdpfp_delete
*/
static inline void usbdpfp_delete(struct kref *kref)
{
    int index;
    struct usbdpfp_device *dev = container_of(kref, struct usbdpfp_device, kref);
    if (down_interruptible(&dev->bulk_sem)) {
        dbg("device minor %d: acquiring bulk->sem failed", dev->minor);
    }

    dev->active_channel = NULL;

    /* free all the allocated frame buffers */
    for(index=0;index<USBDPFP_MAX_CHANNELS;index++)
        cleanup_channel(&dev->channel[index]);

    up(&dev->bulk_sem);

    if (dev->ctldata) {
        usbdpfp_kfree(dev->ctldata);     dev->ctldata=NULL;
    }

    if (dev->ctrl_setup) { 
        usbdpfp_kfree(dev->ctrl_setup);  dev->ctrl_setup=NULL;  
    }
    if (dev->event_data) { 
        usbdpfp_kfree(dev->event_data);  dev->event_data=NULL;  
    }
    if (dev->ctrl_out_urb) { 
        usb_free_urb(dev->ctrl_out_urb); dev->ctrl_out_urb=NULL;
    }
    if (dev->int_in_urb) { 
        usb_free_urb(dev->int_in_urb);   dev->int_in_urb=NULL;   
    }
    if (dev->bulk_in_urb) { 
        usb_free_urb(dev->bulk_in_urb);  dev->bulk_in_urb=NULL; 
    }

    usb_put_dev(dev->udev);
    usbdpfp_kfree(dev);
    dev = NULL;
}


/*!
config_channel
Configures the channel with the channel info.
Returns number of frames configured for the channel on success, 
negetive on error. Returns -EINVAL if argument passed is invalid.        
*/
static int config_channel(struct usbdpfp_device *dev, struct usbdpfp_channel_info *ch_info )
{
    int result = -EINVAL;
    struct usbdpfp_channel_config *cur_ch;
    if(ch_info && (ch_info->ch_id < USBDPFP_MAX_CHANNELS) && 
        (ch_info->max_frames <= USBDPFP_MAX_FRAMES) &&
        (ch_info->bytes_per_frame <= USBDPFP_MAX_FRAME_SIZE)) {
            // allow one thread at a time
            if (down_interruptible(&dev->bulk_sem)) {
                dbg("device minor %d: acquiring bulk->sem failed", dev->minor);
                result =  -ERESTARTSYS;
                goto exit;
            }

            dbg("device minor %d: ch_id=%d, max_frames=%d, bytes per=%u",
                dev->minor, ch_info->ch_id, ch_info->max_frames, ch_info->bytes_per_frame);

            cur_ch = &(dev->channel[ch_info->ch_id]);
            if(USBDPFP_CHANNEL_CONFIGURED == cur_ch->valid) {
                cleanup_channel(cur_ch); 
            }
            //configure the channel
            init_channel(cur_ch, ch_info->max_frames, ch_info->bytes_per_frame);
            cur_ch->valid=USBDPFP_CHANNEL_CONFIGURED;
            cur_ch->ch_id = ch_info->ch_id;
            //result=ch_info->max_frames;
            result = 0;	// return 0 on success

            up(&dev->bulk_sem);
    }
    else {
        err("invalid channel info");
        result = -EINVAL;
    }
exit:
    return result;      
}

/*!
Set the channel as active channel in the device.
The channel will be made active only if it is configured.
*/

static int set_active_channel(struct usbdpfp_device *dev, int ch_id, loff_t *f_pos)
{
    int result = 0;

    // allow one thread at a time
    if (down_interruptible(&dev->bulk_sem)) {
        dbg("device minor %d: acquiring bulk->sem failed", dev->minor);
        result =  -ERESTARTSYS;
        goto exit;
    }

    if(ch_id >= 0 && ch_id < USBDPFP_MAX_CHANNELS &&
        USBDPFP_CHANNEL_CONFIGURED == dev->channel[ch_id].valid) 
    {
        abort_bulk_read(dev);	//synchronous call (wait until abort complete)

        // switch channel
        dev->active_channel = &dev->channel[ch_id];
        dev->do_streaming_read = 0;
        reset_frames(dev->active_channel);	// reset new active channel
    }
    else
        result = -1;

    up(&dev->bulk_sem);

exit:
    return result;
}


/**
*	usbdpfp_open
*/
static int usbdpfp_open(struct inode *inode, struct file *filp)
{
    struct usbdpfp_device* dev = NULL;
    struct usb_interface* interface = NULL;
    int subminor;
    int retval = 0;

    subminor = iminor(inode);

    dbg("open device minor = %d", subminor);

    interface = usb_find_interface(&usbdpfp_usb_driver, subminor);
    if(!interface) {
        err("Cannot find interface for subminor %d", subminor);
        retval = -ENODEV;
        goto out_error;
    }

    dev = usb_get_intfdata(interface);
    if(!dev) {
        retval = -ENODEV;
        goto out_error;
    }
    if (dev->isopen) {
        dbg("device already opened");
        retval = -EBUSY;
        goto out_error;
    }
    dev->isopen = 1;

    init_MUTEX(&dev->event_compl_sem);
    if ((retval=down_interruptible(&dev->event_compl_sem))) {
        dbg("Couldn't hold the semaphore");
        goto out_error;
    }

    if (down_interruptible(&dev->bulk_sem)) {
        dbg("device minor %d: acquiring bulk->sem failed", dev->minor);
        retval =  -ERESTARTSYS;
        goto out_error;
    }

    atomic_set(&dev->cancelable_bulk_urb, 0); 
    init_all_channels(dev);
    // By default: activate the first channel with non-streaming mode 
    init_channel( &dev->channel[0], 1, USBDPFP_MAX_FRAME_SIZE);
    dev->channel[0].valid=USBDPFP_CHANNEL_CONFIGURED; //declare first channel 
    //        as configured
    dev->active_channel = &dev->channel[0];      //0th channel is made active.
    dev->do_streaming_read=0;
    dev->abort_state = 0;

    up(&dev->bulk_sem);

    kref_get(&dev->kref);
    filp->private_data = dev;  /* save our object in file struct */
out_error:
    return retval;
}


/**
*	usbdpfp_close
*/
static int usbdpfp_close(struct inode *inode, struct file *filp)
{
    struct usbdpfp_device *dev;
    int retval = 0;
    int index = 0;

    dbg("close device minor = %d", iminor(inode));

    dev = (struct usbdpfp_device *) filp->private_data;
    if (dev == NULL) {
        err("private data is NULL");
        return -ENODEV;
    }
    if (dev->isopen <= 0) {
        dbg("device not opened");
        retval = -ENODEV;
        goto exit_close;
    }
    dev->isopen = 0;

    if (down_interruptible(&dev->bulk_sem)) {
        dbg("device minor %d: acquiring bulk->sem failed", dev->minor);
        retval = -ERESTARTSYS;
        goto exit_close;
    }

    /* free all the allocated frame buffers */
    for(index=0; index<USBDPFP_MAX_CHANNELS; index++)
        cleanup_channel(&dev->channel[index]);

    up(&dev->bulk_sem);

exit_close:
    usbdpfp_kref_put(&dev->kref, usbdpfp_delete);
    return retval;
}


/*!
usbdpfp_read
Read the existing frame if present otherwise start the usb read operation.
The frames are stored in the active channel's frame buffer, which is a 
circular array.  
*/
static ssize_t usbdpfp_read(struct file *filp, char *buf, size_t count, loff_t * f_pos)
{
    int result = 0, ret=0, /*offset,*/ index;
    struct usbdpfp_frame *cur_frame=NULL;
    struct usbdpfp_channel_config *active_channel=NULL;
    struct usbdpfp_device *dev = (struct usbdpfp_device *)filp->private_data;	

    if (!dev || !dev->udev || dev->disconnected || !buf || count <= 0 ) {
        err("bad parameter (dev=0x%p)", dev); 
        result = -ENODEV;
        goto read_error;
    }

    // allow one thread at a time 
    if (down_interruptible(&dev->bulk_sem)) {
        dbg("device minor %d: acquiring bulk->sem failed", dev->minor);
        result =  -ERESTARTSYS;
        goto read_error;
    }

    if (!dev->isopen)
    {
        err("device minor %d: device is close", dev->minor);
        result =  -ENODEV;
        goto bulk_sem_exit;	
    }

    active_channel = dev->active_channel; 
    if (NULL == active_channel) {
        err("device minor %d: no active channel)", dev->minor); 
        result = -EINVAL;
        goto bulk_sem_exit;
    }

    kref_get(&dev->kref);

    cur_frame =  &active_channel->frame[active_channel->frame_read_index];

    // Check if count is a valid value.
    if(count > USBDPFP_MAX_FRAME_SIZE) 
        count = USBDPFP_MAX_FRAME_SIZE;  

    // If there is no data and we have not yet submit request, then kick off it.
    if (is_empty_frames(active_channel) &&              //no data in the array 
        dev->do_streaming_read == 0 &&
        atomic_read(&dev->cancelable_bulk_urb) == 0   //no pending urb request
        ) 
    {	
        dbg("device minor %d: empty frame, initiate bulk read", dev->minor);

        if(count > active_channel->max_bytes_per_frame) { 
            //need larger buffers, free the current frame buffers and 
            //specify the size needed for the new buffers
            deallocate_frame_buffers(active_channel);          
            active_channel->max_bytes_per_frame = count; 
        }

        if(NULL == cur_frame->buffer) { //frame buffer is not yet allocated
            if(active_channel->max_frames != allocate_frame_buffers(active_channel)) {
                err("could not allocate frame buffers");
                result = -ENOMEM;
                goto kref_exit;
            }
        }

        invalidate_frames(active_channel);//status changes once frame is read          

        if(active_channel->max_frames > 1)
            dev->do_streaming_read = 1;

        if(usbdpfp_bulk_pipe_read(dev, cur_frame->buffer, count, (filp->f_flags & O_NONBLOCK), GFP_KERNEL))
        {     //don't sleep if nobody would wake you up
            for(index=0; index<active_channel->max_frames; index++) {
                active_channel->frame[index].bulk_read_status = -1; 
            }
        }
    }

    do {
        if (wait_event_interruptible (dev->inq, 
            ((cur_frame->bulk_read_count != 0) || 	//data arrived
            (cur_frame->bulk_read_status != 0) || 	//error occurs
            (dev->disconnected)                ||	//device gone
            (cur_frame->short_packet_detected != 0)))) //end of packet
        {
            dbg("device minor %d:  wait_event_interruptible() failed", dev->minor);
            result = -ERESTARTSYS;
            goto kref_exit;
        }
    } while (USBDPFP_FRAME_INVALID == cur_frame->valid &&
        !cur_frame->bulk_read_status  && 
        !cur_frame->short_packet_detected && 
        !dev->disconnected);

    // Woken up by completion handler on successful read
    if (USBDPFP_FRAME_VALID == cur_frame->valid) { //got data or short packet
        // Copy data back to user space.
        if (cur_frame->bulk_read_count > count) {  // should not happen
            cur_frame->bulk_read_count = count;
        }
        if (cur_frame->bulk_read_count != 0) { //got some data
            if (!access_ok(VERIFY_WRITE, (void __user*) buf, count)) {
                err("device minor %d: inaccessible memory, ubuf=%p)", dev->minor, buf);
                result = -EFAULT;
                goto read_exit;
            }
            if ((ret = copy_to_user((void __user*)buf, cur_frame->buffer, cur_frame->bulk_read_count))) {
                err("device minor %d: copy_to_user failed with %d", dev->minor, ret);
                result = -EFAULT;
                goto read_exit;		
            }

            dbg ("device minor %d: bulk read returned %d bytes [frame %d], %d was requested", 
                dev->minor, cur_frame->bulk_read_count, 
                active_channel->frame_read_index, 
                (int)count);

            cur_frame->valid = USBDPFP_FRAME_INVALID;  // TODO: should clear the data for security
            adv_read_frame(active_channel);
        }

        result = cur_frame->bulk_read_count; 
    } 
    else if (USBDPFP_FRAME_ERROR == cur_frame->valid)  {  
        err("device minor %d: error in stream frame (%d), urb status error 0x%x or %d)", 
            dev->minor, active_channel->frame_write_index,
            cur_frame->bulk_read_status, 
            cur_frame->bulk_read_status);

        if (cur_frame->bulk_read_status != 0) {
            result=cur_frame->bulk_read_status;          	
        }
        else {
            result=-1;
        }

        reset_frames(active_channel);        	          	
    }

read_exit:
    cur_frame->valid=USBDPFP_FRAME_INVALID;
kref_exit:
    usbdpfp_kref_put(&dev->kref, usbdpfp_delete);
bulk_sem_exit:
    up(&dev->bulk_sem);	
read_error:	
    dbg("device minor %d: result=0x%x or %d )", dev->minor, result, result);		
    return result;
}


/**
*	usbdpfp_write
*/
static ssize_t usbdpfp_write(struct file *filp, const char *buffer, size_t count, loff_t * f_pos)
{
    return -EFAULT;	// NOT IMPLEMENTED
}

/**
*	usbdpfp_llseek
*/
static loff_t usbdpfp_llseek(struct file *filp, loff_t offset, int whence)
{
    // This routine is no longer necessary.

    loff_t newpos=filp->f_pos;
    struct usbdpfp_device *dev = (struct usbdpfp_device *) filp->private_data;	
    kref_get(&dev->kref);
    if (!dev || dev->disconnected) {/* check if the device wasn't unplugged */
        err("bad parameter (dev=0x%p)", dev); 
        newpos = -ENODEV;
        goto llseek_out;
    }

    /* prevent any read or IOCTL(for streaming_read) to run in parallel */
    if (down_interruptible(&dev->bulk_sem)) {
        dbg("device minor %d: acquiring dev->sem failed", dev->minor);
        newpos =  -ERESTARTSYS;
        goto llseek_out;
    }

    switch(whence) {
          case 0: //SEEK_SET
              newpos=offset;
              break;
          case 1: //SEEK_CUR
              newpos=(filp->f_pos+offset);
              break;
          case 2: //SEEK_END
              newpos = (dev->active_channel->max_frames-1)+offset;
              break;
          default:  /* cant happen */
              newpos = -EINVAL;
    }
    if(newpos>=0 && newpos<dev->active_channel->max_frames)
        filp->f_pos = newpos;
    else
        newpos=-EINVAL;
    up(&dev->bulk_sem);

llseek_out:
    usbdpfp_kref_put(&dev->kref, usbdpfp_delete);
    return newpos;
}

/**
*	usbdpfp_ioctl
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int usbdpfp_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#else
static long usbdpfp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
    int result = 0;
    struct usbdpfp_device* dev = (struct usbdpfp_device *) filp->private_data;
    struct usbdpfp_device_info dev_info;

    kref_get(&dev->kref);	

    /* verify that the device wasn't unplugged */
    if (!dev || !dev->udev) {
        err("Invalid USB device object");
        result = -ENODEV;
        goto ioctl_error;
    }
    /* one thread at a time */
    if (down_interruptible(&dev->sem)) {
        dbg("device minor %d: acquiring dev->sem failed", dev->minor);
        result =  -ERESTARTSYS;
        goto ioctl_error;
    }

    dbg("device minor %d: cmd=0x%x, arg=0x%lx", dev->minor, cmd, arg);

    /* device is assumed here for the following switch */
    switch (cmd) {
    case USBDPFP_IOCTL_GET_INFO:
        dbg("device minor %d: code=USBDPFP_IOCTL_GET_INFO, IOC_SIZE=%d", 
            dev->minor, _IOC_SIZE(cmd));

        if((_IOC_DIR(cmd) & _IOC_READ) && access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd))) {
            get_device_info(dev, &dev_info);
            if (copy_to_user((void __user*) arg, &dev_info, sizeof(struct usbdpfp_device_info))) {
                err("device minor %d: copy_to_user() failed", dev->minor);
                result = -EFAULT;
                break;
            }
        } else {
            err("device minor %d: Incorrect command type or size or inaccessible memory", dev->minor);
            result = -EFAULT;
        }
        break;

    case USBDPFP_IOCTL_SET_DATA:         // write to device I/O registey
        dbg("device minor %d: code=USBDPFP_IOCTL_SET_DATA, IOC_SIZE=%d", 
            dev->minor, _IOC_SIZE(cmd));

        if  ((_IOC_DIR(cmd) & _IOC_WRITE) && access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd))) {
            result = usbdpfp_control_pipe_write(dev, arg);
        } else {
            err("device minor %d: Incorrect command type or size or inaccessible memory", dev->minor);
            result = -EFAULT;
        }
        break;

    case USBDPFP_IOCTL_GET_DATA:
        dbg("device minor %d: code=USBDPFP_IOCTL_GET_DATA, IOC_SIZE=%d", dev->minor, _IOC_SIZE(cmd));
        if  ((_IOC_DIR(cmd) & _IOC_READ) && (_IOC_DIR(cmd) & _IOC_WRITE) &&
            access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd))) {
                result = usbdpfp_control_pipe_read(dev, arg);           
        } else {
            err("device minor %d: Incorrect command type or size or inaccessible memory", dev->minor);
            result = -EFAULT;
        }
        break;

    case USBDPFP_IOCTL_WAIT_EVENT:
        dbg("device minor %d: code=USBDPFP_IOCTL_WAIT_EVENT, IOC_SIZE=%d", dev->minor, _IOC_SIZE(cmd));	

        /* we don't want to lock the device during the wait period */
        up(&dev->sem);

        if  ((_IOC_DIR(cmd) & _IOC_READ) && access_ok(VERIFY_WRITE, (void __user*)arg, _IOC_SIZE(cmd))) {
            if (!dev->abort_state) {
                result = usbdpfp_interrupt_pipe_read(dev, arg); /* blocking call */
            } else {
                err("device minor %d: device is in abort state, cannot request intr pipe read", dev->minor);
                result = -EFAULT;				
            }
        } else {
            err("device minor %d: Incorrect command type or size or inaccessible memory", dev->minor);
            result = -EFAULT;
        }

        /* lock again but will be unlocked soon (just to simplify coding logic) */
        if (down_interruptible(&dev->sem)) {
            dbg("device minor %d: acquiring dev->sem failed", dev->minor);
            result =  -ERESTARTSYS;
            goto ioctl_error;
        }

        break;

    case USBDPFP_IOCTL_ABORT_WAIT_EVENT:
        /* to cancel the pending interrupt pipe request 
        * use: usb_unlink_urb(dev->int_in_urb) for aynchronous call (no wait)
        * use: usb_kill_urb(dev->int_in_urb) for synchronous call (wait until the urb
        *      is completely cancelled)
        *
        * Note: It is possible that it does not seen the cancellable urb at this instance 
        *       of moment: right after the urb was submitted and before the cancellable 
        *       flag is set. If the user is sure about a pending request, it should 
        *       retry a few time of this operation.
        */
        dbg("device minor %d: code=USBDPFP_IOCTL_ABORT_EVENT", dev->minor);

        if ((_IOC_DIR(cmd) & _IOC_NONE) == _IOC_NONE) {
            dev->abort_state = 1;	// set abort state to true to prevent further USBDPFP_IOCTL_WAIT_EVENT ioctl
            if (dev->int_in_urb && (atomic_read(&dev->cancelable_int_urb) == 1)) {
                int retries = 5;
                int delay = 20; 
                usb_unlink_urb(dev->int_in_urb); 

                /* Confirm the result of cancellation. 
                If it is not cancelled within a reasonable timeout, 
                we have to abandon the it. 
                */
                do {
                    mdelay(delay);  // delay some ms between check.
                    if (atomic_read(&dev->cancelable_int_urb) == 0) {
                        break;  // has been cancelled.
                    }
                    delay *= 2; /* inc delay */
                } while (retries-- > 0);

                if (retries <= 0) {
                    // exhaust retry without luck, force issue!!     			
                    up(&dev->event_compl_sem);
                }
                dbg("retries=%d, delay=%d", retries, delay);

            } else { 
                /* no urb to cancel (how can we tell the caller?) */      		
                dbg("device minor %d: no interrupt urb pending", dev->minor); 
            }      
        } else {
            err("device minor %d: Incorrect command type or size or inaccessible memory", dev->minor);
            result = -EFAULT;
        }
        break;

    case USBDPFP_IOCTL_ABORT_BULK_READ:
        /*
        Stop the streaming and unlink the URB if it has been submitted. 
        */
        dbg("device minor %d: code=USBDPFP_IOCTL_ABORT_BULK_READ", dev->minor);

        if ((_IOC_DIR(cmd) & _IOC_NONE) == _IOC_NONE)  {
            abort_bulk_read(dev);
        }
        else {
            err("device minor %d: Incorrect command type", dev->minor);
            result = -ENOTTY;
        }
        break;

    case USBDPFP_IOCTL_CONFIG_CHANNEL:
        /*
        Configure the channel with the channel info got from the user space.
        */
        dbg("device minor %d: code=USBDPFP_IOCTL_CONFIG_CHANNEL, IOC_SIZE=%d", 
            dev->minor, _IOC_SIZE(cmd));	

        if  ((_IOC_DIR(cmd) & _IOC_WRITE) && access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd))) {
            int ret;
            struct usbdpfp_channel_info ch_info;              
            if((ret=copy_from_user(&ch_info,(char *)arg, sizeof(struct usbdpfp_channel_info)))){
                err("copy_from_user failed with %x=%d",ret,ret);
                result = -EFAULT;
                break;
            }
            result=config_channel(dev, &ch_info);
        } else {
            err("device minor %d: Incorrect command type or size or inaccessible memory", dev->minor);
            result = -EFAULT;
        }
        break;

    case USBDPFP_IOCTL_SET_ACTIVE_CHANNEL :
        /*
        set the channel specified by the user as active. 
        deallocate all the frame buffers of the previously active channel.
        */    
        dbg("device minor %d: code=USBDPFP_IOCTL_SET_ACTIVE_CHANNEL, IOC_SIZE=%d", 
            dev->minor, _IOC_SIZE(cmd));	

        //          if  (((_IOC_DIR(cmd) & _IOC_NONE) == _IOC_NONE) && USBDPFP_MAX_CHANNELS>=arg){
        if  ((_IOC_DIR(cmd) & _IOC_WRITE) && access_ok(VERIFY_READ, (void __user*)arg, _IOC_SIZE(cmd))) 
        {
            int ch_id, ret; 	
            //struct usbdpfp_channel_config *old_channel=dev->active_channel;
            if((ret=copy_from_user(&ch_id,(int *)arg, sizeof(int)))) {
                err("copy_from_user failed with %x=%d",ret,ret);
                result = -EFAULT;
                break;
            }

            result = set_active_channel(dev, ch_id, &filp->f_pos);
#if 0
            //deallocate the previous channels' buffers.
            if(!result)
                deallocate_frame_buffers(old_channel); 
#endif                    
        } else {
            err("device minor %d: Incorrect command type or size or inaccessible memory", dev->minor);
            result = -EFAULT;
        }
        break;

    default:
        err("device minor %d: Invalid ioctol code 0x%x", dev->minor, cmd);
        result = -ENOTTY;
        break;
    }

    // unlock the device 
    up(&dev->sem);

ioctl_error:
    dbg("ioctl return, result = 0x%x", result);	
    usbdpfp_kref_put(&dev->kref, usbdpfp_delete);
    return result;
}

//software device for Pnp event notification
static struct usbdpfp_pnp_device *pnp_dev;

/*
* These two callbacks are invoked when an USB device is detached or attached
* to the bus
*/
static int usbdpfp_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct usbdpfp_device *dev = NULL;
    struct usb_endpoint_descriptor *endpoint;
    struct usb_host_interface *iface_desc;
    int ep_cnt = 0;
    int ret = -ENOMEM;

    dbg("called for device %04X %04X", id->idVendor, id->idProduct);

    dev = usbdpfp_new();
    if (!dev) {
        ret = -ENOMEM;
        goto error;
    }

    dev->udev = usb_get_dev(interface_to_usbdev(interface));
    dev->interface = interface;
    //Store the device name and serial no. for later use.
    usb_string(dev->udev, 2, (void*)dev->dev_name, 80);
    usb_string(dev->udev, 3, (void*)dev->serial_no, 80);
    iface_desc = interface->cur_altsetting;
    ep_cnt = 0;
    while (ep_cnt < iface_desc->desc.bNumEndpoints) {
        endpoint = &(iface_desc->endpoint[ep_cnt].desc);
        ep_cnt++;
        if (IS_EP_BULK_IN(endpoint)) {
            if (dev->bulk_in_ep) {
                dbg("ignoring additional bulk in ep: %d", ep_cnt);
                continue;
            }
            dev->bulk_in_ep = endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
            dbg("Found bulk in endpoint %d", dev->bulk_in_ep);
            continue;
        }

        if (IS_EP_INTR(endpoint)) {
            if (dev->int_in_ep) {
                dbg("ignoring additional int in ep: %d", ep_cnt);
                continue;
            }
            dev->int_in_ep = endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
            dev->int_in_int = endpoint->bInterval;
            dbg("Found interrupt in endpoint %d, interval %d (ms)", dev->int_in_ep, 
                dev->int_in_int);
            continue;
        }
        dbg("unknown endpoint detected -- ABORTING");
        ret = -EINVAL;
        goto error;
    }

    usb_set_intfdata(interface, dev);

    if((ret = usb_register_dev(interface, &usbdpfp_class))) {
        err("Unable to register device with USB stack, error=%d", ret);
        usb_set_intfdata(interface, NULL);
        goto error;
    }

    //   dbg("Fingerprint scanner device now attached to usbdpfp%d", interface->minor-USB_USBDPFP_MINOR_BASE);

    //	 Here, usb_dev is same as the class_dev i.e., pointer to the usb class's device, 
    //	 and bus_id is same as the class_id i.e., position on parent bus which is unique to the this class.
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
    dbg("Device attached %s\n", dev_name(interface->usb_dev));
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
    dbg("Device attached %s\n", interface->usb_dev->bus_id); 
#else
    dbg("Device attached %s\n", interface->class_dev->class_id);
#endif

    //PNP event handling extra...
    if(down_interruptible(&pnp_dev->event_lock)) {
        dbg("pnp: acquiring dev->sem failed");
        ret =  -ERESTARTSYS;
        goto error;
    }

    if( (ret=usbdpfp_add_attach_event( interface )) ) {
        err("unable to add an attach event err=%d",ret);
        up(&pnp_dev->event_lock);
        goto error;
    }
    up(&pnp_dev->event_lock);

    return 0;

error:
    if(dev) {
        usbdpfp_kref_put(&dev->kref, usbdpfp_delete);
    }
    return ret;
}

static void usbdpfp_disconnect(struct usb_interface *interface)
{
    struct usbdpfp_device *dev;

    //dbg("Device usbdpfp%d now disconnected", interface->minor-USB_USBDPFP_MINOR_BASE);

    // PNP event handling extra...
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
    dbg("Device removed %s\n", dev_name(interface->usb_dev));
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
    dbg("Device removed %s\n", interface->usb_dev->bus_id); 
#else
    dbg("Device removed %s\n", interface->class_dev->class_id);
#endif

    do {
        if(down_interruptible(&pnp_dev->event_lock)) {
            dbg("pnp: acquiring dev->sem failed; Couldnot handle detach event");
            break;
        }

        usbdpfp_handle_detach_event( interface );

        up(&pnp_dev->event_lock);
    }while(0);

    //lock_kernel();
    dev = usb_get_intfdata(interface);
    dev->disconnected = 1;
    wake_up(&dev->inq);

    usb_set_intfdata(interface, NULL);
    usb_deregister_dev(interface, &usbdpfp_class);
    //unlock_kernel();

    usbdpfp_kref_put(&dev->kref, usbdpfp_delete);
}


/*!
usbdpfp_suspend
Called when the application which has opened the USB device is stopped and 
before the system power state changes.
unlink the URBs, next time system shifts to S0 state the Urb's will be re-issued.  
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
static int usbdpfp_suspend(struct usb_interface *interface, u32 state)
#else
static int usbdpfp_suspend(struct usb_interface *interface, pm_message_t message)
#endif
{
    //#ifdef USBDPFP_ENABLE_SUSPEND
    struct usbdpfp_device *dev;
    int retries = 5;
    int delay = 20;

    dev = usb_get_intfdata(interface);

    if (dev && dev->isopen && (dev->int_in_urb || dev->bulk_in_urb))
    {
        // check and cancel pending interrupt urb 
        if(atomic_read(&dev->cancelable_int_urb) == 1) {  
            atomic_set(&dev->suspended, 1);		// set to original state.
            usb_unlink_urb(dev->int_in_urb);  	// Unlink pending interrupt urb.
        }

        // check and cancel pending bulk urb
        if(atomic_read(&dev->cancelable_bulk_urb) == 1) { 
            usb_unlink_urb(dev->bulk_in_urb); 
        } // Unlink pendong bulk urb.

        // Confirm the result of cancellation. If it is not cancelled within a
        // reasonable timeout, we have to abandon it. 
        do {
            mdelay(delay);  // delay some ms between check
            if (atomic_read(&dev->cancelable_int_urb) == 0 && 
                atomic_read(&dev->cancelable_bulk_urb) == 0) {
                    break;  // has been cancelled
            }
            delay *= 2;
        } while (retries-- > 0);

        dbg("Int-URB cancelled");
    } else {
        dbg("No Pending Int-URBs");
    }
    //#endif

    return 0;
}

/*
* usbdpfp_resume
*/
static int usbdpfp_resume(struct usb_interface *interface)
{
    int ret = 0;

    dbg("usbdpfp Resume");

    //PNP event handling
    if(down_interruptible(&pnp_dev->event_lock)) {
        dbg("pnp: acquiring dev->sem failed");
        ret =  -ERESTARTSYS;
        goto error;
    }

    if( (ret=usbdpfp_add_resume_event(interface)) ) {
        err("unable to add power resume event err=%d", ret);
        up(&pnp_dev->event_lock);
        goto error;
    }
    up(&pnp_dev->event_lock);

error:
    return ret;
}


static int usbdpfp_pnp_open( struct inode *inode, struct file *filp );
static int usbdpfp_pnp_release( struct inode *inode, struct file *filp );

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int usbdpfp_pnp_ioctl( struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg );
#else
static long usbdpfp_pnp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg );
#endif

/*!
Add an event to the new event list, at the end, to be consumed by the PNP IOCTL. 
Also wakes up the IOCTL which waits for event on new_events list.
Called by Probe when a new device is probed successfully.
*/
static int usbdpfp_add_event( struct usb_interface *interface, int detach_state ) 
{
    int retval=0;
    struct usbdpfp_device *dev;
    struct usbdpfp_pnp_node *node;

    dev = usb_get_intfdata(interface);
    if(!dev) {
        retval = -ENODEV;
        goto out_error;
    }

    if(NULL == (node = mempool_alloc(pnp_dev->pool, GFP_ATOMIC))) {
        dbg("couldnot alloc node\n");
        retval = -ENOMEM;
        goto out_error;
    }


    //	 Here, usb_dev is same as the class_dev i.e., pointer to the usb class's device, 
    //	 and bus_id is same as the class_id i.e., position on parent bus which is unique to the this class.
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
    strncpy(node->event.dev_file_name, dev_name(interface->usb_dev), USBDPFP_NAME_BUFF_SIZE); 
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
    strncpy(node->event.dev_file_name, interface->usb_dev->bus_id, USBDPFP_NAME_BUFF_SIZE); 
#else
    strncpy(node->event.dev_file_name, interface->class_dev->class_id, USBDPFP_NAME_BUFF_SIZE); 
#endif	

    node->event.detach_state = detach_state;
    get_device_info(dev, &node->event.dev_info);
    node->minor = interface->minor;
    INIT_LIST_HEAD(&node->event_list);
    list_add_tail(&node->event_list, &pnp_dev->new_events);

    if(pnp_dev->is_active)
        wake_up_interruptible(&pnp_dev->wait_queue);
out_error:	
    return retval;
}

/*! 
Wrapper function to add an attach event to the new_events list.
*/ 
static int usbdpfp_add_attach_event( struct usb_interface *interface )
{
    return usbdpfp_add_event(interface, PNP_STATE_ATTACH);
}

/*!
Wrapper function to add a detach event to the new_events list.
*/
static inline int usbdpfp_add_detach_event( struct usb_interface *interface )
{
    return usbdpfp_add_event(interface, PNP_STATE_DETACH);
}

static int usbdpfp_add_resume_event( struct usb_interface *interface )
{
    return usbdpfp_add_event(interface, PM_STATE_UP);
}

/*!
Called by the drivers' disconnect function when a device is plugged out.
If there is a consumer of the events, it adds the disconnect event to the 
end of the new_events list, to be consumed. 
else if no consumer present, checks for the matching attach event in 
new_events and reported_events list and deletes them(report only the 
devices which are present.).  
*/
static int usbdpfp_handle_detach_event( struct usb_interface *interface ) 
{
    struct usbdpfp_pnp_node *node, *node_match=NULL;
    struct list_head *list, *temp_list;
    if(!pnp_dev->is_active) {
        //search the new_events list
        list_for_each_safe(list, temp_list, &pnp_dev->new_events) {
            node = list_entry(list, struct usbdpfp_pnp_node, event_list);
            if(interface->minor == node->minor) {
                node_match = node;
                list_del(&node_match->event_list);
                break;
            }
        }
        if(NULL == node_match) { // match not found in the new_events list
            //search the reported_events list
            list_for_each_safe(list, temp_list, &pnp_dev->reported_events) {
                node = list_entry(list, struct usbdpfp_pnp_node, event_list);
                if( interface->minor == node->minor) {
                    node_match = node;
                    list_del(&node_match->event_list);
                    break;
                }
            }
        }
        if(node_match) {  //this check should not be necessary
            mempool_free(node_match, pnp_dev->pool);
        }
    }
    else{
        list_for_each_safe(list, temp_list, &pnp_dev->new_events) {
            node = list_entry(list, struct usbdpfp_pnp_node, event_list);
            if(interface->minor == node->minor) {
                node_match = node;
                list_del(&node_match->event_list);
                break;
            }
        }
        if(node_match)
            mempool_free(node_match, pnp_dev->pool);
        else  
            usbdpfp_add_detach_event(interface);
    }
    return 0;
}


static struct file_operations pnp_fops = {
    .owner     = THIS_MODULE,
    .open      = usbdpfp_pnp_open,
    .release   = usbdpfp_pnp_release,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    .ioctl     = usbdpfp_pnp_ioctl,
#else
    .unlocked_ioctl = usbdpfp_pnp_ioctl,
#endif
};

/*!
open entry point for the PNP driver.
Allow only one process to open the PNP device.
*/
static int usbdpfp_pnp_open( struct inode *inode, struct file *filp ) 
{
    if(!test_and_set_bit(PNP_ACTIVE_BIT, &pnp_dev->is_active)) {
        return 0;
    }
    return -EAGAIN;
}

/*!
release entry point for the PNP driver.
Add the events that were reported, to the new_event list,
to be reported to next application that opens the PNP device.
*/
static int usbdpfp_pnp_release( struct inode *inode, struct file *filp )
{
    int result=0;
    struct list_head *list, *temp_list;

    //Move all the reported events to new events list
    if(down_interruptible(&pnp_dev->event_lock)) {
        dbg("pnp: acquiring sem failed");
        result =  -ERESTARTSYS;
        goto error;
    }

    list_for_each_safe(list, temp_list, &pnp_dev->reported_events) {
        list_del_init(list);
        list_add(list, &pnp_dev->new_events);
    }
    up(&pnp_dev->event_lock);

    pnp_dev->terminated = PNP_STATE_NOT_TERMINATED;
    clear_bit(PNP_ACTIVE_BIT, &pnp_dev->is_active);
error:          

    return result;
}


/*! 
IOCTL entry point to the PNP device.
Allows application to wait for the PNP event. 
Allows the cancellation of the wait. 
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int usbdpfp_pnp_ioctl( struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg )
#else
static long usbdpfp_pnp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg )
#endif
{     
    int err = 0;
    struct list_head *list, *temp;
    struct usbdpfp_pnp_node *node, *rep_node;

    if(_IOC_TYPE(cmd) != USBDPFP_IOC_MAGIC) { 
        return -ENOTTY;
    }

    switch(cmd) {
          case USBDPFP_IOCTL_CANCEL_WAIT_PNP_EVENT :
              dbg("pnp: cancel_wait_pnp_event");
              pnp_dev->terminated = PNP_STATE_TERMINATED;
              wake_up_interruptible(&pnp_dev->wait_queue);
              break;

          case USBDPFP_IOCTL_WAIT_PNP_EVENT :
              dbg("pnp: wait_pnp_event");
              if(_IOC_DIR(cmd) & _IOC_READ) {
                  err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
              }
              if(err) {
                  err("pnp: invalid argument");
                  return -EFAULT;
              }
              if( list_empty(&pnp_dev->new_events)) {
                  init_wait((&pnp_dev->wait)); //initialize the wait with the current process.
                  //goto sleep and wake up when probe, disconnect or ioctl wakes you up
                  prepare_to_wait(&pnp_dev->wait_queue, &pnp_dev->wait, TASK_INTERRUPTIBLE);
                  schedule();
                  finish_wait(&pnp_dev->wait_queue, &pnp_dev->wait);
                  if(signal_pending(current)) {
                      dbg("pnp: signal_pending error or interrupted");
                      return -ERESTARTSYS;
                  }
              }
              if(pnp_dev->terminated) {
                  pnp_dev->terminated = PNP_STATE_NOT_TERMINATED;
                  return USBDPFP_IOCTL_CANCEL_WAIT_PNP_EVENT;
              }
              if(down_interruptible(&pnp_dev->event_lock)) {
                  dbg("pnp: acquiring sem failed or interrupted");
                  return -ERESTARTSYS;                         
              }
              if( !list_empty(&pnp_dev->new_events)) {
                  list = pnp_dev->new_events.next;
                  list_del_init(list);	// reinitialize the linked list pointers so it can be inserted to another list
                  node = list_entry(list, struct usbdpfp_pnp_node, event_list);	// get the pointer of the container struc
                  if(copy_to_user((void *)arg, &node->event, sizeof(struct usbdpfp_device_pnp_event))) {
                      err("pnp: could not copy the whole pnp event data to userspace");
                  }

                  if(PNP_STATE_ATTACH == node->event.detach_state) { 
                      dbg("pnp: pnp event type is - connect (%d)", node->event.detach_state);                     
                      list_add(list, &pnp_dev->reported_events);
                  }
                  else if (PNP_STATE_DETACH == node->event.detach_state) { 
                      dbg("pnp: pnp event type is - disconnect (%d)", node->event.detach_state);
                      list_for_each_safe(list, temp, &pnp_dev->reported_events) {
                          rep_node = list_entry(list, struct usbdpfp_pnp_node, event_list);
                          if(rep_node->minor ==  node->minor) {
                              list_del(list);
                              mempool_free(rep_node, pnp_dev->pool);
                              break;
                          }
                      }
                  }
                  else if (PM_STATE_UP == node->event.detach_state) {
                      dbg("pnp: pnp event type is - power up (%d)", node->event.detach_state);
                  }
                  else if (PM_STATE_DOWN == node->event.detach_state) {
                      dbg("pnp: pnp event type is - power down (%d)", node->event.detach_state);
                  }
                  else {
                      dbg("pnp: pnp event type is - unknown (%d)", node->event.detach_state);
                  }
              }
              else {
                  dbg("pnp: wait_queue wakes but pnp event list is empty");
              }

              up(&pnp_dev->event_lock);
              break;
          default :
              return -ENOTTY;
    }

    return 0;
}

static int __init usbdpfp_init(void)
{
    int ret;
    dbg("Registering U.are.U Fingerprint Reader Driver %s %s (debug=%d)", 
        MODULE_NAME, DRIVER_VERSION, debug);

    // create and initialize pnp_dev
    pnp_dev = (struct usbdpfp_pnp_device *) usbdpfp_kmalloc(sizeof(struct usbdpfp_pnp_device), GFP_KERNEL);
    if(pnp_dev == NULL) {
        ret = -EAGAIN;
        dbg("Could not allocate pnp device\n");
        goto fail_pnp_dev;
    }

    memset(pnp_dev, 0, sizeof(struct usbdpfp_pnp_device));
    INIT_LIST_HEAD(&pnp_dev->new_events);
    INIT_LIST_HEAD(&pnp_dev->reported_events);
    init_MUTEX(&pnp_dev->event_lock);
    init_waitqueue_head(&pnp_dev->wait_queue);

    //create a memory pool
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
    pnp_dev->cache = kmem_cache_create("Node_Pool", sizeof(struct usbdpfp_pnp_node), 0, SLAB_HWCACHE_ALIGN, NULL, NULL);
#else
    pnp_dev->cache = kmem_cache_create("Node_Pool", sizeof(struct usbdpfp_pnp_node), 0, SLAB_HWCACHE_ALIGN, NULL      );
#endif  
    if(NULL == pnp_dev->cache) {
        ret = -EAGAIN;
        dbg("Could not create lookaside cache\n");
        goto fail_pnp_cache;
    }

    if(NULL == (pnp_dev->pool = mempool_create(PNP_MAX_NODES,
        mempool_alloc_slab, mempool_free_slab,
        pnp_dev->cache))) {
            ret = -EAGAIN;
            dbg("Could not create memory pool\n"); 
            goto fail_pnp_pool;
    }  

    // request/register pnp char device (dynamically or statically).
    // The device number and name "usbdpfp_pnp_dev" appears in /proc/devices.
    if (usbdpfp_pnp_major) // static assignment of device major
    {        
        dbg("Register Pnp char driver device major (%d) statically \n", usbdpfp_pnp_major);

        pnp_dev->devno = MKDEV(usbdpfp_pnp_major, usbdpfp_pnp_minor);
        if((ret = register_chrdev_region(pnp_dev->devno, PNP_NR_DEVS, "usbdpfp_pnp_dev")) < 0 ){
            dbg("could\'nt register the Pnp char dev number with major %d and minor:%d \n", 
                PNP_MAJOR_NUM, PNP_MINOR_START);
            goto fail_chardev_region;
        }
    }
    else {     // dynamically allocation of device major

        dbg("Allocate Pnp char driver device major dynamically\n");

        if((ret = alloc_chrdev_region(&pnp_dev->devno, usbdpfp_pnp_minor, PNP_NR_DEVS, "usbdpfp_pnp_dev")) < 0 ){
            dbg("could\'nt alloc the char dev region \n");
            goto fail_chardev_region;
        }
    }

    // initialize the cdev and add it to the kernel
    cdev_init(&pnp_dev->cdev, &pnp_fops); 
    pnp_dev->cdev.owner = THIS_MODULE;  
    if((ret = cdev_add(&pnp_dev->cdev, pnp_dev->devno, PNP_NR_DEVS)) < 0) {       
        dbg("Adding cdev failed\n");      
        goto fail_cdev_add;
    }

    // export device number (major/minor) info to /sys/usbdpfpPnpClass/usbdpfpPnp/dev
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
    if( IS_ERR(pnp_dev->class = class_simple_create(THIS_MODULE, "usbdpfpPnpClass"))) {
        ret = -EAGAIN;
        goto fail_class_create;
    }
#else 
    if( IS_ERR(pnp_dev->class = class_create(THIS_MODULE, "usbdpfpPnpClass"))) {
        ret = -EAGAIN;
        goto fail_class_create;
    }
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
    if(IS_ERR(class_simple_device_add(pnp_dev->class, pnp_dev->devno, NULL, "usbdpfpPnp"))) {
        ret = -EAGAIN;
        goto fail_class_device_create;
    }
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16)
    if(IS_ERR(class_device_create(pnp_dev->class, pnp_dev->devno, NULL, "usbdpfpPnp"))) {
        ret = -EAGAIN;
        goto fail_class_device_create;
    }
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
    if(IS_ERR(class_device_create(pnp_dev->class, NULL, pnp_dev->devno, NULL, "usbdpfpPnp"))) {
        ret = -EAGAIN;
        goto fail_class_device_create;
    }
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
    if(IS_ERR(device_create(pnp_dev->class, NULL, pnp_dev->devno, "usbdpfpPnp"))) {
        ret = -EAGAIN;
        goto fail_class_device_create;
    }
#else
    if(IS_ERR(device_create(pnp_dev->class, NULL, pnp_dev->devno, NULL, "usbdpfpPnp"))) {
        ret = -EAGAIN;
        goto fail_class_device_create;
    }
#endif
#endif
    //register the usb driver interface   
    ret = usb_register(&usbdpfp_usb_driver);
    if (ret) {
        err("Unable to register U.are.U Fingerprint Reader Driver");
        goto fail_usb_register;
    }

    return ret;

fail_usb_register:  
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
    class_simple_device_remove(pnp_dev->devno);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
    class_device_destroy(pnp_dev->class, pnp_dev->devno);
#else
    device_destroy(pnp_dev->class, pnp_dev->devno);
#endif

fail_class_device_create:
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
    class_simple_destroy(pnp_dev->class);
#else
    class_destroy(pnp_dev->class);
#endif

fail_class_create:  
    cdev_del(&pnp_dev->cdev);
fail_cdev_add: 
    unregister_chrdev_region(pnp_dev->devno, PNP_NR_DEVS);
fail_chardev_region:

    //The new_events and the reported_events list should be empty 
    //before doing this.
    mempool_destroy(pnp_dev->pool);
fail_pnp_pool:
    kmem_cache_destroy(pnp_dev->cache);   
fail_pnp_cache:
    usbdpfp_kfree(pnp_dev);
fail_pnp_dev:
    return ret;
}

static void __exit usbdpfp_exit(void)
{
    int count=2;
    struct usbdpfp_pnp_node *node;
    struct list_head *list, *tmp;

    dbg("Unregistering U.are.U Fingerprint Reader Driver %s %s", MODULE_NAME, DRIVER_VERSION);
    usb_deregister(&usbdpfp_usb_driver);

    //Delete /dev/usbdpfpPnp
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
    class_simple_device_remove(pnp_dev->devno);
    class_simple_destroy(pnp_dev->class);
#elif LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
    class_device_destroy(pnp_dev->class, pnp_dev->devno);
    class_destroy(pnp_dev->class);
#else
    device_destroy(pnp_dev->class, pnp_dev->devno);
    class_destroy(pnp_dev->class);
#endif

    cdev_del(&pnp_dev->cdev);
    unregister_chrdev_region(pnp_dev->devno, PNP_NR_DEVS );

    //Free all the allocated nodes left in the lists back to the pool. 
    do {    //Try to acquire the Sem in 3 chances 
        if(down_interruptible(&pnp_dev->event_lock)) {
            dbg("acquiring sem failed; Could not release all the events");
            continue;
        }
        break;
    }while(!count--);

    // Empty the Event lists before we remove the event memory pools.
    if(!list_empty(&pnp_dev->new_events))
        list_for_each_safe(list, tmp, &pnp_dev->new_events) {
            list_del(list);
            node = list_entry(list, struct usbdpfp_pnp_node, event_list);
            mempool_free(node, pnp_dev->pool);
    }
    if(!list_empty(&pnp_dev->reported_events))
        list_for_each_safe(list, tmp, &pnp_dev->reported_events) {
            list_del(list);
            node = list_entry(list, struct usbdpfp_pnp_node, event_list);
            mempool_free(node, pnp_dev->pool);
    }
    up(&pnp_dev->event_lock);
    mempool_destroy(pnp_dev->pool);
    kmem_cache_destroy(pnp_dev->cache); 
    usbdpfp_kfree(pnp_dev);
}

module_init(usbdpfp_init);
module_exit(usbdpfp_exit);


// To enable debugging of our driver, we can either build it with the
// CONFIG_USB_DEBUG macro, or you can enable it at runtime with this flag.
#ifdef CONFIG_USB_DEBUG
int debug = 1;
int mdebug = 0;
#else
int debug = 0;
int mdebug = 0;
#endif
module_param(debug, int, 0);
module_param(mdebug, int, 0);
MODULE_PARM_DESC(debug, "Enable debug");

// Allow configurable PnP char driver major/minor device number by insmod
// If usbdpfp_pnp_major is 0, it uses dynamically allocation.
int usbdpfp_pnp_major = PNP_MAJOR_NUM;
int usbdpfp_pnp_minor = PNP_MINOR_START;
module_param(usbdpfp_pnp_major, int, 0);
module_param(usbdpfp_pnp_minor, int, 0);

/*
char *device_name = NULL;
module_param(device_name, charp, 0);
MODULE_PARM_DESC(device_name, "Device name");
*/

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC " " DRIVER_VERSION);
MODULE_LICENSE("GPL");
