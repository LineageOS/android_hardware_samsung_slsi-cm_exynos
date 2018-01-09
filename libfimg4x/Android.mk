#
# Copyright 2012, Samsung Electronics Co. LTD
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(BOARD_USES_SKIA_FIMGAPI),true)
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= \
	FimgApi.cpp   \
	FimgExynos5.cpp

LOCAL_C_INCLUDES += \
	$(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
	$(LOCAL_PATH)/../include \
	hardware/samsung_slsi-cm/$(TARGET_SOC)/include \
	hardware/samsung_slsi-cm/$(TARGET_BOARD_PLATFORM)/include

LOCAL_ADDITIONAL_DEPENDENCIES := \
	INSTALLED_KERNEL_HEADERS

LOCAL_SHARED_LIBRARIES:= liblog libutils libbinder

LOCAL_MODULE:= libfimg

include $(BUILD_SHARED_LIBRARY)

endif
