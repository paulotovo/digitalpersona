/** 
\file dpfpdd.h

\copyright (c) 2011 DigitalPersona, Inc.

\brief U.are.U SDK DP Capture API

Data types and functions to access fingerprint readers.

\version 2.0.0
*/

#ifndef _DPFPDD_API_H_
#define _DPFPDD_API_H_

/** \cond NEVER */
#ifndef DPAPICALL
#	if defined(_WIN32) || defined(_WIN64)
#		ifdef WINCE
#			define DPAPICALL __cdecl
#		else
#			define DPAPICALL __stdcall
#		endif
#	else
#		define DPAPICALL
#	endif
#endif

#ifndef NULL
#	ifdef __cplusplus
#		define NULL    0
#	else
#		define NULL    ((void *)0)
#	endif
#endif

#ifndef DPERROR
#	define _DP_FACILITY  0x05BA
#	define DPERROR(err)  ((int)err | (_DP_FACILITY << 16))
#endif /* DPERROR */

/* api version 1.9 */
#define DPFPDD_API_VERSION_MAJOR 1
#define DPFPDD_API_VERSION_MINOR 9
/** \endcond */


/****************************************************************************************************
 Error codes
****************************************************************************************************/

/** 
\brief API call succeeded. 
*/
#define DPFPDD_SUCCESS             0

/** 
\brief API call is not implemented.
*/
#define DPFPDD_E_NOT_IMPLEMENTED   DPERROR(0x0a)

/**
\brief Unspecified failure.

"Catch-all" generic failure code. Can be returned by all API calls in case of failure, when the reason for the failure is unknown or cannot be specified.
*/
#define DPFPDD_E_FAILURE           DPERROR(0x0b)

/**
\brief No data is available.
*/
#define DPFPDD_E_NO_DATA           DPERROR(0x0c)

/**
\brief The memory allocated by the application is not big enough for the data which is expected.
*/
#define DPFPDD_E_MORE_DATA         DPERROR(0x0d)

/**
\brief One or more parameters passed to the API call are invalid.
*/
#define DPFPDD_E_INVALID_PARAMETER DPERROR(0x14)

/**
\brief Reader handle is not valid.
*/
#define DPFPDD_E_INVALID_DEVICE    DPERROR(0x15)

/** 
\brief The API call cannot be completed because another call is in progress.
*/
#define DPFPDD_E_DEVICE_BUSY       DPERROR(0x1e)

/**
\brief The reader is not working properly.
*/
#define DPFPDD_E_DEVICE_FAILURE    DPERROR(0x1f)


/****************************************************************************************************
 Data types and structures
****************************************************************************************************/

/** 
\brief Reader handle. 

Calling dpfpdd_open() connects to a device and returns a handle.
Open handles must be released when no longer needed by calling dpfpdd_close().
*/
typedef void* DPFPDD_DEV;

/**
\brief API version information. 
*/
typedef struct dpfpdd_ver_info {
	int major;       /**< major version number */
	int minor;       /**< minor version number */
	int maintenance; /**< maintenance or revision number */
} DPFPDD_VER_INFO;

/**
\brief Complete information about library/SDK.
*/
typedef struct dpfpdd_version {
	unsigned int    size;    /**< size of the structure, in bytes	*/		
	DPFPDD_VER_INFO lib_ver; /**< file version of the SDK/library */
	DPFPDD_VER_INFO api_ver; /**< version of the API */
} DPFPDD_VERSION;

/**
\brief Reader modality.
*/
typedef unsigned int DPFPDD_HW_MODALITY;
#define DPFPDD_HW_MODALITY_UNKNOWN 0 /**< modality is not known */
#define DPFPDD_HW_MODALITY_SWIPE   1 /**< swipe reader */
#define DPFPDD_HW_MODALITY_AREA    2 /**< area or placement reader */

/** 
\brief Reader technology.
*/
typedef unsigned int DPFPDD_HW_TECHNOLOGY;
#define DP_HW_TECHNOLOGY_UNKNOWN     0 /**< technology is not known */
#define DP_HW_TECHNOLOGY_OPTICAL     1 /**< optical reader */
#define DP_HW_TECHNOLOGY_CAPACITIVE  2 /**< capacitive reader */
#define DP_HW_TECHNOLOGY_THERMAL     3 /**< thermal reader */
#define DP_HW_TECHNOLOGY_PRESSURE    4 /**< pressure reader */

