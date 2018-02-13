/*
 *
 * Copyright 2012 Samsung Electronics S.LSI Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * @file        csc.c
 *
 * @brief       color space convertion abstract source
 *
 * @author      Pyoungjae Jung(pjet.jung@samsung.com)
 *
 * @version     1.0.0
 *
 * @history
 *   2012.1.11 : Create
 */
#define LOG_TAG "libcsc"
#include <cutils/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/Log.h>
#include <system/graphics.h>

#include "csc.h"
#include "exynos_format.h"
#include "swconverter.h"

#ifdef ENABLE_FIMC
#include "exynos_fimc.h"
#endif

#ifdef ENABLE_GSCALER
#include "exynos_gscaler.h"
#ifdef ENABLE_SCALER
#include "exynos_scaler.h"
#endif
#endif

#define GSCALER_IMG_ALIGN 16
#define FIMC_IMG_ALIGN_WIDTH 16
#define FIMC_IMG_ALIGN_HEIGHT 2
#define MFC_IMG_ALIGN_WIDTH 16

static CSC_ERRORCODE copy_mfc_data(CSC_HANDLE *handle) {
    CSC_ERRORCODE ret = CSC_ErrorNone;

    int i;
    char *pSrc = NULL;
    char *pDst = NULL;

    switch (handle->src_format.color_format) {
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P_M:
    case HAL_PIXEL_FORMAT_EXYNOS_YV12_M:
        pSrc = (char *)handle->src_buffer.planes[CSC_Y_PLANE];
        pDst = (char *)handle->dst_buffer.planes[CSC_Y_PLANE];
        for (i = 0; i < (int)handle->src_format.crop_height; i++) {
            memcpy(pDst + (handle->src_format.crop_width * i),
                   pSrc + (handle->src_format.width * i),
                   handle->src_format.crop_width);
        }

        pSrc = (char *)handle->src_buffer.planes[CSC_U_PLANE];
        pDst = (char *)handle->dst_buffer.planes[CSC_U_PLANE];
        for (i = 0; i < (int)(handle->src_format.crop_height >> 1); i++) {
            memcpy(pDst + ((handle->src_format.crop_width >> 1) * i),
                   pSrc + (ALIGN((handle->src_format.crop_width >> 1), MFC_IMG_ALIGN_WIDTH) * i),
                   (handle->src_format.crop_width >> 1));
        }

        pSrc = (char *)handle->src_buffer.planes[CSC_V_PLANE];
        pDst = (char *)handle->dst_buffer.planes[CSC_V_PLANE];
        for (i = 0; i < (int)(handle->src_format.crop_height >> 1); i++) {
            memcpy(pDst + ((handle->src_format.crop_width >> 1) * i),
                   pSrc + (ALIGN((handle->src_format.crop_width >> 1), MFC_IMG_ALIGN_WIDTH) * i),
                   (handle->src_format.crop_width >> 1));
        }
        break;
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP_M:
    case HAL_PIXEL_FORMAT_EXYNOS_YCrCb_420_SP_M:
        pSrc = (char *)handle->src_buffer.planes[CSC_Y_PLANE];
        pDst = (char *)handle->dst_buffer.planes[CSC_Y_PLANE];
        for (i = 0; i < (int)handle->src_format.crop_height; i++) {
            memcpy(pDst + (handle->src_format.crop_width * i),
                   pSrc + (handle->src_format.width * i),
                   handle->src_format.crop_width);
        }

        pSrc = (char *)handle->src_buffer.planes[CSC_UV_PLANE];
        pDst = (char *)handle->dst_buffer.planes[CSC_UV_PLANE];
        for (i = 0; i < (int)(handle->src_format.crop_height >> 1); i++) {
            memcpy(pDst + (handle->src_format.crop_width * i),
                   pSrc + (handle->src_format.width * i),
                   handle->src_format.crop_width);
        }
        break;
    default:
        ret = CSC_ErrorUnsupportFormat;
        break;
    }

    return ret;
}

