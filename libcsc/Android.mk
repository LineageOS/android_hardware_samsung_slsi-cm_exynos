LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	csc.c

LOCAL_C_INCLUDES := \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
	hardware/samsung_slsi-cm/$(TARGET_BOARD_PLATFORM)/include \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../libexynosutils

LOCAL_ADDITIONAL_DEPENDENCIES := \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)

LOCAL_CFLAGS :=

LOCAL_MODULE := libcsc

LOCAL_PRELINK_MODULE := false

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := libswconverter
LOCAL_SHARED_LIBRARIES := liblog libexynosutils

LOCAL_CFLAGS += -DUSE_SAMSUNG_COLORFORMAT

ifeq ($(BOARD_USES_FIMC),true)
LOCAL_CFLAGS += -DENABLE_FIMC
LOCAL_SHARED_LIBRARIES += libexynosfimc
endif
ifeq ($(TARGET_BOARD_PLATFORM),exynos5)
LOCAL_CFLAGS += -DENABLE_GSCALER
LOCAL_SHARED_LIBRARIES += libexynosgscaler
ifeq ($(BOARD_USES_SCALER), true)
LOCAL_CFLAGS += -DENABLE_SCALER
LOCAL_SHARED_LIBRARIES += libexynosscaler
endif
endif

LOCAL_CFLAGS += -DUSE_ION

ifeq ($(TARGET_ARCH), arm64)
ifeq ($(TARGET_2ND_ARCH),arm)
LOCAL_CFLAGS += -DDISABLE_NEON
endif
endif

LOCAL_SHARED_LIBRARIES += libion_exynos

include $(BUILD_SHARED_LIBRARY)
