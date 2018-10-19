INCLUDE(ExternalProject)

IF(WIN32)
	RETURN() # FIXME 'mruby' fails to build due to broken 'mruby-process'
ENDIF()

SET(EXTERNALPROJECT_CMAKE_ARGS
	"-DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}"
	"-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
)

IF(CMAKE_TOOLCHAIN_FILE)
	SET(EXTERNALPROJECT_CMAKE_ARGS
		"${EXTERNALPROJECT_CMAKE_ARGS}"
		"-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
	)
ENDIF()

IF(WIN32)
	SET(ZEST_BUILD_ENV "")
	SET(ZEST_BUILD_JOB "windows")
ELSEIF(APPLE)
	SET(ZEST_BUILD_ENV "OS=Mac")
	SET(ZEST_BUILD_JOB "osx")
ELSE()
	SET(ZEST_BUILD_ENV "")
	SET(ZEST_BUILD_JOB "all")
ENDIF()

ExternalProject_Add(mruby-zest-build
	SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/mruby-zest-build"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND env CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} LD=${CMAKE_CXX_COMPILER} "${ZEST_BUILD_ENV}" make ${ZEST_BUILD_JOB}
	INSTALL_COMMAND ""
	BUILD_IN_SOURCE ON # TODO can we work around this?
)

ExternalProject_Add_Step(mruby-zest-build
	prepare
	COMMAND make setup
	COMMAND make builddep
	DEPENDERS build
	WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/mruby-zest-build"
)

ExternalProject_Add_Step(mruby-zest-build
	package
	DEPENDEES build
	COMMAND make pack
	WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/mruby-zest-build"
)

ADD_CUSTOM_TARGET(bundle-mruby-zest-build
	COMMAND cmake -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/mruby-zest-build/package" "${CMAKE_CURRENT_BINARY_DIR}/.."
	DEPENDS mruby-zest-build
)

SET(CMAKE_PREFIX_PATH_ZYN "${CMAKE_PREFIX_PATH};${CMAKE_BINARY_DIR}/src/3rdparty/spa/target")
STRING(REPLACE ";" ":" CMAKE_PREFIX_PATH_ZYN "${CMAKE_PREFIX_PATH_ZYN}") # FIXME can be problematic

FILE(RELATIVE_PATH LIB_DIR_RELATIVE "/${BIN_DIR}" "/${LIB_DIR}")

ExternalProject_Add(zynaddsubfx-spa
	SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/zynaddsubfx"
	INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/zynaddsubfx/target"
	DEPENDS spa-build bundle-mruby-zest-build
	LIST_SEPARATOR ":"
	CMAKE_ARGS
		"${EXTERNALPROJECT_CMAKE_ARGS}"
		"-DAlsaEnable=OFF"
		"-DJackEnable=OFF"
		"-DOssEnable=OFF"
		"-DPaEnable=OFF"
		"-DGuiModule=zest"
		"-DZynFusionDir=${LIB_DIR_RELATIVE}"
		"-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ZYN}"
		"-DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/zynaddsubfx/target"
)

ADD_CUSTOM_TARGET(bundle-zynaddsubfx-spa
	COMMAND cmake -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/zynaddsubfx/target/lib/spa" "${CMAKE_CURRENT_BINARY_DIR}/.."
	DEPENDS zynaddsubfx-spa
)

