#--
#--  Copyright (c) 2011, Michael A. Jackson. BlueQuartz Software
#--  Copyright (c) 2011, Michael Groeber, US Air Force Research Laboratory
#--  Copyright (c) 2014 Dr. Joseph C. Tucker (UES, Inc.)
#--  All rights reserved.
#--  BSD License: http://www.opensource.org/licenses/bsd-license.html
#--
#-- This code was partly written under US Air Force Contract FA8650-07-D-5800 & FA8650-13-M-5048
#--
	
project(R3DAnalytics)

set(PLUGIN_NAME R3DAnalytics)

# --------------------------------------------------------------------
# Look for Qt 4 as we need it for the plugin GUI to be generated
if(NOT QT4_FOUND)
    message(FATAL_ERROR "Qt 4 is Needed for this R3DAnalytics.")
endif()
include( ${QT_USE_FILE} )


set(R3DAnalytics_Project_SRCS "")

include(${DREAM3DProj_SOURCE_DIR}/Source/DREAM3DLib/DREAM3DLibMacros.cmake)

#-------
# Start writing this header file
set(AllFiltersHeaderFile ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}Filters/${PLUGIN_NAME}Filters.h_tmp)
file(WRITE ${AllFiltersHeaderFile} "#ifndef _${PLUGIN_NAME}_Filters_H_\n#define _${PLUGIN_NAME}_Filters_H_\n\n")
# --------------------------------------------------------------------
# Create a File that will be used to generate a program that itself generates a
# QFilterWidget derived header for each of the filters based on thier exposed
# filter options
set(CodeGeneratorFile  ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}CodeGenerator.h)
file(WRITE ${CodeGeneratorFile} "")

# --------------------------------------------------------------------
# Create a File that has all the headers for the various Filter Widgets
set(AllFilterWidgetsHeaderFile ${${PLUGIN_NAME}_BINARY_DIR}/FilterWidgetHeaders.h_tmp)
file(WRITE ${AllFilterWidgetsHeaderFile} "#ifndef _ALL${PLUGIN_NAME}_H_\n#define _ALL${PLUGIN_NAME}_H_\n")
# --------------------------------------------------------------------
# Create a File that will register all of the DREAM3D Filters
set(RegisterKnownFilterWidgetsFile ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}Filters/RegisterKnownFilterWidgets.cpp_tmp)
file(WRITE ${RegisterKnownFilterWidgetsFile} "/* This file was AUTO GENERATED During the CMake Run. DO NOT EDIT THIS FILE. Edit the CMake files that generate this file */\n")
file(APPEND ${RegisterKnownFilterWidgetsFile} "#include \"PipelineBuilder/FilterWidgetManager.h\"\n")
file(APPEND ${RegisterKnownFilterWidgetsFile} "#include \"QFilterWidgetFactory.hpp\"\n")
file(APPEND ${RegisterKnownFilterWidgetsFile} "#include \"FilterWidgetHeaders.h\"\n")
file(APPEND ${RegisterKnownFilterWidgetsFile} "\nvoid ${PLUGIN_NAME}Plugin::registerFilterWidgets()\n{\n")

# --------------------------------------------------------------------
# Create a File that will register all of the DREAM3D Filters
set(RegisterKnownFiltersFile ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}Filters/RegisterKnownFilters.cpp_tmp)
file(WRITE ${RegisterKnownFiltersFile} "/* This file was AUTO GENERATED During the CMake Run. DO NOT EDIT THIS FILE. Edit the CMake files that generate this file */\n")
file(APPEND ${RegisterKnownFiltersFile} "/* The CMake code that generates this file is located in ${CMAKE_CURRENT_LIST_FILE}*/\n")
file(APPEND ${RegisterKnownFiltersFile} "/* This file will be directly included in another source file. */\n")
file(APPEND ${RegisterKnownFiltersFile} "#include \"${PLUGIN_NAME}Filters.h\"\n")
file(APPEND ${RegisterKnownFiltersFile} "\nvoid ${PLUGIN_NAME}Plugin::registerFilters(FilterManager* fm)\n{\n")

# --------------------------------------------------------------------
# Generate a file with all of our MOC'ed sources and  headers
set(FilterWidget_GEN_HDRS_File ${R3DAnalytics_BINARY_DIR}/FilterWidget_GEN_HDRS_File.cmake)
set(FilterWidget_GEN_SRCS_File ${R3DAnalytics_BINARY_DIR}/FilterWidget_GEN_SRCS_File.cmake)
file(WRITE ${FilterWidget_GEN_HDRS_File})
file(WRITE ${FilterWidget_GEN_SRCS_File})

