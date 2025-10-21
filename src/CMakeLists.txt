add_library(ptc_qt OBJECT ptc_qt.c)

add_executable(ptc_qt1_self_demo
    ptc_qt
    ptc_qt1.c
)
target_compile_definitions(ptc_qt1_self_demo PRIVATE SELFCAP)
target_compile_definitions(ptc_qt1_self_demo PRIVATE SAMA5D27_WLSOM1_EK=${SAMA5D27_WLSOM1_EK})

add_executable(ptc_qt1_mutual_demo
    ptc_qt
    ptc_qt1.c
)
target_compile_definitions(ptc_qt1_mutual_demo PRIVATE SAMA5D27_WLSOM1_EK=${SAMA5D27_WLSOM1_EK})

add_executable(ptc_qt2_mutual_demo
    ptc_qt
    ptc_qt2.c
)

add_executable(ptc_qt6_mutual_demo
    ptc_qt
    ptc_qt6.c
)

foreach(tgt IN ITEMS ptc_qt ptc_qt1_self_demo ptc_qt1_mutual_demo ptc_qt2_mutual_demo ptc_qt6_mutual_demo)
    target_include_directories(${tgt} PRIVATE ${LIBGPIOD_INCLUDE_DIRS} ${LIBEVDEV_INCLUDE_DIRS})
    target_compile_options(${tgt} PRIVATE ${LIBGPIOD_CFLAGS_OTHER} ${LIBEVDEV_CFLAGS_OTHER})
    target_link_directories(${tgt} PRIVATE ${LIBGPIOD_LIBRARY_DIRS} ${LIBEVDEV_LIBRARY_DIRS})
    target_link_libraries(${tgt} PRIVATE ${LIBGPIOD_LIBRARIES} ${LIBEVDEV_LIBRARIES})
    target_link_options(${tgt} PRIVATE ${LIBGPIOD_LDFLAGS_OTHER} ${LIBEVDEV_LDFLAGS_OTHER})
endforeach()

install(TARGETS ptc_qt1_self_demo ptc_qt1_mutual_demo ptc_qt2_mutual_demo ptc_qt6_mutual_demo)
