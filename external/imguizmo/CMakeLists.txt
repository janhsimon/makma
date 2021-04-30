set(TARGET_NAME imguizmo)

add_library(${TARGET_NAME} STATIC IMPORTED GLOBAL)
target_include_directories(${TARGET_NAME} INTERFACE include)

if(WIN32)
  set(LIB lib/win/imguizmo.lib)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/${LIB})