LOCAL_PATH := $(call my-dir)

#########################sample_pwm###########################
include $(CLEAR_VARS)

LOCAL_MODULE :=	uvc
#ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -lpthread

LOCAL_SRC_FILES := main.c uvc.c  minrry.c

#LOCAL_C_INCLUDES := $(COMMON_UNF_INCLUDE)
#LOCAL_C_INCLUDES += $(COMMON_DRV_INCLUDE)
#LOCAL_C_INCLUDES += $(COMMON_API_INCLUDE)
#LOCAL_C_INCLUDES += $(MSP_UNF_INCLUDE)
#LOCAL_C_INCLUDES += $(MSP_DRV_INCLUDE)
#LOCAL_C_INCLUDES += $(MSP_API_INCLUDE)
#LOCAL_C_INCLUDES := $(TOP)/frameworks/av/include/media/
#LOCAL_C_INCLUDES += $(TOP)/system/core/include/system/
#LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/binder/
#LOCAL_C_INCLUDES := $(TOP)/bionic
#LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES := $(LOCAL_PATH)

#LOCAL_SHARED_LIBRARIES := libmedia liblog libutils libui libbinder libcutils libstlport

include $(BUILD_EXECUTABLE)