/* source is RGB888 */
static CSC_ERRORCODE conv_sw_src_argb888(
    CSC_HANDLE *handle)
{
    CSC_ERRORCODE ret = CSC_ErrorNone;

    switch (handle->dst_format.color_format) {
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P_M:
        csc_ARGB8888_to_YUV420P(
            (unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_U_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_V_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_RGB_PLANE],
            handle->src_format.width,
            handle->src_format.height);
        ret = CSC_ErrorNone;
        break;
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP_M:
#ifdef __aarch64__
        csc_ARGB8888_to_YUV420SP(
#else
        csc_ARGB8888_to_YUV420SP_NEON(
#endif
            (unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_UV_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_RGB_PLANE],
            handle->src_format.width,
            handle->src_format.height);
        ret = CSC_ErrorNone;
        break;
    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_EXYNOS_YV12_M:
        csc_ARGB8888_to_YUV420P(
            (unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_V_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_U_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_RGB_PLANE],
            handle->src_format.width,
            handle->src_format.height);
        ret = CSC_ErrorNone;
        break;
    default:
        ret = CSC_ErrorUnsupportFormat;
        break;
    }

    return ret;
}

/* source is NV12T */
static CSC_ERRORCODE conv_sw_src_nv12t(
    CSC_HANDLE *handle)
{
    CSC_ERRORCODE ret = CSC_ErrorNone;

    switch (handle->dst_format.color_format) {
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P_M:
#ifdef __aarch64__
        csc_tiled_to_linear_y(
#else
        csc_tiled_to_linear_y_neon(
#endif
            (unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
            handle->src_format.width,
            handle->src_format.height);
#ifdef __aarch64__
        csc_tiled_to_linear_uv_deinterleave(
#else
        csc_tiled_to_linear_uv_deinterleave_neon(
#endif
            (unsigned char *)handle->dst_buffer.planes[CSC_U_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_V_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_UV_PLANE],
            handle->src_format.width,
            handle->src_format.height / 2);
        ret = CSC_ErrorNone;
        break;
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP_M:
#ifdef __aarch64__
        csc_tiled_to_linear_y(
#else
        csc_tiled_to_linear_y_neon(
#endif
            (unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
            handle->src_format.width,
            handle->src_format.height);
#ifdef __aarch64__
        csc_tiled_to_linear_uv(
#else
        csc_tiled_to_linear_uv_neon(
#endif
            (unsigned char *)handle->dst_buffer.planes[CSC_UV_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_UV_PLANE],
            handle->src_format.width,
            handle->src_format.height / 2);
        ret = CSC_ErrorNone;
        break;
    default:
        ret = CSC_ErrorUnsupportFormat;
        break;
    }

    return ret;
}

/* source is YUV420P */
static CSC_ERRORCODE conv_sw_src_yuv420p(
    CSC_HANDLE *handle)
{
    CSC_ERRORCODE ret = CSC_ErrorNone;

    switch (handle->dst_format.color_format) {
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P:    /* bypass */
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P_M:
        if (handle->src_buffer.mem_type == CSC_MEMORY_MFC) {
            ret = copy_mfc_data(handle);
        } else {
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
                   handle->src_format.width * handle->src_format.height);
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_U_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_U_PLANE],
                   (handle->src_format.width * handle->src_format.height) >> 2);
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_V_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_V_PLANE],
                   (handle->src_format.width * handle->src_format.height) >> 2);
            ret = CSC_ErrorNone;
        }
        break;
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP_M:
        memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
               (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
               handle->src_format.width * handle->src_format.height);
#ifdef __aarch64__
        csc_interleave_memcpy(
#else
        csc_interleave_memcpy_neon(
#endif
            (unsigned char *)handle->dst_buffer.planes[CSC_UV_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_U_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_V_PLANE],
            (handle->src_format.width * handle->src_format.height) >> 2);
        ret = CSC_ErrorNone;
        break;
    default:
        ret = CSC_ErrorUnsupportFormat;
        break;
    }

    return ret;
}

/* source is YVU420P */
static CSC_ERRORCODE conv_sw_src_yvu420p(
    CSC_HANDLE *handle)
{
    CSC_ERRORCODE ret = CSC_ErrorNone;

    switch (handle->dst_format.color_format) {
    case HAL_PIXEL_FORMAT_YV12:  /* bypass */
    case HAL_PIXEL_FORMAT_EXYNOS_YV12_M:
        if (handle->src_buffer.mem_type == CSC_MEMORY_MFC) {
            ret = copy_mfc_data(handle);
        } else {
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
                   handle->src_format.width * handle->src_format.height);
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_U_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_U_PLANE],
                   (handle->src_format.width * handle->src_format.height) >> 2);
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_V_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_V_PLANE],
                   (handle->src_format.width * handle->src_format.height) >> 2);
            ret = CSC_ErrorNone;
        }
        break;
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP_M:
        memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
               (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
               handle->src_format.width * handle->src_format.height);
