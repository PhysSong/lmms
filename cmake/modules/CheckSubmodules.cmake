# Utility for validating and, if needed, cloning all submodules
#
# Looks for a .gitmodules in the root project folder
# Loops over all modules looking well-known configure/build scripts
#
# Usage:
#       INCLUDE(CheckSubmodules)
#
# Options:
#       SET(SKIP_SUBMODULES "foo;bar")
#
# Or via command line:       
#       cmake -DSKIP_SUBMODULES=foo;bar
#
# Copyright (c) 2017, Tres Finocchiaro, <tres.finocchiaro@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Files which confirm a successful clone
SET(VALID_CRUMBS "CMakeLists.txt;Makefile;Makefile.in;Makefile.am;configure.ac;configure.py;autogen.sh;.gitignore;LICENSE;Home.md")

# Number of times git commands will retry before failing
SET(MAX_ATTEMPTS 2)

SET(MISSING_COMMIT_PHRASES "no such remote ref;reference is not a tree;unadvertised object")
SET(RETRY_PHRASES "Failed to recurse;cannot create directory;already exists;not a git repository;${MISSING_COMMIT_PHRASES}")


# Force English locale
SET(LC_ALL_BACKUP "$ENV{LC_ALL}")
SET(LANG_BACKUP "$ENV{LANG}")
SET(ENV{LC_ALL} "C")
SET(ENV{LANG} "en_US")


# Once called, status is stored in GIT_RESULT respectively.
# Note: Git likes to write to stderr.  Don't assume stderr is error; Check GIT_RESULT instead.
MACRO(GIT_SUBMODULE_TRY WORKING_DIRECTORY SUBMODULE_PATH)
	FIND_PACKAGE(Git REQUIRED)

	# Try to use the depth switch
	# Shallow submodules were introduced in 1.8.4
	IF(GIT_VERSION_STRING VERSION_GREATER "1.8.3" AND DEPTH_VALUE)
		SET(DEPTH_CMD "--depth=${DEPTH_VALUE}")
		MESSAGE("--   Fetching ${SUBMODULE_PATH} @ ${DEPTH_CMD}")
	ELSE()
		SET(DEPTH_CMD "")
		MESSAGE("--   Fetching ${SUBMODULE_PATH}")
	ENDIF()

	# Don't use --recursive here; Try that later.
	EXECUTE_PROCESS(
		COMMAND ${GIT_EXECUTABLE} submodule update --init ${DEPTH_CMD} ${WORKING_DIRECTORY}/${SUBMODULE_PATH}
		WORKING_DIRECTORY ${WORKING_DIRECTORY}
		RESULT_VARIABLE GIT_RESULT
		OUTPUT_VARIABLE GIT_STDOUT
		ERROR_VARIABLE GIT_STDERR
	)

	SET(GIT_MESSAGE "${GIT_STDOUT}${GIT_STDERR}")
	MESSAGE("${GIT_MESSAGE}")
ENDMACRO()


MACRO(GIT_SUBMODULE_FIX_REMOTE WORKING_DIRECTORY SUBMODULE_PATH FORCE_REMOTE)
	FIND_PACKAGE(Git REQUIRED)
	MESSAGE("--   Adding and fetching remote submodulefix to ${SUBMODULE_PATH}")

	# Try unshallowing to fix missing commits.
	# --unshallow was introduced in 1.8.2; Use '--depth=2147483647' for old versions
	IF(GIT_VERSION_STRING VERSION_GREATER "1.8.1")
		SET(UNSHALLOW_CMD "--unshallow")
	ELSE()
		SET(UNSHALLOW_CMD "--depth=2147483647")
	ENDIF()

	EXECUTE_PROCESS(
		COMMAND ${GIT_EXECUTABLE} remote rm submodulefix
		COMMAND ${GIT_EXECUTABLE} remote add submodulefix ${FORCE_REMOTE}
		COMMAND ${GIT_EXECUTABLE} fetch submodulefix ${UNSHALLOW_CMD}
		WORKING_DIRECTORY ${WORKING_DIRECTORY}/${SUBMODULE_PATH}
		OUTPUT_QUIET ERROR_QUIET
	)
ENDMACRO()


MACRO(GIT_SUBMODULE_RESET WORKING_DIRECTORY SUBMODULE_PATH)
	FIND_PACKAGE(Git REQUIRED)
	MESSAGE("--   Resetting ${SUBMODULE_PATH}")
	EXECUTE_PROCESS(
		COMMAND ${GIT_EXECUTABLE} submodule deinit -f ${WORKING_DIRECTORY}/${SUBMODULE_PATH}
		WORKING_DIRECTORY ${WORKING_DIRECTORY}
		OUTPUT_QUIET
	)
ENDMACRO()


