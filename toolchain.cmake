# TODO Copyright

set (CMAKE_SYSTEM_NAME Linux)

add_definitions ("-Wall")
add_definitions ("-g -O3 -DLINUX=1 -DEGL_API_FB -DEGL_API_WL")

# FIXME as your toolchain path

set (CMAKE_FIND_ROOT_PATH $ENV{PKG_CONFIG_SYSROOT_DIR})
set (CMAKE_INSTALL_PREFIX $ENV{PKG_CONFIG_SYSROOT_DIR})
# EOF