#ifdef __aarch64__
        csc_interleave_memcpy(
#else
        csc_interleave_memcpy_neon(
#endif
            (unsigned char *)handle->dst_buffer.planes[CSC_UV_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_V_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_U_PLANE],
            (handle->src_format.width * handle->src_format.height) >> 2);
        ret = CSC_ErrorNone;
        break;
    default:
        ret = CSC_ErrorUnsupportFormat;
        break;
    }

    return ret;
}

/* source is YUV420SP */
static CSC_ERRORCODE conv_sw_src_yuv420sp(
    CSC_HANDLE *handle)
{
    CSC_ERRORCODE ret = CSC_ErrorNone;

    switch (handle->dst_format.color_format) {
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP:    /* bypass */
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP_M:
        if (handle->src_buffer.mem_type == CSC_MEMORY_MFC) {
            ret = copy_mfc_data(handle);
        } else {
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
                   handle->src_format.width * handle->src_format.height);
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_UV_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_UV_PLANE],
                   handle->src_format.width * handle->src_format.height >> 1);
            ret = CSC_ErrorNone;
        }
        break;
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P_M:
        memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
               (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
               handle->src_format.width * handle->src_format.height);
        csc_deinterleave_memcpy(
            (unsigned char *)handle->dst_buffer.planes[CSC_U_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_V_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_UV_PLANE],
            handle->src_format.width * handle->src_format.height >> 1);
        ret = CSC_ErrorNone;
        break;
    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_EXYNOS_YV12_M:
        memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
               (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
               handle->src_format.width * handle->src_format.height);
        csc_deinterleave_memcpy(
            (unsigned char *)handle->dst_buffer.planes[CSC_V_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_U_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_UV_PLANE],
            handle->src_format.width * handle->src_format.height >> 1);
        ret = CSC_ErrorNone;
        break;
    default:
        ret = CSC_ErrorUnsupportFormat;
        break;
    }

    return ret;
}

/* source is YVU420SP */
static CSC_ERRORCODE conv_sw_src_yvu420sp(
    CSC_HANDLE *handle)
{
    CSC_ERRORCODE ret = CSC_ErrorNone;

    switch (handle->dst_format.color_format) {
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:  /* bypass */
    case HAL_PIXEL_FORMAT_EXYNOS_YCrCb_420_SP_M:
        if (handle->src_buffer.mem_type == CSC_MEMORY_MFC) {
            ret = copy_mfc_data(handle);
        } else {
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
                   handle->src_format.width * handle->src_format.height);
            memcpy((unsigned char *)handle->dst_buffer.planes[CSC_UV_PLANE],
                   (unsigned char *)handle->src_buffer.planes[CSC_UV_PLANE],
                   handle->src_format.width * handle->src_format.height >> 1);
            ret = CSC_ErrorNone;
        }
        break;
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P_M:
        memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
               (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
               handle->src_format.width * handle->src_format.height);
        csc_deinterleave_memcpy(
            (unsigned char *)handle->dst_buffer.planes[CSC_V_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_U_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_UV_PLANE],
            handle->src_format.width * handle->src_format.height >> 1);
        ret = CSC_ErrorNone;
        break;
    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_EXYNOS_YV12_M:
        memcpy((unsigned char *)handle->dst_buffer.planes[CSC_Y_PLANE],
               (unsigned char *)handle->src_buffer.planes[CSC_Y_PLANE],
               handle->src_format.width * handle->src_format.height);
        csc_deinterleave_memcpy(
            (unsigned char *)handle->dst_buffer.planes[CSC_U_PLANE],
            (unsigned char *)handle->dst_buffer.planes[CSC_V_PLANE],
            (unsigned char *)handle->src_buffer.planes[CSC_UV_PLANE],
            handle->src_format.width * handle->src_format.height >> 1);
        ret = CSC_ErrorNone;
        break;
    default:
        ret = CSC_ErrorUnsupportFormat;
        break;
    }

    return ret;
}

