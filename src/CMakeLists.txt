qt5_wrap_ui(UI_MAINWINDOW mainwindow.ui)


set(SOURCES main.cc mainwindow.cc notes.cc)
set(HEADERS mainwindow.h notes.h)

add_executable(qnv ${SOURCES} ${UI_MAINWINDOW})

target_link_libraries(qnv ${Boost_LIBRARIES})
qt5_use_modules(qnv Widgets)


qnv_clang_format(${SOURCES} ${HEADERS})

set_property(SOURCE ${UI_MAINWINDOW} PROPERTY SKIP_AUTOMOC ON)

install(TARGETS qnv RUNTIME DESTINATION bin)