/**
\brief Maximum length of the strings in the descriptors, in bytes.
*/
#define MAX_STR_LENGTH 128

/**
\brief Maximum length of the reader name.
*/
#define MAX_DEVICE_NAME_LENGTH 1024

/**
\brief Reader hardware descriptor.
*/
typedef struct dpfpdd_hw_descr {
	char vendor_name[MAX_STR_LENGTH];  /**< name of the vendor */
	char product_name[MAX_STR_LENGTH]; /**< name of the product */
	char serial_num[MAX_STR_LENGTH];   /**< serial number */
} DPFPDD_HW_DESCR;

/**
\brief Reader Hardware ID.
*/
typedef struct dpfpdd_hw_id {
	unsigned short  vendor_id;  /**< vendor ID (USB VID) */
	unsigned short  product_id; /**< product ID (USB PID) */
} DPFPDD_HW_ID;

/**
\brief Reader hardware version.
*/
typedef struct dpfpdd_hw_version {
 	DPFPDD_VER_INFO hw_ver; /**< hardware version */
 	DPFPDD_VER_INFO fw_ver; /**< firmware version */
 	unsigned short bcd_rev;	/**< USB bcd revision */
} DPFPDD_HW_VERSION;

/**
\brief Complete information about reader hardware.
*/
typedef struct dpfpdd_dev_info {
	unsigned int         size; /**< size of the structure */
	char                 name[MAX_DEVICE_NAME_LENGTH]; /**< unique name of the reader */
	DPFPDD_HW_DESCR	     descr;       /**< displayable information about reader */
	DPFPDD_HW_ID         id;          /**< USB ID */
	DPFPDD_HW_VERSION    ver;         /**< reader hardware version information */
 	DPFPDD_HW_MODALITY   modality;    /**< reader modality */
 	DPFPDD_HW_TECHNOLOGY technology;  /**< reader technology */
} DPFPDD_DEV_INFO;

/**
\brief Constants describing priority of the client opening the reader (Windows-only)
*/
typedef unsigned int DPFPDD_PRIORITY;
#define DPFPDD_PRIORITY_COOPERATIVE  2    /**< Client uses this priority to open reader in cooperative mode. Multiple clients with this priority are allowed. Client receives captured images if it has window with focus. */
#define DPFPDD_PRIORITY_EXCLUSIVE    4    /**< Client uses this priority to open reader exclusively. Only one client with this priority is allowed. */

/**
\brief Information about reader capabilites.
*/
typedef struct dpfpdd_dev_caps {
 	unsigned int size; /**< size of the structure */
	int          can_capture_image;	    /**< flag: reader can capture images */
	int          can_stream_image;      /**< flag: reader can stream images */
	int          can_extract_features;  /**< flag: reader can extract features from captured image and return fingerprint features data */
	int          can_match;             /**< flag: reader can perform match one-to-one */
	int          can_identify;          /**< flag: reader can perform match one-to-many */
	int          has_fp_storage;        /**< flag: reader has storage for fingerprint features data */
	unsigned int indicator_type;        /**< bitmask: existing LEDs */
	int          has_pwr_mgmt;          /**< flag: power mode of the reader can be controlled  */
	int          has_calibration;       /**< flag: reader can be calibrated */
	int          piv_compliant;         /**< flag: can produce PIV compliant images */
	unsigned int resolution_cnt;        /**< counter: number of the image resolutions reader can produce */
	unsigned int resolutions[1];        /**< array: available resolutions */
} DPFPDD_DEV_CAPS;

/**
\brief Constants describing status of the reader
*/
typedef unsigned int DPFPDD_STATUS;
#define DPFPDD_STATUS_READY             0 /**< ready for capture */
#define DPFPDD_STATUS_BUSY              1 /**< cannot capture, another operation is in progress */
#define DPFPDD_STATUS_NEED_CALIBRATION  2 /**< ready for capture, but calibration needs to be performed soon */
#define DPFPDD_STATUS_FAILURE           3 /**< cannot capture, reset is needed */