static CSC_ERRORCODE conv_sw(
    CSC_HANDLE *handle)
{
    CSC_ERRORCODE ret = CSC_ErrorNone;

    switch (handle->src_format.color_format) {
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP_M_TILED:
        ret = conv_sw_src_nv12t(handle);
        break;
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_P_M:
        ret = conv_sw_src_yuv420p(handle);
        break;
    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_EXYNOS_YV12_M:
        ret = conv_sw_src_yvu420p(handle);
        break;
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP:
    case HAL_PIXEL_FORMAT_EXYNOS_YCbCr_420_SP_M:
        ret = conv_sw_src_yuv420sp(handle);
        break;
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
    case HAL_PIXEL_FORMAT_EXYNOS_YCrCb_420_SP_M:
        ret = conv_sw_src_yvu420sp(handle);
        break;
    case HAL_PIXEL_FORMAT_BGRA_8888:
        ret = conv_sw_src_argb888(handle);
        break;
    default:
        ret = CSC_ErrorUnsupportFormat;
        break;
    }

    return ret;
}

static CSC_ERRORCODE conv_hw(
    CSC_HANDLE *handle)
{
    CSC_ERRORCODE ret = CSC_ErrorNone;
    switch (handle->csc_hw_type) {
#ifdef ENABLE_FIMC
    case CSC_HW_TYPE_FIMC:
        if (exynos_fimc_convert(handle->csc_hw_handle) != 0) {
            ALOGE("%s:: exynos_fimc_convert() fail", __func__);
            ret = CSC_Error;
        }
        break;
#endif
#ifdef ENABLE_GSCALER
    case CSC_HW_TYPE_GSCALER:
        if (handle->hw_property.fixed_node < CSC_HW_SC0) {
            if (exynos_gsc_convert(handle->csc_hw_handle) != 0) {
                ALOGE("%s:: exynos_gsc_convert() fail", __func__);
                ret = CSC_Error;
            }
#ifdef ENABLE_SCALER
        } else {
            if (exynos_sc_convert(handle->csc_hw_handle) != 0) {
                ALOGE("%s:: exynos_sc_convert() fail", __func__);
                ret = CSC_Error;
            }
#endif
        }
        break;
#endif
    default:
        ALOGE("%s:: unsupported csc_hw_type(%d)", __func__, handle->csc_hw_type);
        ret = CSC_ErrorNotImplemented;
        break;
    }

    return ret;
}

static CSC_ERRORCODE csc_init_hw(
    void *handle)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    csc_handle = (CSC_HANDLE *)handle;
    if (csc_handle->csc_method == CSC_METHOD_HW) {
#ifdef ENABLE_FIMC
        csc_handle->csc_hw_type = CSC_HW_TYPE_FIMC;
#endif
#ifdef ENABLE_GSCALER
        csc_handle->csc_hw_type = CSC_HW_TYPE_GSCALER;
#endif
        switch (csc_handle->csc_hw_type) {
#ifdef ENABLE_FIMC
        case CSC_HW_TYPE_FIMC:
            if (csc_handle->hw_property.fixed_node >= 0)
                csc_handle->csc_hw_handle = exynos_fimc_create_exclusive(csc_handle->hw_property.fixed_node, FIMC_M2M_MODE, 0, 0);
            else
            csc_handle->csc_hw_handle = exynos_fimc_create();
            ALOGV("%s:: CSC_HW_TYPE_FIMC", __func__);
            break;
#endif
#ifdef ENABLE_GSCALER
        case CSC_HW_TYPE_GSCALER:
            if (csc_handle->hw_property.fixed_node >= 0) {
                if (csc_handle->hw_property.fixed_node < CSC_HW_SC0)
                    csc_handle->csc_hw_handle = exynos_gsc_create_exclusive(csc_handle->hw_property.fixed_node, GSC_M2M_MODE, 0, 0);
#ifdef ENABLE_SCALER
                else if (csc_handle->hw_property.fixed_node < CSC_HW_MAX)
                    csc_handle->csc_hw_handle = exynos_sc_create(csc_handle->hw_property.fixed_node - CSC_HW_SC0);
#endif
                else
                    csc_handle->csc_hw_handle = NULL;
            } else {
                csc_handle->csc_hw_handle = exynos_gsc_create();
            }
            ALOGV("%s:: CSC_HW_TYPE_GSCALER", __func__);
            break;
#endif
        default:
            ALOGE("%s:: unsupported csc_hw_type, csc use sw", __func__);
            csc_handle->csc_hw_handle = NULL;
            break;
        }
    }

    if (csc_handle->csc_method == CSC_METHOD_HW) {
        if (csc_handle->csc_hw_handle == NULL) {
            ALOGE("%s:: CSC_METHOD_HW can't open HW", __func__);
            ret = CSC_Error;
        }
    }

    ALOGV("%s:: CSC_METHOD=%d", __func__, csc_handle->csc_method);

    return ret;
}