FUNCTION(GIT_SUBMODULE WORKING_DIRECTORY)
	IF(NOT EXISTS "${WORKING_DIRECTORY}/.gitmodules")
		RETURN()
	ENDIF()
	FILE(READ "${WORKING_DIRECTORY}/.gitmodules" SUBMODULE_DATA)

	# Assume alpha-numeric paths
	STRING(REGEX MATCHALL "path = [-0-9A-Za-z/]+" SUBMODULE_LIST ${SUBMODULE_DATA})
	STRING(REGEX MATCHALL "url = [.:%-0-9A-Za-z/]+" SUBMODULE_URL_LIST ${SUBMODULE_DATA})

	FOREACH(_part ${SUBMODULE_LIST})
		STRING(REPLACE "path = " "" SUBMODULE_PATH ${_part})

		LIST(FIND SUBMODULE_LIST ${_part} SUBMODULE_INDEX)
		LIST(GET SUBMODULE_URL_LIST ${SUBMODULE_INDEX} _url)
		STRING(REPLACE "url = " "" SUBMODULE_URL ${_url})

		# Remove submodules from validation as specified in -DSKIP_SUBMODULES=foo;bar
		SET(SKIP false)
		IF(SKIP_SUBMODULES)
			FOREACH(_skip ${SKIP_SUBMODULES})
				IF(${SUBMODULE_PATH} MATCHES ${_skip})
					MESSAGE("-- Skipping ${SUBMODULE_PATH} matches \"${_skip}\"")
					SET(SKIP true)
				ENDIF()
			ENDFOREACH()
		ENDIF()
		IF(NOT SKIP)
			LIST(INSERT SUBMODULE_LIST ${SUBMODULE_INDEX} ${SUBMODULE_PATH})
			LIST(INSERT SUBMODULE_URL_LIST ${SUBMODULE_INDEX} ${SUBMODULE_URL})
		ENDIF()
		LIST(REMOVE_ITEM SUBMODULE_LIST ${_part})
		LIST(REMOVE_ITEM SUBMODULE_URL_LIST ${_url})
	ENDFOREACH()

	# Attempt to do lazy clone
	FOREACH(_submodule ${SUBMODULE_LIST})
		STRING(REPLACE "/" ";" PATH_PARTS ${_submodule})
		LIST(REVERSE PATH_PARTS)
		LIST(GET PATH_PARTS 0 SUBMODULE_NAME)

		MESSAGE("-- Checking ${SUBMODULE_NAME}...")
		SET(CRUMB_FOUND false)
		FOREACH(_crumb ${VALID_CRUMBS})
			IF(EXISTS "${WORKING_DIRECTORY}/${_submodule}/${_crumb}")
				SET(CRUMB_FOUND true)
				MESSAGE("--   Found ${_submodule}/${_crumb}")
				BREAK()
			ENDIF()
		ENDFOREACH()
		IF(NOT CRUMB_FOUND)
			SET(DEPTH_VALUE 100)
			GIT_SUBMODULE_TRY(${WORKING_DIRECTORY} ${_submodule})

			SET(COUNTED 0)
			SET(COUNTING "")
			# Handle edge-cases where submodule didn't clone properly or re-uses a non-empty directory
			WHILE(NOT GIT_RESULT EQUAL 0 AND COUNTED LESS MAX_ATTEMPTS)
				LIST(APPEND COUNTING "x")
				LIST(LENGTH COUNTING COUNTED)
				SET(MISSING_COMMIT false)

				# Shallow commits can fail to clone from non-default branches, only try once
				IF(COUNTED LESS 2)
					# Try a shallow submodule clone
				ELSE()
					UNSET(DEPTH_VALUE)
				ENDIF()

				FOREACH(_phrase ${MISSING_COMMIT_PHRASES})
					IF("${GIT_MESSAGE}" MATCHES "${_phrase}")
						SET(MISSING_COMMIT true)
						BREAK()
					ENDIF()
				ENDFOREACH()
				FOREACH(_phrase ${RETRY_PHRASES})
					IF(${MISSING_COMMIT})
						LIST(FIND SUBMODULE_LIST ${_submodule} SUBMODULE_INDEX)
						LIST(GET SUBMODULE_URL_LIST ${SUBMODULE_INDEX} SUBMODULE_URL)
						MESSAGE("--   Retrying ${_submodule} using 'remote add submodulefix' (attempt ${COUNTED} of ${MAX_ATTEMPTS})...")

						GIT_SUBMODULE_FIX_REMOTE(${WORKING_DIRECTORY} ${_submodule} ${SUBMODULE_URL})
						BREAK()
					ELSEIF("${GIT_MESSAGE}" MATCHES "${_phrase}")
						MESSAGE("--   Retrying ${_submodule} using 'deinit' (attempt ${COUNTED} of ${MAX_ATTEMPTS})...")

						GIT_SUBMODULE_RESET(${WORKING_DIRECTORY} ${_submodule})
						BREAK()
					ENDIF()
				ENDFOREACH()
				GIT_SUBMODULE_TRY(${WORKING_DIRECTORY} ${_submodule})
			ENDWHILE()

			IF(NOT GIT_RESULT EQUAL 0)
				MESSAGE(FATAL_ERROR "${GIT_EXECUTABLE} exited with status of ${GIT_RESULT}")
			ENDIF()
		ENDIF()
	GIT_SUBMODULE(${WORKING_DIRECTORY}/${_submodule})
	ENDFOREACH()
ENDFUNCTION()


MESSAGE("\nValidating submodules...")
GIT_SUBMODULE("${CMAKE_SOURCE_DIR}")
MESSAGE("-- Done validating submodules.\n")

# Reset locale
SET(ENV{LC_ALL} "${LC_ALL_BACKUP}")
SET(ENV{LANG} "${LANG_BACKUP}")
