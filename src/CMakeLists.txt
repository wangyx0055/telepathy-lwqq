set(SRC 
    main.c 
    connection.c 
    connection-manager.c 
    contact-list.c
    protocol.c 
    im-manager.c
    captcha.c
    presence.c
    aliasing.c
    avatar.c
    )

include_directories(
    ${PROJECT_BINARY_DIR}
    ${TP_GLIB_INCLUDE_DIRS}
    ${LWQQ_INCLUDE_DIRS}
    )

add_executable(telepathy-lwqq ${SRC})

target_link_libraries(telepathy-lwqq 
    ${DBUS_GLIB_LIBRARIES}
    ${TP_GLIB_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${LWQQ_LIBRARIES}
    )

install(TARGETS telepathy-lwqq DESTINATION "/usr/lib/telepathy")