/**
\brief Describes status of the reader
*/
typedef struct  dpfpdd_dev_status {
	unsigned int  size;            /**< total size of the allocated memory including size of additional data */
	DPFPDD_STATUS status;          /**< reader status */
	int           finger_detected; /**< flag: finger detected on the reader */
	unsigned char data[1];         /**< additional vendor-specific data which may be passed by the driver */
} DPFPDD_DEV_STATUS;

/**
\brief Result of the capture operation
*/
typedef unsigned int DPFPDD_QUALITY;
#define DPFPDD_QUALITY_GOOD                 0       /**< capture succeeded */
#define DPFPDD_QUALITY_TIMED_OUT            1       /**< timeout expired */
#define DPFPDD_QUALITY_CANCELED             (1<<1)  /**< capture was canceled */
#define DPFPDD_QUALITY_NO_FINGER            (1<<2)  /**< non-finger detected */
#define DPFPDD_QUALITY_FAKE_FINGER          (1<<3)  /**< fake finger detected */
#define DPFPDD_QUALITY_FINGER_TOO_LEFT      (1<<4)  /**< finger is too far left on the reader */
#define DPFPDD_QUALITY_FINGER_TOO_RIGHT     (1<<5)  /**< finger is too far right on the reader */
#define DPFPDD_QUALITY_FINGER_TOO_HIGH      (1<<6)  /**< finger is too high on the reader */
#define DPFPDD_QUALITY_FINGER_TOO_LOW       (1<<7)  /**< finger is too low in the reader */
#define DPFPDD_QUALITY_FINGER_OFF_CENTER    (1<<8)  /**< finger is not centered on the reader */
#define DPFPDD_QUALITY_SCAN_SKEWED          (1<<9)  /**< scan is skewed too much */
#define DPFPDD_QUALITY_SCAN_TOO_SHORT       (1<<10) /**< scan is too short */
#define DPFPDD_QUALITY_SCAN_TOO_LONG        (1<<11) /**< scan is too long */
#define DPFPDD_QUALITY_SCAN_TOO_SLOW        (1<<12) /**< speed of the swipe is too slow */
#define DPFPDD_QUALITY_SCAN_TOO_FAST        (1<<13) /**< speed of the swipe is too fast */
#define DPFPDD_QUALITY_SCAN_WRONG_DIRECTION (1<<14) /**< direction of the swipe is wrong */
#define DPFPDD_QUALITY_READER_DIRTY         (1<<15) /**< reader needs cleaning */

/**
\brief Format of captured fingerprint image.
*/
typedef unsigned int DPFPDD_IMAGE_FMT;
#define DPFPDD_IMG_FMT_PIXEL_BUFFER 0          /**< "raw" format, pixel buffer without a header */
#define DPFPDD_IMG_FMT_ANSI381      0x001B0401 /**< ANSI INSITS 381-2004 format */
#define DPFPDD_IMG_FMT_ISOIEC19794  0x01010007 /**< ISO IEC 19794-4-2005 format */

/**
\brief Image processing.
*/

typedef unsigned int DPFPDD_IMAGE_PROC;
#define DPFPDD_IMG_PROC_DEFAULT               0 /**< Recommended processing. This is to acquire image as fast as possible. The image is suitable for feature extraction. */
#define DPFPDD_IMG_PROC_PIV                   1 /**< To request image which will be PIV compliant. Not every reader supports this mode. The image is suitable for feature extraction. */
#define DPFPDD_IMG_PROC_ENHANCED              2 /**< To request visually enhanced image. The image is suitable for feature extraction. */
#define DPFPDD_IMG_PROC_UNPROCESSED  0x52617749 /**< To request image which is not processed in any way. The image is NOT suitable for feature extraction. */

#define DPFPDD_IMG_PROC_NONE     DPFPDD_IMG_PROC_DEFAULT /**< for backward compatibility */

/**
\brief Describes image parameters for capture
*/
typedef struct dpfpdd_capture_param {
 	unsigned int      size;       /**< size of the structure */
 	DPFPDD_IMAGE_FMT  image_fmt;  /**< format of the image */
	DPFPDD_IMAGE_PROC image_proc; /**< processing of the image */
 	unsigned int      image_res;  /**< resolution of the image */
} DPFPDD_CAPTURE_PARAM;

