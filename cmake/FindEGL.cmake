FIND_PATH(EGL_INCLUDE_DIR EGL/egl.h
/usr/include
/include
/home/denso/weeklybuild/e09/imx-drv-libs/gpu-viv-bin-mx6q/hardfp/usr/include
)

FIND_LIBRARY(EGL_LIBRARY
NAMES EGL
PATHS /home/denso/weeklybuild/e09/imx-drv-libs/gpu-viv-bin-mx6q/hardfp/usr/lib 
)

SET(EGL_FOUND "NO")

IF(EGL_LIBRARY)
    SET(EGL_FOUND "YES")
    MESSAGE(STATUS "Found EGL libs: ${EGL_LIBRARY}")
    MESSAGE(STATUS "Found EGL includes: ${EGL_INCLUDE_DIR}")
ENDIF(EGL_LIBRARY)

MARK_AS_ADVANCED(
  EGL_INCLUDE_DIR
  EGL_LIBRARY
)
