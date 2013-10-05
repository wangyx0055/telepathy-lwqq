set(SRC 
    main.c connection-manager.c protocol.c connection.c
    )

include_directories(
    ${PROJECT_BINARY_DIR}
    ${TP_GLIB_INCLUDE_DIRS}
    )

add_executable(telepathy-lwqq ${SRC})

target_link_libraries(telepathy-lwqq 
    ${TP_GLIB_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${GLIB_LIBRARIES}
    lwqq
    )

install(TARGETS telepathy-lwqq DESTINATION "/usr/lib/telepathy")
