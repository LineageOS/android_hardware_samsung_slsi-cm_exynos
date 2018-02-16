# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SHARED_LIBRARIES := liblog libutils libcutils libexynosutils \
	libexynosv4l2 libsync libhwcutils libhwcutilsmodule libmpp libbase libnativewindow

LOCAL_STATIC_LIBRARIES := libarect

ifeq ($(BOARD_USES_FIMC), true)
LOCAL_SHARED_LIBRARIES += libexynosfimc
else
LOCAL_SHARED_LIBRARIES += libexynosgscaler
endif
ifeq ($(BOARD_USES_HWC_SERVICES),true)
	LOCAL_CFLAGS += -DHWC_SERVICES
ifeq ($(BOARD_USE_S3D_SUPPORT),true)
	LOCAL_CFLAGS += -DS3D_SUPPORT
endif
endif

ifeq ($(BOARD_USES_VIRTUAL_DISPLAY), true)
	LOCAL_CFLAGS += -DUSES_VIRTUAL_DISPLAY
endif

ifeq ($(BOARD_USES_GSC_VIDEO),true)
	LOCAL_CFLAGS += -DGSC_VIDEO
endif

ifeq ($(BOARD_HDMI_INCAPABLE), true)
	LOCAL_CFLAGS += -DHDMI_INCAPABLE
endif

ifeq ($(BOARD_USES_FIMC), true)
	LOCAL_CFLAGS += -DUSES_FIMC
endif

# Exynos 5430 onwards use a decon frame buffer device, but still have the
# old kernel APIs for calling it (S3C_FB_*).
# Newer SoCs (Exynos 7420 onwards) make use of a new kernel API.
# WARNING: Support is highly experimental!
ifneq ($(filter exynos7420 exynos7580 exynos7880 exynos8890, $(TARGET_SOC)),)
	LOCAL_CFLAGS += -DDECON_FB
endif

LOCAL_CFLAGS += -DLOG_TAG=\"display\"
LOCAL_C_INCLUDES := \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../libhwc \
	$(LOCAL_PATH)/../libhwcutils \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_BOARD_PLATFORM)/include \
	$(TOP)/hardware/samsung_slsi-cm/exynos/libexynosutils \
	$(TOP)/hardware/samsung_slsi-cm/exynos/libmpp \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_SOC)/include \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_SOC)/libhwcmodule \
	$(TOP)/hardware/samsung_slsi-cm/$(TARGET_SOC)/libhwcutilsmodule

LOCAL_ADDITIONAL_DEPENDENCIES := \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_SRC_FILES := \
	ExynosDisplay.cpp \
	ExynosOverlayDisplay.cpp

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := libexynosdisplay
include $(BUILD_SHARED_LIBRARY)

