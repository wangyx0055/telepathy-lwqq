set(SRC 
    main.c 
    connection.c 
    connection-manager.c 
    contact-list.c
    protocol.c 
    im-manager.c
    captcha.c
    )

include_directories(
    ${PROJECT_BINARY_DIR}
    ${TP_GLIB_INCLUDE_DIRS}
	/usr/local/include/lwqq
    )

add_executable(telepathy-lwqq ${SRC})

target_link_libraries(telepathy-lwqq 
    ${TP_GLIB_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${GLIB_LIBRARIES}
    lwqq
    )

install(TARGETS telepathy-lwqq DESTINATION "/usr/lib/telepathy")