static CSC_ERRORCODE csc_set_format(
    void *handle)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    if (csc_handle->csc_method == CSC_METHOD_HW) {
        switch (csc_handle->csc_hw_type) {
#ifdef ENABLE_FIMC
        case CSC_HW_TYPE_FIMC:
            exynos_fimc_set_src_format(
                csc_handle->csc_hw_handle,
                ALIGN(csc_handle->src_format.width, FIMC_IMG_ALIGN_WIDTH),
                ALIGN(csc_handle->src_format.height, FIMC_IMG_ALIGN_HEIGHT),
                csc_handle->src_format.crop_left,
                csc_handle->src_format.crop_top,
                csc_handle->src_format.crop_width,
                csc_handle->src_format.crop_height,
                HAL_PIXEL_FORMAT_2_V4L2_PIX(csc_handle->src_format.color_format),
                csc_handle->src_format.cacheable,
                csc_handle->hw_property.mode_drm);

            exynos_fimc_set_dst_format(
                csc_handle->csc_hw_handle,
                ALIGN(csc_handle->dst_format.width, FIMC_IMG_ALIGN_WIDTH),
                ALIGN(csc_handle->dst_format.height, FIMC_IMG_ALIGN_HEIGHT),
                csc_handle->dst_format.crop_left,
                csc_handle->dst_format.crop_top,
                csc_handle->dst_format.crop_width,
                csc_handle->dst_format.crop_height,
                HAL_PIXEL_FORMAT_2_V4L2_PIX(csc_handle->dst_format.color_format),
                csc_handle->dst_format.cacheable,
                csc_handle->hw_property.mode_drm,
                0);
            break;
#endif
#ifdef ENABLE_GSCALER
        case CSC_HW_TYPE_GSCALER:
            if (csc_handle->hw_property.fixed_node < CSC_HW_SC0) {
                exynos_gsc_set_csc_property(
                    csc_handle->csc_hw_handle,
                    csc_handle->csc_mode,
                    csc_handle->csc_range,
                    csc_handle->colorspace);

                exynos_gsc_set_src_format(
                    csc_handle->csc_hw_handle,
                    ALIGN(csc_handle->src_format.width, GSCALER_IMG_ALIGN),
                    ALIGN(csc_handle->src_format.height, GSCALER_IMG_ALIGN),
                    csc_handle->src_format.crop_left,
                    csc_handle->src_format.crop_top,
                    csc_handle->src_format.crop_width,
                    csc_handle->src_format.crop_height,
                    HAL_PIXEL_FORMAT_2_V4L2_PIX(csc_handle->src_format.color_format),
                    csc_handle->src_format.cacheable,
                    csc_handle->hw_property.mode_drm);

                exynos_gsc_set_dst_format(
                    csc_handle->csc_hw_handle,
                    csc_handle->dst_format.width,
                    csc_handle->dst_format.height,
                    csc_handle->dst_format.crop_left,
                    csc_handle->dst_format.crop_top,
                    csc_handle->dst_format.crop_width,
                    csc_handle->dst_format.crop_height,
                    HAL_PIXEL_FORMAT_2_V4L2_PIX(csc_handle->dst_format.color_format),
                    csc_handle->dst_format.cacheable,
                    csc_handle->hw_property.mode_drm);
#ifdef ENABLE_SCALER
            } else {
                exynos_sc_set_src_format(
                    csc_handle->csc_hw_handle,
                    csc_handle->src_format.width,
                    csc_handle->src_format.height,
                    csc_handle->src_format.crop_left,
                    csc_handle->src_format.crop_top,
                    csc_handle->src_format.crop_width,
                    csc_handle->src_format.crop_height,
                    HAL_PIXEL_FORMAT_2_V4L2_PIX(csc_handle->src_format.color_format),
                    csc_handle->src_format.cacheable,
                    csc_handle->hw_property.mode_drm,
                    1);

                exynos_sc_set_dst_format(
                    csc_handle->csc_hw_handle,
                    csc_handle->dst_format.width,
                    csc_handle->dst_format.height,
                    csc_handle->dst_format.crop_left,
                    csc_handle->dst_format.crop_top,
                    csc_handle->dst_format.crop_width,
                    csc_handle->dst_format.crop_height,
                    HAL_PIXEL_FORMAT_2_V4L2_PIX(csc_handle->dst_format.color_format),
                    csc_handle->dst_format.cacheable,
                    csc_handle->hw_property.mode_drm,
                    1);
#endif
            }
            break;
#endif
        default:
            ALOGE("%s:: unsupported csc_hw_type", __func__);
            break;
        }
    }

    return ret;
}