/**
\brief Describes captured image

The output parameter of the dpfpdd_capture() and dpfpdd_get_stream_image() functions. 
*/
typedef struct dpfpdd_image_info {
	unsigned int size;    /**< size of the structure */
	unsigned int width;   /**< width of the captured image */
	unsigned int height;  /**< height of the captured image */
	unsigned int res;     /**< resolution of the captured image */
	unsigned int bpp;     /**< pixel depth of the captured image */
} DPFPDD_IMAGE_INFO;

/**
\brief Describes the result of the capture operation
*/
typedef struct dpfpdd_capture_result{
	unsigned int      size;    /**< size of the structure */
	int               success; /**< success flag; 0: capture failed, 1: capture succeeded, image is good */
	DPFPDD_QUALITY    quality; /**< image quality */
	unsigned int      score;   /**< image score */
	DPFPDD_IMAGE_INFO info;    /**< image info */
} DPFPDD_CAPTURE_RESULT;

/**
\brief Describes the result of asyncronos capture operation
*/
typedef struct dpfpdd_capture_callback_data_0{
	unsigned int          size;            /**< size of the structure */
	int                   error;           /**< error code */
	DPFPDD_CAPTURE_PARAM  capture_parm;    /**< capture parameters passed to dpfpdd_capture_async */
	DPFPDD_CAPTURE_RESULT capture_result;  /**< result of the capture operation */ 
	unsigned int          image_size;      /**< size of the image data */
	unsigned char*        image_data;      /**< image data */
} DPFPDD_CAPTURE_CALLBACK_DATA_0;

/**
\brief Callback for asyncronous capture
*/
typedef void (DPAPICALL *DPFPDD_CAPTURE_CALLBACK)(
	void*        callback_context,         /**< client context */
	unsigned int reserved,                 /**< currently reserved, always 0 */
	unsigned int callback_data_size,       /**< size of the callback data */
	void*        callback_data             /**< callback data (currently DPFPDD_CAPTURE_CALLBACK_DATA_0) */
);

/**
\brief LED identifiers
*/
typedef unsigned int DPFPDD_LED_ID;
#define DPFPDD_LED_MAIN           0x01 /**< main (illumination) LED */ 
#define DPFPDD_LED_REJECT         0x04 /**< red (reject) LED */
#define DPFPDD_LED_ACCEPT         0x08 /**< green (accept) LED */
#define DPFPDD_LED_FINGER_DETECT  0x10 /**< blue (finger detect) LED */
#define DPFPDD_LED_ALL            0xffffffff /**< all present LEDs */

/**
\brief LED operation mode
*/
typedef unsigned int DPFPDD_LED_MODE_TYPE;
#define DPFPDD_LED_AUTO   1 /**< automatic, default configuration */
#define DPFPDD_LED_CLIENT 2 /**< client application controls LED */

/**
\brief LED state commands
*/
typedef unsigned int DPFPDD_LED_CMD_TYPE;
#define DPFPDD_LED_CMD_OFF   0 /**< turn LED off */
#define DPFPDD_LED_CMD_ON    1 /**< turn LED on */

/**
\brief Reader and driver settings
*/
typedef unsigned int DPFPDD_PARMID;
#define DPFPDD_PARMID_ROTATE              0x100 /**< rotate image 180 degrees */
#define DPFPDD_PARMID_FINGERDETECT_ENABLE 0x104 /**< enable detection of fingers */
#define DPFPDD_PARMID_IOMAP               0x105 /**< I/O map settings */

/**
\brief I/O map setting parameters.
*/
typedef struct dpfpdd_iomap {
	unsigned short	addr;	 /**< I/O address or offset */
	unsigned short	len;	 /**< size size of the data buffer */
	unsigned char 	buff[1]; /**< data buffer */
} DPFPDD_IOMAP;

