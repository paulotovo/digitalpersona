/**
\file dpfj_compression.h

\copyright (c) 2012 DigitalPersona, Inc

\brief U.are.U SDK Image Compression API

Data types and functions for compression.

\version 2.0.0
*/

#ifndef __DPFJ_COMPRESSION_H__
#define __DPFJ_COMPRESSION_H__

#include <dpfj.h>

/**
\brief Compression or decompression operation is in progress.
*/
#define DPFJ_E_COMPRESSION_IN_PROGRESS            DPERROR(0x141)

/**
\brief Compression or decompression operation was not started.
*/
#define DPFJ_E_COMPRESSION_NOT_STARTED            DPERROR(0x142)

/**
\brief One or more parameters passed for WSQ compression are invalid.
*/
#define DPFJ_E_COMPRESSION_INVALID_WSQ_PARAMETER  DPERROR(0x14a)

/**
\brief Unspecified error during WSQ compression or decompression.
*/
#define DPFJ_E_COMPRESSION_WSQ_FAILURE            DPERROR(0x14b)

/**
\brief Library for WSQ compression is not found or not built-in.
*/
#define DPFJ_E_COMPRESSION_WSQ_LIB_NOT_FOUND      DPERROR(0x14c)


typedef int DPFJ_COMPRESSION_ALGORITHM;

#define DPFJ_COMPRESSION_WSQ_NIST    1 /**< WSQ compression, NIST algorithm */
#define DPFJ_COMPRESSION_WSQ_AWARE   2 /**< WSQ compression, Aware SDK */

