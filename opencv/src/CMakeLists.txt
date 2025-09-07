#----------------------------------------------------------------------------
# Application rule
#----------------------------------------------------------------------------
set(target_name myapp)
add_executable(${target_name})

# app sources
set(app_sources
  main.cpp
  version.h
  $<$<CXX_COMPILER_ID:MSVC>:utf8.manifest>
  $<$<CXX_COMPILER_ID:MSVC>:resource.rc>
)
configure_file(resource.rc.in
              "${CMAKE_CURRENT_SOURCE_DIR}/resource.rc" @ONLY)

target_sources(${target_name} PRIVATE ${app_sources})

set_target_properties(${target_name}
  PROPERTIES
  DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}"
  MSVC_RUNTIME_LIBRARY "${MSVC_RUNTIME_LIBRARY}"
)
target_compile_features(${target_name} PRIVATE cxx_std_17)
target_compile_features(${target_name} PRIVATE c_std_99)
target_compile_definitions(${target_name} PRIVATE NOMINMAX)

# optimize options
target_compile_options(${target_name} PRIVATE
#   $<$<CONFIG:Release>:/O2 /GL /Ob2 /Oi /Qpar /arch:AVX2>
  $<$<CONFIG:Release>:/O2 /GL /Ob2 /Oi /Qpar>
)

target_link_options(${target_name} PRIVATE
  $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO>
)

#
# Dependencies
#
find_package(cxxopts REQUIRED)
target_link_libraries(${target_name} PRIVATE cxxopts::cxxopts)

find_package(log4cplus CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE log4cplus::log4cplus)

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE fmt::fmt)

find_package(i18n-gettext CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE i18n-gettext)

find_package(OpenCV REQUIRED COMPONENTS core highgui video videoio)
target_include_directories(${target_name} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${target_name} PRIVATE ${OpenCV_LIBS})

find_package(cvui CONFIG REQUIRED)
target_link_libraries(${target_name} PRIVATE cvui::cvui)

#[[
# genelate pdb file for Release build.
if(MSVC)
  # genalate pdb file for release
  target_compile_options(${target_name} PRIVATE "$<$<CONFIG:Release>:/Zi>")
  target_link_options(${target_name} PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
  target_link_options(${target_name} PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
  target_link_options(${target_name} PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")
endif()
#]]

if(MSVC)
  # copy gstreamer plugins
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
  if(GSTREAMER_FOUND)
     message(STATUS "Detecting GStreamer plugins")
    pkg_get_variable(GSTREAMER_PLUGINS_DIR gstreamer-1.0 pluginsdir)
    file(GLOB GSTREAMER_PLUGINS_DLLS "${GSTREAMER_PLUGINS_DIR}/*.dll")
    add_custom_command(TARGET ${target_name} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target_name}>/gstreamer-1.0"
                      COMMAND ${CMAKE_COMMAND} -E copy_if_different
                      ${GSTREAMER_PLUGINS_DLLS}
                      "$<TARGET_FILE_DIR:${target_name}>/gstreamer-1.0"
                      COMMAND_EXPAND_LISTS)
    string(REPLACE "${GSTREAMER_PLUGINS_DIR}/" "" __PLUGINS_DLLS_REL "${GSTREAMER_PLUGINS_DLLS}")
    oslw_find_depdll(GSTREAMER_PLUGINS_DEPENDENT_DLLS
                     DLLS ${__PLUGINS_DLLS_REL}
                     WORKDIR ${GSTREAMER_PLUGINS_DIR})
    list(FILTER GSTREAMER_PLUGINS_DEPENDENT_DLLS EXCLUDE REGEX "lib/gstreamer-1.0")
    add_custom_command(TARGET ${target_name} POST_BUILD
                      COMMAND ${CMAKE_COMMAND} -E copy_if_different
                      ${GSTREAMER_PLUGINS_DEPENDENT_DLLS}
                      "$<TARGET_FILE_DIR:${target_name}>"
                      COMMAND_EXPAND_LISTS)
  endif()
endif()
if(MSVC)
  # copy external dlls to bindir.
  oslw_copy_depdll(TARGET ${target_name})
endif()

if(MSVC)
  # set startup project for Visual Studio.
  set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${target_name}")
endif()
#----------------------------------------------------------------------------
# Install Rules
#----------------------------------------------------------------------------
install(TARGETS ${target_name}
  RUNTIME DESTINATION bin COMPONENT Application
  LIBRARY DESTINATION lib COMPONENT Application
  ARCHIVE DESTINATION lib COMPONENT Application
)

if(MSVC)
  if(GSTREAMER_PLUGINS_DLLS)
    # install gstreamer plugins
    install(FILES ${GSTREAMER_PLUGINS_DLLS}
            DESTINATION bin/gstreamer-1.0 COMPONENT Application)
    install(FILES ${GSTREAMER_PLUGINS_DEPENDENT_DLLS}
            DESTINATION bin COMPONENT Application)
    # remove duplicated
    string(REGEX MATCHALL "[^/;]+.dll" EXCLUDE_DLLS "${GSTREAMER_PLUGINS_DEPENDENT_DLLS}")
  endif()
  # install external dlls to distdir
  oslw_install_depdll(
    TARGET ${target_name}
    DESTINATION bin
    COMPONENT Application
    EXCLUDE "${EXCLUDE_DLLS}"
  )
endif()

#[[
if(MSVC)
  # Install MSVC runtime libraries
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  include(InstallRequiredSystemLibraries)
  install(PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
          DESTINATION bin
          COMPONENT Application)
endif()
#]]
#----------------------------- End of file ----------------------------------
