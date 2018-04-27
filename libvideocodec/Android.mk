LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(BOARD_USE_VP8ENC_SUPPORT), true)
LOCAL_CFLAGS += -DUSE_VP8ENC_SUPPORT
endif

LOCAL_SRC_FILES := \
	ExynosVideoInterface.c \
	dec/ExynosVideoDecoder.c \
	enc/ExynosVideoEncoder.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(TOP)/hardware/samsung_slsi-cm/exynos/include \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_BOARD_PLATFORM)/include

LOCAL_HEADER_LIBRARIES := generated_kernel_headers

ifeq ($(BOARD_USE_KHRONOS_OMX_HEADER), true)
LOCAL_C_INCLUDES += $(TOP)/hardware/samsung_slsi-cm/openmax/include/khronos
else
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/openmax
endif

ifeq ($(BOARD_USE_HEVCDEC_SUPPORT), true)
LOCAL_CFLAGS += -DUSE_HEVCDEC_SUPPORT
endif

ifeq ($(BOARD_USE_HEVC_HWIP), true)
LOCAL_CFLAGS += -DUSE_HEVC_HWIP
endif

ifneq ($(filter exynos5422 exynos5430 exynos5433 exynos7420 exynos7580 exynos7870 exynos7880 exynos8890 exynos8895, $(TARGET_SOC)),)
LOCAL_CFLAGS += -DNEW_API
endif

LOCAL_MODULE := libExynosVideoApi
LOCAL_MODULE_TAGS := optional
LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)
