
LOCAL_PATH:= $(call my-dir)

#--------------------------------------------------------
# libvrcubeworld.so
#--------------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE			:= vrapp
LOCAL_CFLAGS			:= -std=c++11 -Werror
LOCAL_SRC_FILES			:= ../../../Src/app.cpp
LOCAL_SRC_FILES			+= ../../../Src/cube.cpp
LOCAL_SRC_FILES			+= ../../../Src/geom.cpp
LOCAL_SRC_FILES			+= ../../../Src/gl.cpp
LOCAL_SRC_FILES			+= ../../../Src/prog.cpp
LOCAL_SRC_FILES			+= ../../../Src/sphere.cpp
LOCAL_SRC_FILES			+= ../../../Src/square.cpp
LOCAL_SRC_FILES			+= ../../../Src/stb.cpp
LOCAL_SRC_FILES			+= ../../../Src/tetra.cpp
LOCAL_SRC_FILES			+= ../../../Src/torus.cpp
LOCAL_LDLIBS			:= -llog -landroid -lGLESv3 -lEGL		# include default libraries

LOCAL_LDFLAGS			:= -u ANativeActivity_onCreate

LOCAL_STATIC_LIBRARIES	:= android_native_app_glue 
LOCAL_SHARED_LIBRARIES	:= vrapi

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)