static CSC_ERRORCODE csc_set_buffer(
    void *handle)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    if (csc_handle->csc_method == CSC_METHOD_HW) {
        switch (csc_handle->csc_hw_type) {
#ifdef ENABLE_FIMC
        case CSC_HW_TYPE_FIMC:
            exynos_fimc_set_src_addr(csc_handle->csc_hw_handle, csc_handle->src_buffer.planes, csc_handle->src_buffer.mem_type, -1);
            exynos_fimc_set_dst_addr(csc_handle->csc_hw_handle, csc_handle->dst_buffer.planes, csc_handle->dst_buffer.mem_type, -1);
            break;
#endif
#ifdef ENABLE_GSCALER
        case CSC_HW_TYPE_GSCALER:
            if (csc_handle->hw_property.fixed_node < CSC_HW_SC0) {
                exynos_gsc_set_src_addr(csc_handle->csc_hw_handle, csc_handle->src_buffer.planes, csc_handle->src_buffer.mem_type, -1);
                exynos_gsc_set_dst_addr(csc_handle->csc_hw_handle, csc_handle->dst_buffer.planes, csc_handle->dst_buffer.mem_type, -1);
#ifdef ENABLE_SCALER
            } else {
                exynos_sc_set_src_addr(csc_handle->csc_hw_handle, csc_handle->src_buffer.planes, csc_handle->src_buffer.mem_type, -1);
                exynos_sc_set_dst_addr(csc_handle->csc_hw_handle, csc_handle->dst_buffer.planes, csc_handle->dst_buffer.mem_type, -1);
#endif
            }
            break;
#endif
        default:
            ALOGE("%s:: unsupported csc_hw_type", __func__);
            break;
        }
    }

    return ret;
}

void *csc_init(
    CSC_METHOD method)
{
    CSC_HANDLE *csc_handle;
    csc_handle = (CSC_HANDLE *)malloc(sizeof(CSC_HANDLE));
    if (csc_handle == NULL)
        return NULL;

    memset(csc_handle, 0, sizeof(CSC_HANDLE));
    csc_handle->hw_property.fixed_node = -1;
    csc_handle->hw_property.mode_drm = 0;
    csc_handle->csc_method = method;

    return (void *)csc_handle;
}

CSC_ERRORCODE csc_deinit(
    void *handle)
{
    CSC_ERRORCODE ret = CSC_ErrorNone;
    CSC_HANDLE *csc_handle;

    if (handle == NULL)
        return ret;

    csc_handle = (CSC_HANDLE *)handle;
    if (csc_handle->csc_method == CSC_METHOD_HW) {
        switch (csc_handle->csc_hw_type) {
#ifdef ENABLE_FIMC
        case CSC_HW_TYPE_FIMC:
            exynos_fimc_destroy(csc_handle->csc_hw_handle);
            break;
#endif
#ifdef ENABLE_GSCALER
        case CSC_HW_TYPE_GSCALER:
            if (csc_handle->hw_property.fixed_node < CSC_HW_SC0)
                exynos_gsc_destroy(csc_handle->csc_hw_handle);
#ifdef ENABLE_SCALER
            else
                exynos_sc_destroy(csc_handle->csc_hw_handle);
#endif
            break;
#endif
        default:
            ALOGE("%s:: unsupported csc_hw_type", __func__);
            break;
        }
    }

    free(csc_handle);
    ret = CSC_ErrorNone;

    return ret;
}

