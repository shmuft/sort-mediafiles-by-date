find_program(GIT NAMES git)
macro(git_execute OUTPUT_VARIABLE)
    execute_process(
        COMMAND "${GIT}" ${ARGN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_VARIABLE ${OUTPUT_VARIABLE}
        ERROR_VARIABLE ${OUTPUT_VARIABLE}
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if (NOT ${GIT_RESULT} EQUAL 0)
        message(FATAL ERROR "Git execution failed with code: ${GIT_RESULT}. Git output: ${OUTPUT_VARIABLE}")
    endif ()
endmacro()

git_execute(PROJECT_SVN_VERSION rev-list HEAD --count)

set(PROJECT_PROGRAM_VERSION "0.1.${PROJECT_SVN_VERSION}")