/****************************************************************************************************
 API calls
****************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	/**
	\brief Queries the library and API version information. 
	
	This is the only function which can be called before dpfpdd_init() or after dpfpdd_exit().
	
	\param ver  [in] Pointer to memory buffer; [out] Pointer to the version information (per DPFPDD_VERSION)
	\return DPFPDD_SUCCESS:   Version information was acquired;
	\return DPFPDD_E_FAILURE: Failed to acquire version information.
	*/
	int DPAPICALL dpfpdd_version(
		DPFPDD_VERSION* ver
	);

	/**
	\brief Library initialization.
	
	This function initializes the library. It must be called before calling any other functions from the library, except dpfpdd_version().
	
	\return DPFPDD_SUCCESS:   Library was initialized;
	\return DPFPDD_E_FAILURE: Failed to initialize library.
	*/
	int DPAPICALL dpfpdd_init(void);

	/**
	\brief Library release.
	
	This function releases the library. After calling this function the application can only call dpfpdd_version(), and  dpfpdd_init().
	
	\return DPFPDD_SUCCESS:   Library was released;
	\return DPFPDD_E_FAILURE: Failed to release library.
	*/
	int DPAPICALL dpfpdd_exit(void);


	/**
	\brief Returns information about connected readers. 
	
	Client application must allocate memory for the list of the available devices and pass number of entries in the dev_cnt parameter. 
	If memory is not sufficient to contain information about all connected readers, then DPFPDD_E_MORE_DATA will be returned. 
	The number of connected devices will be returned in dev_cnt parameter.
	
	\param dev_cnt    [in] Number of entries in the dev_infos memory block; [out] Number of devices detected
	\param dev_infos  [in] Memory block; [out] Information about connected readers (per DPFPDD_DEV_INFO)
	\return DPFPDD_SUCCESS:      Information about connected readers obtained; 
	\return DPFPDD_E_FAILURE:    Unexpected failure;
	\return DPFPDD_E_MORE_DATA:	 Insufficient memory in dev_infos memory block for all readers. No data was returned. The required number of entries is in the dev_cnt.
	*/
	int DPAPICALL dpfpdd_query_devices(
 		unsigned int*    dev_cnt,
		DPFPDD_DEV_INFO* dev_infos
	);

	/**
	\brief Opens a fingerprint reader in exclusive mode.

	If you or another process have already opened the reader, you cannot open it again.
	
	\param dev_name  Name of the reader, as acquired from dpfpdd_query_devices().
	\param pdev      [in] Pointer to empty handle (per DPFPDD_DEV); [out] Pointer to reader handle.
	\return DPFPDD_SUCCESS:             A valid reader handle is in the ppdev;
	\return DPFPDD_E_FAILURE:           Unexpected failure;
	\return DPFPDD_E_INVALID_PARAMETER: No reader with this name found;
	\return DPFPDD_E_DEVICE_BUSY:       Reader is already opened by the same or another process;
	\return DPFPDD_E_DEVICE_FAILURE:    Failed to open the reader.
	*/
	int DPAPICALL dpfpdd_open(
 		char*       dev_name,
 		DPFPDD_DEV* pdev
	);

	/**
	\brief Opens a fingerprint reader.

	On Windows, client can choose if to open reader exclusively or in cooperative mode. In cooperative mode the process which has window in focus 
	will receive captured image.
	On Linux and Windows CE functionality is identical to dpfpdd_open. Priority is ignored and reader is always opened in exclusive mode.

	\param dev_name  Name of the reader, as acquired from dpfpdd_query_devices().
	\param priority  Priority of the client.
	\param pdev      [in] Pointer to empty handle (per DPFPDD_DEV); [out] Pointer to reader handle.
	\return DPFPDD_SUCCESS:             A valid reader handle is in the ppdev;
	\return DPFPDD_E_FAILURE:           Unexpected failure;
	\return DPFPDD_E_INVALID_PARAMETER: No reader with this name found;
	\return DPFPDD_E_DEVICE_BUSY:       Reader is already opened by the same or another process;
	\return DPFPDD_E_DEVICE_FAILURE:    Failed to open the reader.
	*/
	int DPAPICALL dpfpdd_open_ext(
 		char*           dev_name,
		DPFPDD_PRIORITY priority,
 		DPFPDD_DEV*     pdev
	);


	/**
	\brief Releases the reader.
	
	\param dev  Reader handle, as obtained from dpfpdd_open()
	\return DPFPDD_SUCCESS:          Reader closed, handle released
	\return DPFPDD_E_FAILURE:        Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE: Invalid reader handle
	\return DPFPDD_E_DEVICE_BUSY:    Another operation is in progress
	\return DPFPDD_E_DEVICE_FAILURE: Failed to close the reader
	*/
	int DPAPICALL dpfpdd_close(
 		DPFPDD_DEV dev
	);

	/**
	\brief Returns status of the reader.
	
	\param dev         Reader handle, as obtained from dpfpdd_open()
	\param dev_status  [in] Pointer to empty status (per DPFPDD_DEV_STATUS); [out] Pointer to  status of the reader
	\return DPFPDD_SUCCESS:             Reader status obtained
	\return DPFPDD_E_FAILURE:           Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE:    Invalid reader handle
	\return DPFPDD_E_MORE_DATA:         Insufficient memory is allocated for the dev_status, the required size is in the dev_status.size
	*/
	int DPAPICALL dpfpdd_get_device_status(
 		DPFPDD_DEV         dev,
 		DPFPDD_DEV_STATUS* dev_status
	);

	/**
	\brief Queries hardware info and capabilities of the reader.
	
	Client application must allocate memory for the information about the reader. If the allocated memory is not sufficient to hold
	information about all resolutions, then DPFPDD_E_MORE_DATA will be returned. 
	The number of resolutions will be returned in the dev_caps.resolution_cnt field, and the required size of 
	the .dev_caps will be returned in the dev_caps.size field.
	
	\param dev       Reader handle, as obtained from dpfpdd_open();
	\param dev_caps  [in] Pointer empty info structure (per DPFPDD_DEV_CAPS); [out] Pointer to reader capabilities.
	\return DPFPDD_SUCCESS:             Reader capabilities obtained
	\return DPFPDD_E_FAILURE:           Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE:    Invalid reader handle
	\return DPFPDD_E_DEVICE_BUSY:       Another operation is in progress
	\return DPFPDD_E_MORE_DATA:         Insufficient memory is allocated for the dev_caps, the required size is in the dev_caps.size
	\return DPFPDD_E_DEVICE_FAILURE:    Failed to obtain capabilities, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_get_device_capabilities(
		DPFPDD_DEV       dev,
 		DPFPDD_DEV_CAPS* dev_caps
	);

	/**
	\brief Capture a fingerprint image.

	This function captures a fingerprint image from the opened reader device.  
	This function signals the device that a fingerprint is expected and waits until a fingerprint is received.
	This function blocks until an image is captured, capture fails or timeout is expired. This function cannot 
	be called in streaming mode. Client application must allocate memory for the image_data. If memory 
	is not sufficient for the image, then DPFPDD_E_MORE_DATA will be returned. The required size of the 
	image_data will be returned in image_size parameter.
	
	\param dev              Reader handle, as obtained from dpfpdd_open()
	\param capture_parm	    Defines data type and image format (per DPFPDD_CAPTURE_PARAM)
	\param timeout_cnt      Defines timeout in milliseconds; (unsigned int)(-1) means no timeout (function will block until a fingerprint is captured)
	\param capture_result   [in] Pointer to memory buffer; [out] Pointer to status of results (per DPFPDD_CAPTURE_RESULT)
	\param image_size       [in] Size of the allocated memory for the image_data; [out] Actual size needed for the image_data
	\param image_data       [in] Memory buffer; [out] Captured image
	\return DPFPDD_SUCCESS:             Image captured. Extended result is in capture_result
	\return DPFPDD_E_FAILURE:           Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE:    Invalid reader handle
	\return DPFPDD_E_DEVICE_BUSY:       Another operation is in progress
	\return DPFPDD_E_MORE_DATA:         Insufficient  memory is allocated for the image_data, the required size is in the image_size
	\return DPFPDD_E_INVALID_PARAMETER: Wrong data type or image format in the capture_parm
	\return DPFPDD_E_DEVICE_FAILURE:    Failed to start capture, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_capture(
 		DPFPDD_DEV             dev,
 		DPFPDD_CAPTURE_PARAM*  capture_parm,
		unsigned int           timeout_cnt,
		DPFPDD_CAPTURE_RESULT* capture_result,
		unsigned int*          image_size,
 		unsigned char*         image_data
	);

	/**
	\brief Capture a fingerprint image asynchronously.

	This function starts asynchronous capture on the opened reader device.  
	This function signals the device that a fingerprint is expected and then exits.
	
	\param dev             Reader handle, as obtained from dpfpdd_open()
	\param capture_parm	   Defines data type and image format (per DPFPDD_CAPTURE_PARAM)
	\param context         Client context, passed into the callback
	\param callback        Address of the callback function, to be called when image is ready
	\return DPFPDD_SUCCESS:             Image captured. Extended result is in capture_result
	\return DPFPDD_E_FAILURE:           Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE:    Invalid reader handle
	\return DPFPDD_E_DEVICE_BUSY:       Another operation is in progress
	\return DPFPDD_E_INVALID_PARAMETER: Wrong data type or image format in the capture_parm
	\return DPFPDD_E_DEVICE_FAILURE:    Failed to start capture, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_capture_async(
		DPFPDD_DEV              dev,
		DPFPDD_CAPTURE_PARAM*   capture_parm,
		void*                   context,
		DPFPDD_CAPTURE_CALLBACK callback
	);

	/**
	\brief Cancels pending capture.
	
	\param dev  Reader handle, as obtained from dpfpdd_open();
	\return DPFPDD_SUCCESS:          Capture canceled
	\return DPFPDD_E_FAILURE:        Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE: Invalid reader handle
	\return DPFPDD_E_DEVICE_FAILURE: Failed to cancel capture, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_cancel(
 		DPFPDD_DEV dev
	);

	/**
	\brief Puts reader into streaming mode.
	
	Not all readers support this mode. When the reader is in streaming mode, the application can only call 
	dpfpdd_get_stream_image() to acquire images from the stream.
	
	\param dev  Reader handle, as obtained from dpfpdd_open()
	\return DPFPDD_SUCCESS:          Reader put into streaming mode
	\return DPFPDD_E_FAILURE:        Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE: Invalid reader handle
	\return DPFPDD_E_DEVICE_BUSY:    Another operation is in progress
	\return DPFPDD_E_DEVICE_FAILURE: Failed to start streaming, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_start_stream(
 		DPFPDD_DEV dev
	);

	/**
	\brief Stops streaming mode.
	
	\param dev  Reader handle, obtained from dpfpdd_open()
	\return DPFPDD_SUCCESS:          Streaming was stopped
	\return DPFPDD_E_FAILURE:        Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE: Invalid reader handle
	\return DPFPDD_E_DEVICE_FAILURE: Failed to stop streaming, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_stop_stream(
 		DPFPDD_DEV dev
	);

	/**
	\brief Takes an image from the stream.
	
	After the reader is put into streaming mode this function takes an image from the stream. After this function returns, the  
	reader stays in the streaming mode. Frame selection, scoring or other image processing is not performed.
	
	The client application must allocate memory for the image_data. If the memory is not sufficient for the image, then 
	DPFPDD_E_MORE_DATA will be returned. The required size of the image_data will be returned in the image_size parameter.
	For every image from the stream, the driver provides a score (in capture_result.score) and quality feedback (in capture_result.quailty). 
	
	\param dev             Reader handle, obtained from dpfpdd_open()
	\param capture_parm	   Defines data type and image format (per DPFPDD_CAPTURE_PARAM)
	\param capture_result  Pointer to the structure to receive result of the capture (per DPFPDD_CAPTURE_RESULT)
	\param image_size      [in] Size of the allocated memory for the image_data; [out] Actual size needed for the image_data
	\param image_data      Receives captured image
	\return DPFPDD_SUCCESS:             Image acquired from the stream. Extended result is in capture_result
	\return DPFPDD_E_FAILURE:           Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE:    Invalid reader handle
	\return DPFPDD_E_DEVICE_BUSY:       Another operation is in progress
	\return DPFPDD_E_MORE_DATA:         Insufficient  memory is allocated for the image_data, the required size is in the image_size
	\return DPFPDD_E_INVALID_PARAMETER: Wrong data type or image format in the capture_parm
	\return DPFPDD_E_DEVICE_FAILURE:    Failed to acquire image from the stream, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_get_stream_image (
 		DPFPDD_DEV             dev,
 		DPFPDD_CAPTURE_PARAM*  capture_parm,
		DPFPDD_CAPTURE_RESULT* capture_result,
		unsigned int*          image_size,
 		unsigned char*         image_data
	);

	/**
	\brief Resets the reader.
	
	This function performs a hardware reset on the reader.  Hardware resets are typically needed only 
	after a hardware problem (e.g., the reader is unplugged or receives an electrostatic shock). 
	This function blocks until the reset is complete.

	\param dev  Reader handle, as obtained from dpfpdd_open();
	\return DPFPDD_SUCCESS:          Reset succeeded;
	\return DPFPDD_E_FAILURE:        Unexpected failure;
	\return DPFPDD_E_INVALID_DEVICE: Invalid reader handle;
	\return DPFPDD_E_DEVICE_BUSY:    Another operation is in progress;
	\return DPFPDD_E_DEVICE_FAILURE: Failed to reset, reader is not functioning properly.
	*/
	int DPAPICALL dpfpdd_reset(
 		DPFPDD_DEV dev
	);

	/**
	\brief Calibrates the reader.

	This function calibrates a reader and blocks until the calibration is complete.  It can take several seconds to calibrate for some devices.
	
	\param dev  Reader handle, as obtained from dpfpdd_open();
	\return DPFPDD_SUCCESS:          Calibration succeeded
	\return DPFPDD_E_FAILURE:        Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE: Invalid reader handle
	\return DPFPDD_E_DEVICE_BUSY:    Another operation is in progress
	\return DPFPDD_E_DEVICE_FAILURE: Failed to calibrate, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_calibrate(
 		DPFPDD_DEV dev
	);

	/**
	\brief Sets configuration parameters for LED.

	Function sets operation mode for LED: automatic or controlled by client application.

	\param dev         Reader handle, as obtained from dpfpdd_open().
	\param led_id      LED type.
	\param led_mode    LED operation mode.
	\param reserved    Reserved for future use, must be NULL.
	*/
	int DPAPICALL dpfpdd_led_config(
		   DPFPDD_DEV               dev,
		   DPFPDD_LED_ID            led_id,
		   DPFPDD_LED_MODE_TYPE     led_mode,
		   void*                    reserved
	);

	/**
	\brief Turns LED on/off or starts LED event

	If LED is controlled by client application this function allows to turn LED on or off.
	LED must be configured by calling dpfpdd_led_config().

	\param dev      Reader handle, as obtained from dpfpdd_open().
	\param led_id   LED type.
	\param led_cmd  LED command.
	*/
	int DPAPICALL dpfpdd_led_ctrl(
		   DPFPDD_DEV            dev,
		   DPFPDD_LED_ID         led_id,
		   DPFPDD_LED_CMD_TYPE   led_cmd
	);

	/**
	\brief Changes reader or driver setting.
	
	\param dev      Reader handle, as obtained from dpfpdd_open();
	\param parm_id  Parameter ID;
	\param size     Size of the parameter buffer;
	\param buffer   Parameter buffer;
	\return DPFPDD_SUCCESS:             Parameter was set
	\return DPFPDD_E_FAILURE:           Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE:    Invalid reader handle
	\return DPFPDD_E_DEVICE_BUSY:       Another operation is in progress
	\return DPFPDD_E_INVALID_PARAMETER: Parameter ID is incorrect or not supported
	\return DPFPDD_E_DEVICE_FAILURE:    Failed to set parameter, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_set_parameter(
 		DPFPDD_DEV     dev,
 		DPFPDD_PARMID  parm_id,
 		unsigned int   size,
 		unsigned char* buffer
	);

	/** \brief Reads reader or driver setting.
	 
	\param dev      Reader handle, obtained from dpfpdd_open();
	\param parm_id  Parameter ID;
	\param size     Size of the parameter buffer;
	\param buffer   Parameter buffer;
	\return DPFPDD_SUCCESS:             Parameter was set
	\return DPFPDD_E_FAILURE:           Unexpected failure
	\return DPFPDD_E_INVALID_DEVICE:    Invalid reader handle
	\return DPFPDD_E_DEVICE_BUSY:       Another operation is in progress
	\return DPFPDD_E_INVALID_PARAMETER: Parameter ID is incorrect or not supported
	\return DPFPDD_E_DEVICE_FAILURE:    Failed to set parameter, reader is not functioning properly
	*/
	int DPAPICALL dpfpdd_get_parameter(
 		DPFPDD_DEV     dev,
 		DPFPDD_PARMID  parm_id,
 		unsigned int   size,
 		unsigned char* buffer
	);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _DPFPDD_API_H_ */