CSC_ERRORCODE csc_get_method(
    void           *handle,
    CSC_METHOD     *method)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    *method = csc_handle->csc_method;

    return ret;
}

CSC_ERRORCODE csc_set_method(
    void           *handle,
    CSC_METHOD      method)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;
    csc_handle = (CSC_HANDLE *)handle;

    switch (method) {
    case CSC_METHOD_SW:
    case CSC_METHOD_HW:
        csc_handle->csc_method = method;
        break;
    default:
        ret = CSC_Error;
        break;
    }

    return ret;
}

CSC_ERRORCODE csc_set_hw_property(
    void                *handle,
    CSC_HW_PROPERTY_TYPE property,
    int                  value)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    switch (property) {
    case CSC_HW_PROPERTY_FIXED_NODE:
        csc_handle->hw_property.fixed_node = value;
        break;
    case CSC_HW_PROPERTY_MODE_DRM:
        csc_handle->hw_property.mode_drm = value;
        break;
    default:
        ALOGE("%s:: not supported hw property", __func__);
        ret = CSC_ErrorUnsupportFormat;
    }

    return ret;
}

CSC_ERRORCODE csc_get_eq_property(
    void              *handle,
    CSC_EQ_MODE       *csc_mode,
    CSC_EQ_RANGE      *csc_range,
    CSC_EQ_COLORSPACE *colorspace)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    *csc_mode = csc_handle->csc_mode;
    *csc_range = csc_handle->csc_range;
    *colorspace = csc_handle->colorspace;

    return ret;
}

CSC_ERRORCODE csc_set_eq_property(
    void              *handle,
    CSC_EQ_MODE        csc_mode,
    CSC_EQ_RANGE       csc_range,
    CSC_EQ_COLORSPACE  colorspace)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_Error;

    csc_handle = (CSC_HANDLE *)handle;
    csc_handle->csc_mode = csc_mode;
    csc_handle->csc_range = csc_range;
    csc_handle->colorspace = colorspace;

    return ret;
}

CSC_ERRORCODE csc_get_src_format(
    void           *handle,
    unsigned int   *width,
    unsigned int   *height,
    unsigned int   *crop_left,
    unsigned int   *crop_top,
    unsigned int   *crop_width,
    unsigned int   *crop_height,
    unsigned int   *color_format,
    unsigned int   *cacheable)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    *width = csc_handle->src_format.width;
    *height = csc_handle->src_format.height;
    *crop_left = csc_handle->src_format.crop_left;
    *crop_top = csc_handle->src_format.crop_top;
    *crop_width = csc_handle->src_format.crop_width;
    *crop_height = csc_handle->src_format.crop_height;
    *color_format = csc_handle->src_format.color_format;
    *cacheable = csc_handle->src_format.cacheable;

    return ret;
}

CSC_ERRORCODE csc_set_src_format(
    void           *handle,
    unsigned int    width,
    unsigned int    height,
    unsigned int    crop_left,
    unsigned int    crop_top,
    unsigned int    crop_width,
    unsigned int    crop_height,
    unsigned int    color_format,
    unsigned int    cacheable)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    csc_handle->src_format.width = width;
    csc_handle->src_format.height = height;
    csc_handle->src_format.crop_left = crop_left;
    csc_handle->src_format.crop_top = crop_top;
    csc_handle->src_format.crop_width = crop_width;
    csc_handle->src_format.crop_height = crop_height;
    csc_handle->src_format.color_format = color_format;
    csc_handle->src_format.cacheable = cacheable;

    return ret;
}

CSC_ERRORCODE csc_get_dst_format(
    void           *handle,
    unsigned int   *width,
    unsigned int   *height,
    unsigned int   *crop_left,
    unsigned int   *crop_top,
    unsigned int   *crop_width,
    unsigned int   *crop_height,
    unsigned int   *color_format,
    unsigned int   *cacheable)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    *width = csc_handle->dst_format.width;
    *height = csc_handle->dst_format.height;
    *crop_left = csc_handle->dst_format.crop_left;
    *crop_top = csc_handle->dst_format.crop_top;
    *crop_width = csc_handle->dst_format.crop_width;
    *crop_height = csc_handle->dst_format.crop_height;
    *color_format = csc_handle->dst_format.color_format;
    *cacheable = csc_handle->dst_format.cacheable;

    return ret;
}

