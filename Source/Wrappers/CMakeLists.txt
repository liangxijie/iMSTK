# \todo: Wrapper generation would make more sense in the superbuild above one level. For now it is here
# \todo: A different c wrapper is required to be generated per language
set(SWIG_INTERFACE_DIR ${CMAKE_CURRENT_LIST_DIR}/SwigInterface)
set(SWIG_CSHARP_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/iMSTKCSharpWrapper/src")

# include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include(UseSWIG)
set_source_files_properties(${SWIG_INTERFACE_DIR}/imstkCWrapper.i PROPERTIES CPLUSPLUS ON)

swig_add_library(iMSTKCWrapper
    TYPE SHARED
    OUTPUT_DIR ${SWIG_CSHARP_OUTPUT_DIR}
    LANGUAGE csharp
    SOURCES
    ${SWIG_INTERFACE_DIR}/imstkCWrapper.i)
# Always clear the ".cs" files from the build directory before generating
add_custom_command(
    TARGET iMSTKCWrapper
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -Ddir:PATH=${SWIG_CSHARP_OUTPUT_DIR}/ -P ${CMAKE_CURRENT_LIST_DIR}/ClearDirectory.cmake
    DEPENDS ${PROJECT_NAME})

# These files should not be provided via swig_add_library SOURCES
# But still useful to display in visual studios
if (MSVC) 
    set(OTHER_FILES
		${SWIG_INTERFACE_DIR}/callback.i
		${SWIG_INTERFACE_DIR}/common.i
		${SWIG_INTERFACE_DIR}/debug.i
		${SWIG_INTERFACE_DIR}/ignored.i
		${SWIG_INTERFACE_DIR}/modifiers.i
		${SWIG_INTERFACE_DIR}/shared_ptr_instantiation.i
		${SWIG_INTERFACE_DIR}/std_function.i
		${SWIG_INTERFACE_DIR}/type_cast.i
		${SWIG_INTERFACE_DIR}/weak_ptr.i)

    # \todo: I think there's some property we can use to indicate to visual studios
    # that it should recompile if one of them changes, this must internally
    # happen in swig_add_library somewhere
    target_sources(iMSTKCWrapper PRIVATE ${OTHER_FILES})
    set_source_files_properties(${OTHER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

set_target_properties(iMSTKCWrapper PROPERTIES
    SWIG_COMPILE_OPTIONS "-namespace;Imstk"
    FOLDER Wrappers)
if(iMSTK_SWIG_PINNED_ARRAY)
    set_target_properties(iMSTKCWrapper PROPERTIES
        SWIG_COMPILE_DEFINITIONS "SWIG_PINNED_ARRAY")
endif()
if(iMSTK_USE_OpenHaptics)
    set_target_properties(iMSTKCWrapper PROPERTIES
        SWIG_COMPILE_DEFINITIONS "iMSTK_USE_OpenHaptics")
	target_compile_definitions(iMSTKCWrapper PRIVATE iMSTK_USE_OpenHaptics)
endif()

# Link C Wrapper to the imstk libraries
target_link_libraries(iMSTKCWrapper PRIVATE
    Common
    Materials
	Geometry
	DataStructures
	Devices
	Filtering
	FilteringCore
	Materials
	RenderingCore
    RenderingVTK
	Solvers
	DynamicalModels
	CollisionDetection
	CollisionHandling
	SceneEntities
	Scene
	SimulationManager
    ViewerCore
    ViewerVTK
	Constraints
	Animation)
target_compile_options(iMSTKCWrapper PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wno-unused-function>
    $<$<CXX_COMPILER_ID:MSVC>:
        -W4 -MP -wd4505 /bigobj>)

# Install the c wrapper
install( TARGETS iMSTKCWrapper EXPORT iMSTKCWrapperTargets
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries
    LIBRARY DESTINATION lib COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib COMPONENT Development)

if (UNIX AND NOT APPLE)
	# Why use CODE and then execute COMMAND?
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink iMSTKCWrapper.so ${CMAKE_INSTALL_PREFIX}/lib/libiMSTKCWrapper.so)")
endif()

# Export the target file for the config file to use later			
export(EXPORT iMSTKCWrapperTargets
	FILE ${CMAKE_CURRENT_BINARY_DIR}/iMSTKCWrapperTargets.cmake)

configure_file(iMSTKCWrapperConfig.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/iMSTKCWrapperConfig.cmake"
	@ONLY)

# If we're on msvc we will generate a library/.dll out of our generated C# files
if (MSVC)
    # Because the files aren't generated until build time of the iMSTKCWrapper target
    # we must wait to configure it until after it's done building
    include(ExternalProject)
    ExternalProject_Add(iMSTKSharp
        DEPENDS
            iMSTKCWrapper
        DOWNLOAD_COMMAND ""
        PREFIX "${CMAKE_CURRENT_BINARY_DIR}/iMSTKCSharpWrapper"
        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/iMSTKCSharpWrapper"
        BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/iMSTKCSharpWrapper/build"
        STAMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/iMSTKCSharpWrapper/stamp"
        TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/iMSTKCSharpWrapper/tmp"
        INSTALL_DIR "install"
        CMAKE_CACHE_ARGS
            -DiMSTKCWrapper_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
            -DSWIG_OUTPUT_DIR:PATH=${SWIG_CSHARP_OUTPUT_DIR}
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_PREFIX_PATH})
    set_target_properties (iMSTKSharp PROPERTIES
        FOLDER Wrappers)
endif()

# Clear the ".cs" wrapper code in the install directory and install new ones
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -Dsource_dir:PATH=${SWIG_CSHARP_OUTPUT_DIR} -Ddest_dir:PATH=${CMAKE_INSTALL_PREFIX}/include/iMSTKSharp/ -P ${CMAKE_CURRENT_LIST_DIR}/ClearInstall.cmake)")