/****************************************************************************************************
 API calls
****************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	/**
	\brief Starts compression or decompression operation.

	\return DPFJ_SUCCESS:                   Operation started.
	\return DPFJ_E_COMPRESSION_IN_PROGRESS: Another operation is in progress.
	\return DPFJ_E_FAILURE:                 Unknown error.
	 */
	int DPAPICALL dpfj_start_compression();

	/**
	\brief Sets target bitrate and tolerance for WSQ compression operation.

	\param bitrate_x100   Requested bitrate multiplied by 100. For example, to request bitrate of 4.19, pass 419.
	\param tolerance_aw   Sets tolerance in percents for the Aware WSQ compression. Range is 1 to 100. For NIST algorithm this parameter will be ignored.
	\return DPFJ_SUCCESS:                             Bitrate and tolerance are set.
	\return DPFJ_E_COMPRESSION_NOT_STARTED:           Operation is not started.
	\return DPFJ_E_COMPRESSION_INVALID_WSQ_PARAMETER: One or more parameters passed are invalid.
	\return DPFJ_E_FAILURE:                           Unknown error.
	 */
	int DPAPICALL dpfj_set_wsq_bitrate(
		unsigned int bitrate_x100,
		unsigned int tolerance_aw /**< when using Aware WSQ compression */
	);

	/**
	\brief Sets target size and tolerance for WSQ compression operation.

	\param size           Requested size of compressed image.
	\param tolerance_aw   Sets tolerance in percents for the Aware WSQ compression. Range is 1 to 100. For NIST algorithm this parameter will be ignored.
	\return DPFJ_SUCCESS:                             Size and tolerance are set.
	\return DPFJ_E_COMPRESSION_NOT_STARTED:           Operation is not started.
	\return DPFJ_E_COMPRESSION_INVALID_WSQ_PARAMETER: One or more parameters passed are invalid.
	\return DPFJ_E_FAILURE:                           Unknown error.
	 */
	int DPAPICALL dpfj_set_wsq_size(
		unsigned int size,
		unsigned int tolerance_aw /**< when using Aware WSQ compression */
	);

	/**
	\brief Compresses FID.

	This function attempts to compress FID using Aware WSQ library (WSQ1000) or NIST algorithm. It's necessary to set target bitrate or
	target size before calling dpfj_compress_fid(). The function will allocate memory for the compressed FID. The resulting size of the
	compressed FID can be acquired by calling dpfj_get_processed_size(). The resulting size likely will differ from the requested size, or bitrate.
	How much the difference will be is dictated by the image, requested size or bitrate and tolerance (only for Aware algorithm).

	\param fid_type          type of the FID.
	\param fid               pointer to the FID data.
	\param fid_size          size of the FID data.
	\param compression_alg   compression algorithm to use, currently Aware WSQ and NIST WSQ algorithms are supported.
	\return DPFJ_SUCCESS:                             FID compressed.
	\return DPFJ_E_COMPRESSION_NOT_STARTED:           Operation is not started.
	\return DPFJ_E_COMPRESSION_INVALID_WSQ_PARAMETER: One or more parameters passed are invalid, or one or more parameters set for the operation are invalid.
	\return DPFJ_E_COMPRESSION_WSQ_LIB_NOT_FOUND:     Aware WSQ library is not found, or NIST WSQ algorithm is not built-in.
	\return DPFJ_E_COMPRESSION_WSQ_FAILURE:           Unknown error.
	 */
	int DPAPICALL dpfj_compress_fid(
		DPFJ_FID_FORMAT            fid_type,
 		const unsigned char*       fid,
		unsigned int               fid_size,
		DPFJ_COMPRESSION_ALGORITHM compression_alg
	);

	/**
	\brief Compresses raw image.

	This function attempts to compress image using Aware WSQ library (WSQ1000) or NIST algorithm. It's necessary to set target bitrate or
	target size before calling dpfj_compress_raw(). The function will allocate memory for the compressed image. The resulting size of the
	compressed image can be acquired by calling dpfj_get_processed_size(). The resulting size likely will differ from the requested size, or bitrate.
	How much the difference will be is dictated by the image, requested size or bitrate and tolerance (only for Aware algorithm).

	\param image_data        pointer to the image data
	\param image_size        size of the image data
	\param image_width       width of the image
	\param image_height      height of the image
	\param image_dpi         resolution of the image
	\param image_bpp         pixel depth of the image. Only 8bpp images are currently processed.
	\param compression_alg   compression algorithm to use, currently Aware WSQ and NIST WSQ algorithms are supported.
	\return DPFJ_SUCCESS:                             Image compressed.
	\return DPFJ_E_COMPRESSION_NOT_STARTED:           Operation is not started.
	\return DPFJ_E_COMPRESSION_INVALID_WSQ_PARAMETER: One or more parameters passed are invalid, or one or more parameters set for the operation are invalid.
	\return DPFJ_E_COMPRESSION_WSQ_LIB_NOT_FOUND:     Aware WSQ library is not found, or NIST WSQ algorithm is not built-in.
	\return DPFJ_E_COMPRESSION_WSQ_FAILURE:           Unknown error.
	 */
	int DPAPICALL dpfj_compress_raw(
		const unsigned char*       image_data,
		unsigned int               image_size,
	 	unsigned int               image_width,
	 	unsigned int               image_height,
		unsigned int               image_dpi,
		unsigned int               image_bpp,
		DPFJ_COMPRESSION_ALGORITHM compression_alg
	);

	/**
	\brief Expands compressed FID.

	This function attempts to expand FID using Aware WSQ library (WSQ1000) or NIST algorithm. The function will allocate memory for the
	expanded FID. The resulting size of the expanded FID can be acquired by calling dpfj_get_processed_size().

	\param fid_type          type of the compressed FID.
	\param fid               pointer to the FID data.
	\param fid_size          size of the FID data.
	\param compression_alg   compression algorithm to use, currently Aware WSQ and NIST WSQ algorithms are supported.
	\return DPFJ_SUCCESS:                             FID expanded.
	\return DPFJ_E_COMPRESSION_NOT_STARTED:           Operation is not started.
	\return DPFJ_E_COMPRESSION_INVALID_WSQ_PARAMETER: One or more parameters passed are invalid.
	\return DPFJ_E_COMPRESSION_WSQ_LIB_NOT_FOUND:     Aware WSQ library is not found, or NIST WSQ algorithm is not built-in.
	\return DPFJ_E_COMPRESSION_WSQ_FAILURE:           Unknown error.
	 */
	int DPAPICALL dpfj_expand_fid(
		DPFJ_FID_FORMAT            fid_type,
 		const unsigned char*       fid,
		unsigned int               fid_size,
		DPFJ_COMPRESSION_ALGORITHM compression_alg
	);

	/**
	\brief Expands compressed raw image.

	This function attempts to expand FID using Aware WSQ library (WSQ1000) or NIST algorithm. The function will allocate memory for the
	expanded FID. The resulting size of the expanded FID can be acquired by calling dpfj_get_processed_size().

	\param image_data        pointer to the compressed image data
	\param image_size        size of the image data
	\param compression_alg   compression algorithm to use, currently Aware WSQ and NIST WSQ algorithms are supported.
	\param image_width       pointer to receive width of the image
	\param image_height      pointer to receive height of the image
	\param image_dpi         pointer to receive resolution of the image
	\param image_bpp         pointer to receive pixel depth of the image. Only 8bpp images are currently processed.
	\return DPFJ_SUCCESS:                             Image expanded.
	\return DPFJ_E_COMPRESSION_NOT_STARTED:           Operation is not started.
	\return DPFJ_E_COMPRESSION_INVALID_WSQ_PARAMETER: One or more parameters passed are invalid.
	\return DPFJ_E_COMPRESSION_WSQ_LIB_NOT_FOUND:     Aware WSQ library is not found, or NIST WSQ algorithm is not built-in.
	\return DPFJ_E_COMPRESSION_WSQ_FAILURE:           Unknown error.
	 */
	int DPAPICALL dpfj_expand_raw(
		const unsigned char*       image_data,
		unsigned int               image_size,
		DPFJ_COMPRESSION_ALGORITHM compression_alg,
		unsigned int*              image_width,
		unsigned int*              image_height,
		unsigned int*              image_dpi,
		unsigned int*              image_bpp
	);

	/**
	\brief Returns the processed data.

	Application must determine the size of the data by calling dpfj_get_processed_data() and allocate memory to receive processed data.

	\param image_data        pointer to the allocated memory for the processed data
	\param image_size        size of the allocated memory
	\return DPFJ_SUCCESS:                             Processed data copied.
	\return DPFJ_E_COMPRESSION_NOT_STARTED:           Operation is not started.
	\return DPFJ_E_MORE_DATA:                         Allocated memory is not sufficient. The required memory size is in the image_size.
	\return DPFJ_E_COMPRESSION_INVALID_WSQ_PARAMETER: One or more parameters passed are invalid.
	\return DPFJ_E_COMPRESSION_WSQ_FAILURE:           Unknown error.
	 */
	int DPAPICALL dpfj_get_processed_data(
		unsigned char* image_data,
		unsigned int*  image_size
	);

	/**
	\brief Ends compression or decompression operation, releases memory.

	This function releases resources used during the enrollment process.  Call after enrollment is complete.
	\return DPFJ_SUCCESS:   Operation ended.
	\return DPFJ_E_FAILURE: Unknown error.
	*/
	int DPAPICALL dpfj_finish_compression();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DPFJ_COMPRESSION_H__ */