set(Project_SRCS "")

# Add in our Filter classes
include(${${PLUGIN_NAME}_SOURCE_DIR}/${PLUGIN_NAME}Filters/SourceList.cmake)


set(${PLUGIN_NAME}_Project_SRCS ${Project_SRCS})

#-----------------------------------------------------
# Close up all the files that we just generated
file(APPEND ${AllFiltersHeaderFile} "\n#endif\n")
file(APPEND ${AllFilterWidgetsHeaderFile} "\n#endif\n")
file(APPEND ${RegisterKnownFilterWidgetsFile} "}\n")
file(APPEND ${RegisterKnownFiltersFile} "\n }\n")

cmpReplaceFileIfDifferent(OLD_FILE_PATH  ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}Filters/${PLUGIN_NAME}Filters.h
                          NEW_FILE_PATH  ${AllFiltersHeaderFile} )
cmpReplaceFileIfDifferent(OLD_FILE_PATH  ${${PLUGIN_NAME}_BINARY_DIR}/FilterWidgetHeaders.h
                          NEW_FILE_PATH  ${AllFilterWidgetsHeaderFile} )
cmpReplaceFileIfDifferent(OLD_FILE_PATH  ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}Filters/RegisterKnownFilterWidgets.cpp
                          NEW_FILE_PATH  ${RegisterKnownFilterWidgetsFile} )
cmpReplaceFileIfDifferent(OLD_FILE_PATH  ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}Filters/RegisterKnownFilters.cpp
                          NEW_FILE_PATH  ${RegisterKnownFiltersFile} )

set(AllFiltersHeaderFile ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}Filters/${PLUGIN_NAME}Filters.h)
set(AllFilterWidgetsHeaderFile ${${PLUGIN_NAME}_BINARY_DIR}/FilterWidgetHeaders.h)
set(RegisterKnownFilterWidgetsFile ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}Filters/RegisterKnownFilterWidgets.cpp)
set(RegisterKnownFiltersFile ${${PLUGIN_NAME}_BINARY_DIR}/${PLUGIN_NAME}Filters/RegisterKnownFilters.cpp)

#--------------------------
include_directories( ${HDF5_INCLUDE_DIR} )
include_directories( ${PROJECT_CODE_DIR} )
include_directories( ${QtSupport_BINARY_DIR} )
include_directories( ${QtSupport_SOURCE_DIR} )
include_directories( ${PipelineBuilderLib_SOURCE_DIR} )
include_directories( ${PipelineBuilderLib_BINARY_DIR} )
include_directories( ${FilterWidgetsLib_SOURCE_DIR} )
include_directories( ${${PLUGIN_NAME}_BINARY_DIR} )
include_directories( ${${PLUGIN_NAME}_BINARY_DIR}/.. )
include_directories( ${${PLUGIN_NAME}_SOURCE_DIR}/.. )
include_directories( ${${PLUGIN_NAME}_SOURCE_DIR})

# ******************************************************************************
# Create our custom executable that will generate most of our QFilterWidget
# classes from information stored in the Filters themselves.
# Read the file back into a string so we can configure our file
file(READ ${RegisterKnownFilterWidgetsFile} QFILTERWIDGET_FACTORY_SOURCE)
file(READ ${CodeGeneratorFile} CODE_GEN_FILTER_FUNCTION)
file(READ ${FilterWidget_GEN_HDRS_File} ${PLUGIN_NAME}Widget_GEN_HDRS)
file(READ ${FilterWidget_GEN_SRCS_File} ${PLUGIN_NAME}Widget_GEN_SRCS)

# Remove the temp files that we no longer need
file(REMOVE ${CodeGeneratorFile})
file(REMOVE ${FilterWidget_GEN_HDRS_File})
file(REMOVE ${FilterWidget_GEN_SRCS_File})


