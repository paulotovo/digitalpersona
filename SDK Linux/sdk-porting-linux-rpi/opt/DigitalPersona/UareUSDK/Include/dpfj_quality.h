/**
\file dpfj_quality.h

\copyright (c) 2012 DigitalPersona, Inc

\brief U.are.U SDK Image Quality API

Data types and functions for compression.

\version 2.0.0
*/

#ifndef __DPFJ_QUALITY_H__
#define __DPFJ_QUALITY_H__

#include <dpfj.h>

/**
\brief Image is invalid or absent.
*/
#define DPFJ_E_QUALITY_NO_IMAGE            DPERROR(0x15e)

/**
\brief Too few minutia detected in the fingerprint image.
*/
#define DPFJ_E_QUALITY_TOO_FEW_MINUTIA     DPERROR(0x15f)

/**
\brief Unspecified error during execution.
*/ 
#define DPFJ_E_QUALITY_FAILURE             DPERROR(0x160)

/**
\brief Library for image quality is not found or not built-in.
*/
#define DPFJ_E_QUALITY_LIB_NOT_FOUND       DPERROR(0x161)


typedef int DPFJ_QUALITY_ALGORITHM;

#define DPFJ_QUALITY_NFIQ_NIST    1 /**< NFIQ, NIST algorithm */
#define DPFJ_QUALITY_NFIQ_AWARE   2 /**< NFIQ, Aware SDK */

/****************************************************************************************************
 API calls
****************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	/**
	\brief Calculates NFIQ score of the image in raw format.

	This function attempts to calculate NFIQ score of raw image using Aware WSQ library (WSQ1000) or NIST algorithm. 

	\param image_data    pointer to the image data
	\param image_size    size of the image data
	\param image_width   width of the image
	\param image_height  height of the image
	\param image_dpi     resolution of the image
	\param image_bpp     pixel depth of the image. Only 8bpp images are currently processed.
	\param quality_alg   NFIQ algorithm to use, currently Aware WSQ and NIST algorithms are supported.
	\param nfiq_score    pointer to the variable to receive NFIQ score.
	\return DPFJ_SUCCESS:                      NFIQ score calculated.
	\return DPFJ_E_QUALITY_INVALID_PARAMETER:  One or more parameters passed are invalid, or one or more parameters set for the operation are invalid.
	\return DPFJ_E_QUALITY_TOO_FEW_MINUTIA:    Too few minutia detected in the fingerprint image.
	\return DPFJ_E_QUALITY_LIB_NOT_FOUND:      Aware WSQ library is not found, or NIST WSQ algorithm is not built-in.
	\return DPFJ_E_QUALITY_FAILURE:            Unknown error.
	 */
	int DPAPICALL dpfj_quality_nfiq_from_raw(
		const unsigned char*    image_data,
		unsigned int            image_size,
	 	unsigned int            image_width,
	 	unsigned int            image_height,
		unsigned int            image_dpi,
		unsigned int            image_bpp,
		DPFJ_QUALITY_ALGORITHM  quality_alg,
		unsigned int*           nfiq_score
	);

	/**
	\brief Calculates NFIQ score of the view in the FID.

	This function attempts to calculate NFIQ score of single view in the FID using Aware WSQ library (WSQ1000) or NIST algorithm. 

	\param fid_type      type of the FID.
	\param fid           pointer to the FID data.
	\param fid_size      size of the FID data.
	\param view_idx      index of the view.
	\param quality_alg   NFIQ algorithm to use, currently Aware WSQ and NIST algorithms are supported.
	\param nfiq_score    pointer to the variable to receive NFIQ score.
	\return DPFJ_SUCCESS:                      NFIQ score calculated.
	\return DPFJ_E_QUALITY_INVALID_PARAMETER:  One or more parameters passed are invalid, or one or more parameters set for the operation are invalid.
	\return DPFJ_E_QUALITY_TOO_FEW_MINUTIA:    Too few minutia detected in the fingerprint image.
	\return DPFJ_E_QUALITY_LIB_NOT_FOUND:      Aware WSQ library is not found, or NIST WSQ algorithm is not built-in.
	\return DPFJ_E_QUALITY_FAILURE:            Unknown error.
	 */
	int DPAPICALL dpfj_quality_nfiq_from_fid(
		DPFJ_FID_FORMAT         fid_type,
 		const unsigned char*    fid,
		unsigned int            fid_size,
		unsigned int            view_idx,
		DPFJ_QUALITY_ALGORITHM  quality_alg,
		unsigned int*           nfiq_score
	);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DPFJ_QUALITY_H__ */

