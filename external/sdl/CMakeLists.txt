set(TARGET_NAME sdl)

add_library(${TARGET_NAME} SHARED IMPORTED GLOBAL)
target_include_directories(${TARGET_NAME} INTERFACE include)

if(WIN32)
  set(STATIC_LIB lib/win/SDL2.lib)
  set(SHARED_LIB lib/win/SDL2.dll)
  set_target_properties(${TARGET_NAME} PROPERTIES IMPORTED_IMPLIB ${CMAKE_CURRENT_LIST_DIR}/${STATIC_LIB} IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/${SHARED_LIB})
  install(FILES ${CMAKE_CURRENT_LIST_DIR}/${SHARED_LIB} DESTINATION $<TARGET_FILE_DIR:${PROJECT_NAME}>)
elseif(UNIX)
  set(LIB lib/linux/libSDL2.so)
  set_target_properties(${TARGET_NAME} PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/${LIB})
  install(FILES ${CMAKE_CURRENT_LIST_DIR}/${LIB} DESTINATION $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

set(TARGET_NAME sdl-main)

add_library(${TARGET_NAME} STATIC IMPORTED GLOBAL)
target_include_directories(${TARGET_NAME} INTERFACE include)

if(WIN32)
  set(LIB lib/win/SDL2main.lib)
elseif(UNIX)
  set(LIB lib/linux/libSDL2main.a)
endif()

set_target_properties(${TARGET_NAME} PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_LIST_DIR}/${LIB})