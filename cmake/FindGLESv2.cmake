FIND_PATH(GLESv2_INCLUDE_DIR GLES2/gl2.h
/usr/include
/home/denso/weeklybuild/e09/imx-drv-libs/gpu-viv-bin-mx6q/hardfp/usr/include
)

FIND_LIBRARY(GLESv2_LIBRARY
NAMES GLESv2
PATHS /home/denso/weeklybuild/e09/imx-drv-libs/gpu-viv-bin-mx6q/hardfp/usr/lib 
)

SET(GLESv2_FOUND "NO")

IF(GLESv2_LIBRARY)
    SET(GLESv2_FOUND "YES")
    MESSAGE(STATUS "Found GLESv2 libs: ${GLESv2_LIBRARY}")
    MESSAGE(STATUS "Found GLESv2 includes: ${GLESv2_INCLUDE_DIR}")
ENDIF(GLESv2_LIBRARY)

MARK_AS_ADVANCED(
  GLESv2_INCLUDE_DIR
  GLESv2_LIBRARY
)