set(FILTER_HEADERS "#include \"${AllFiltersHeaderFile}\"")
set(FILTER_WIDGETS_TEMP_DIR ${${PLUGIN_NAME}_BINARY_DIR}/Temp)
set(FILTER_WIDGETS_BINARY_DIR ${${PLUGIN_NAME}_BINARY_DIR})
set(FILTER_WIDGETS_SOURCE_DIR ${${PLUGIN_NAME}_SOURCE_DIR})
set(FILTER_WIDGETS_DOCS_DIR ${${PLUGIN_NAME}_SOURCE_DIR}/Documentation/Filters)
set(FILTER_INCLUDE_PREFIX "${PLUGIN_NAME}")
file(MAKE_DIRECTORY ${${PLUGIN_NAME}_BINARY_DIR}/Temp)
configure_file( ${FilterWidgetsLib_SOURCE_DIR}/FilterWidgetsCodeGen.h.in
                ${${PLUGIN_NAME}_BINARY_DIR}/FilterWidgetCodeGen.h)

# --------------------------------------------------------------------
# On Linux we need -fPIC for linking static and shared libraries
if(UNIX AND NOT APPLE)
  set_source_files_properties(${${PLUGIN_NAME}_Project_SRCS} PROPERTIES COMPILE_FLAGS -fPIC)
  set_source_files_properties(${FilterWidgetsLib_SOURCE_DIR}/FilterWidgetCodeGen.h PROPERTIES COMPILE_FLAGS -fPIC)
  set_source_files_properties(${FilterWidgetsLib_SOURCE_DIR}/FilterWidgetCodeGen.cpp PROPERTIES COMPILE_FLAGS -fPIC)
endif()

add_executable(${PLUGIN_NAME}WidgetCodeGen  ${FilterWidgetsLib_SOURCE_DIR}/FilterWidgetCodeGen.cpp
                                            ${${PLUGIN_NAME}_BINARY_DIR}/FilterWidgetCodeGen.h
                                            ${${PLUGIN_NAME}_Project_SRCS}
                                            )
target_link_libraries(${PLUGIN_NAME}WidgetCodeGen MXA EbsdLib DREAM3DLib ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})


# Now run the code to generate the header files which will over write the place
# holder files that were generated from above
add_custom_command(TARGET ${PLUGIN_NAME}WidgetCodeGen POST_BUILD
                    COMMAND $<TARGET_FILE:${PLUGIN_NAME}WidgetCodeGen> )
# ******************************************************************************


add_definitions(-DDREAM3D_USE_QT)
if(MSVC)   # Do not generate manifests for the plugins - caused issues loading plugins
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
endif()


# --------------------------------------------------------------------
# These are the NON-Filter but Qt derived sources of the plugin. In case you have
# additional Qt source files that you need compiled.
set(R3DAnalytics_HDRS
  ${PROJECT_SOURCE_DIR}/R3DAnalyticsPlugin.h
)

set(R3DAnalytics_SRCS
 ${PROJECT_SOURCE_DIR}/R3DAnalyticsPlugin.cpp
)
cmp_IDE_SOURCE_PROPERTIES( "R3DAnalytics/" "${R3DAnalytics_HDRS}" "${R3DAnalytics_SRCS}" "0")


# --------------------------------------------------------------------
# This section is for a Custom GUI for a Filter. We need to add the Header, Source and UI file here so that
# they do not get auto generated during cmake and build time.
set(${PLUGIN_NAME}_CustomWidgets_HDRS
   ${PROJECT_SOURCE_DIR}/${PLUGIN_NAME}/${PLUGIN_NAME}FiltersWidgets/QImportR3DStackWidget.h
)

set(${PLUGIN_NAME}_CustomWidgets_SRCS
   ${PROJECT_SOURCE_DIR}/${PLUGIN_NAME}/${PLUGIN_NAME}FiltersWidgets/QImportR3DStackWidget.cpp
)

QT4_WRAP_UI( ${PLUGIN_NAME}_Generated_UI_HDRS
   ${PROJECT_SOURCE_DIR}/${PLUGIN_NAME}/${PLUGIN_NAME}FiltersWidgets/QImportR3DStackWidget.ui
)

# --------------------------------------------------------------------
# We need to run Qt's moc system on our Qt based files (Anything deriving from QObject)
# This cmake code sets up moc to be run.
QT4_WRAP_CPP( ${PLUGIN_NAME}_Generated_MOC_SRCS ${${PLUGIN_NAME}Widget_GEN_HDRS} )
QT4_WRAP_CPP( ${PLUGIN_NAME}_MOC_SRCS ${${PLUGIN_NAME}_HDRS} ${${PLUGIN_NAME}_CustomWidgets_HDRS})

