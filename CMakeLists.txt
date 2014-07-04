project(telepathy-lwqq)
cmake_minimum_required(VERSION 2.8)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TP_GLIB REQUIRED telepathy-glib)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(DBUS_GLIB REQUIRED dbus-glib-1)
pkg_check_modules(LWQQ REQUIRED lwqq>=0.3.1)

set(datadir "/usr/local/share/lwqq")
configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

add_subdirectory(src)
