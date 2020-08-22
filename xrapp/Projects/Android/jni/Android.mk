
LOCAL_PATH:= $(call my-dir)

#--------------------------------------------------------
# libvrcubeworld.so
#--------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE			:= xrapp
LOCAL_CFLAGS			:= -std=c99 -Werror
LOCAL_SRC_FILES			:= ../../../Src/app.c
LOCAL_LDLIBS			:= -llog -landroid -lGLESv3 -lEGL		# include default libraries

LOCAL_LDFLAGS			:= -u ANativeActivity_onCreate

LOCAL_STATIC_LIBRARIES	:= android_native_app_glue 
LOCAL_SHARED_LIBRARIES	:= openxr_loader

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,OpenXR/Projects/Android/jni)