# These generated moc files will be #include in the FilterWidget source file that
# are generated so we need to tell the build system to NOT compile these files
set_source_files_properties( ${${PLUGIN_NAME}_Generated_MOC_SRCS} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties( ${${PLUGIN_NAME}_MOC_SRCS} PROPERTIES HEADER_FILE_ONLY TRUE)

# --------------------------------------------------------------------
# -- Create a Qt Resource header file of the documentation html files
#QT4_ADD_RESOURCES( ${PLUGIN_NAME}_Generated_RC_SRCS "${HtmlDocQrcFile}"  )

# --------------------------------------------------------------------
# Group all the generated source files in IDE's that support it. Makes it clear
# to the user of the IDE what is generated and what is not.
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Moc" "${${PLUGIN_NAME}_GEN_HDRS}" "${${PLUGIN_NAME}_Generated_MOC_SRCS};${${PLUGIN_NAME}_MOC_SRCS};${${PLUGIN_NAME}_GEN_SRCS}" "0")
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Uic" "${${PLUGIN_NAME}_Generated_UI_HDRS}" "" "0")
cmp_IDE_SOURCE_PROPERTIES( "Generated/Qt_Qrc" "${${PLUGIN_NAME}_Generated_RC_SRCS}" "" "0")


# --------------------------------------------------------------------
# Create an install rule for the headers. If you do NOT want your headers
# installed for a deployment to other machines then change the PROJECT_INSTALL_HEADERS
# to a Zero
if( ${PROJECT_INSTALL_HEADERS} EQUAL 1 )
    INSTALL(FILES ${${PLUGIN_NAME}_HDRS} DESTINATION include/${PLUGIN_NAME}/${PLUGIN_NAME} COMPONENT Headers)
endif()


set(${PLUGIN_NAME}_Project_SRCS
   ${${PLUGIN_NAME}_Project_SRCS}
   ${${PLUGIN_NAME}_HDRS}
   ${${PLUGIN_NAME}_SRCS}
   ${${PLUGIN_NAME}_CustomWidgets_HDRS}
   ${${PLUGIN_NAME}_CustomWidgets_SRCS}
   ${${PLUGIN_NAME}_Generated_UI_HDRS}
   ${${PLUGIN_NAME}Widget_GEN_HDRS}
   ${${PLUGIN_NAME}Widget_GEN_SRCS}
   ${${PLUGIN_NAME}_Generated_MOC_SRCS}
   ${${PLUGIN_NAME}_MOC_SRCS}
   ${${PLUGIN_NAME}_Generated_RC_SRCS}
   )

# --------------------------------------------------------------------
# Setup output Directory for the plugin
if(NOT MSVC)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${DREAM3DProj_BINARY_DIR}/Bin/plugins  )
endif()

# --------------------------------------------------------------------
# Add the plugin as a MODULE
add_library(${PLUGIN_NAME} MODULE ${${PLUGIN_NAME}_Project_SRCS})

# --------------------------------------------------------------------
# Link the plugin with other DREAM3D Libraries.
target_link_libraries(${PLUGIN_NAME}
                    MXA
                    DREAM3DLib
                    ${QT_QTCORE_LIBRARY}
                    ${QT_QTGUI_LIBRARY}
                    QtSupport
                    PipelineBuilderLib
                    )

# --------------------------------------------------------------------
# This is a special section for this plugin which requires the Qt based
# image plugins so we need to make sure those are deployed with the Application
# Clear the file first otherwise we get duplicates
file(WRITE ${PROJECT_BINARY_DIR}/Qt_Plugins.txt "")
FindQt4Plugins("qgif;qjpeg;qtiff" "${DREAM3DProj_BINARY_DIR}/Qt_Plugins.txt"
                "${DREAM3DProj_BINARY_DIR}/LibSearchDirs.txt"
                "imageformats" )


# --------------------------------------------------------------------
# Add a dependency on the code generation step to make sure the code is
# generated before we try to compile the plugin
add_dependencies(${PLUGIN_NAME} ${PLUGIN_NAME}WidgetCodeGen)

# --------------------------------------------------------------------
# Set some additional properties of the plugin like its output name
PluginProperties(${PLUGIN_NAME} "_debug" "${DREAM3D_VERSION}"
                "${DREAM3D_BINARY_DIR}" "${CMP_PLUGIN_LIST_FILE}")

# --------------------------------------------------------------------
# Put back the output directory
if(NOT MSVC)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${DREAM3DProj_BINARY_DIR}/Bin  )
endif()