CSC_ERRORCODE csc_set_dst_format(
    void           *handle,
    unsigned int    width,
    unsigned int    height,
    unsigned int    crop_left,
    unsigned int    crop_top,
    unsigned int    crop_width,
    unsigned int    crop_height,
    unsigned int    color_format,
    unsigned int    cacheable)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    csc_handle->dst_format.width = width;
    csc_handle->dst_format.height = height;
    csc_handle->dst_format.crop_left = crop_left;
    csc_handle->dst_format.crop_top = crop_top;
    csc_handle->dst_format.crop_width = crop_width;
    csc_handle->dst_format.crop_height = crop_height;
    csc_handle->dst_format.color_format = color_format;
    csc_handle->dst_format.cacheable = cacheable;

    return ret;
}

CSC_ERRORCODE csc_set_src_buffer(
    void *handle,
    void *addr[3],
    int mem_type)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    csc_handle->src_buffer.planes[CSC_Y_PLANE] = addr[0];
    csc_handle->src_buffer.planes[CSC_U_PLANE] = addr[1];
    csc_handle->src_buffer.planes[CSC_V_PLANE] = addr[2];
    csc_handle->src_buffer.mem_type = mem_type;

    return ret;
}

CSC_ERRORCODE csc_set_dst_buffer(
    void *handle,
    void *addr[3],
    int mem_type)
{
    CSC_HANDLE *csc_handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (handle == NULL)
        return CSC_ErrorNotInit;

    csc_handle = (CSC_HANDLE *)handle;
    csc_handle->dst_buffer.planes[CSC_Y_PLANE] = addr[0];
    csc_handle->dst_buffer.planes[CSC_U_PLANE] = addr[1];
    csc_handle->dst_buffer.planes[CSC_V_PLANE] = addr[2];
    csc_handle->dst_buffer.mem_type = mem_type;

    return ret;
}

CSC_ERRORCODE csc_convert(
    void *handle)
{
    CSC_HANDLE *csc_handle = (CSC_HANDLE *)handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (csc_handle == NULL)
        return CSC_ErrorNotInit;

    if ((csc_handle->csc_method == CSC_METHOD_HW) &&
        (csc_handle->csc_hw_handle == NULL)) {
        ret = csc_init_hw(handle);
        if (ret != CSC_ErrorNone)
            return ret;
    }

    ret = csc_set_format(csc_handle);
    if (ret != CSC_ErrorNone)
        return ret;

    ret = csc_set_buffer(csc_handle);
    if (ret != CSC_ErrorNone)
        return ret;

    if (csc_handle->csc_method == CSC_METHOD_HW)
        ret = conv_hw(csc_handle);
    else
        ret = conv_sw(csc_handle);

    return ret;
}

CSC_ERRORCODE csc_convert_with_rotation(
    void *handle, int rotation, int flip_horizontal, int flip_vertical)
{
    CSC_HANDLE *csc_handle = (CSC_HANDLE *)handle;
    CSC_ERRORCODE ret = CSC_ErrorNone;

    if (csc_handle == NULL)
        return CSC_ErrorNotInit;

    if ((csc_handle->csc_method == CSC_METHOD_HW) &&
        (csc_handle->csc_hw_handle == NULL))
        csc_init_hw(handle);

    csc_set_format(csc_handle);
    csc_set_buffer(csc_handle);

#ifdef ENABLE_FIMC
    exynos_fimc_set_rotation(csc_handle->csc_hw_handle, rotation, flip_horizontal, flip_vertical);
#endif
#ifdef ENABLE_GSCALER
    if (csc_handle->hw_property.fixed_node < CSC_HW_SC0)
        exynos_gsc_set_rotation(csc_handle->csc_hw_handle, rotation, flip_horizontal, flip_vertical);
#ifdef ENABLE_SCALER
    else
        exynos_sc_set_rotation(csc_handle->csc_hw_handle, rotation, flip_horizontal, flip_vertical);
#endif
#endif

    if (csc_handle->csc_method == CSC_METHOD_HW)
        ret = conv_hw(csc_handle);
    else
        ret = conv_sw(csc_handle);

    return ret;
}

CSC_ERRORCODE csc_set_filter_property(void *handle)
{
    return CSC_ErrorNone;
}
