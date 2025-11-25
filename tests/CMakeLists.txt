# Correct root!
set(TEST_ROOT "${CMAKE_SOURCE_DIR}/tests/vm")

# Find all test subdirectories
file(GLOB TEST_DIRS "${TEST_ROOT}/*")

enable_testing()


foreach(TEST_DIR ${TEST_DIRS})

    # Only actual directories
    if(IS_DIRECTORY "${TEST_DIR}")
        message("DOING ${TEST_DIR}")

        get_filename_component(TEST_NAME "${TEST_DIR}" NAME)

        set(INPUT_FILE "${TEST_DIR}/1.in")
        set(BC_FILE    "${TEST_DIR}/${TEST_NAME}.bc")
        set(CPP_FILE   "${TEST_DIR}/${TEST_NAME}.cpp")

        # Correct path to VM binary:
        set(VM "${CMAKE_BINARY_DIR}/bin/my_vm")

        # ------------- VM TEST -------------
        add_test(
            NAME test_${TEST_NAME}
            COMMAND ${CMAKE_COMMAND} -E env bash -c "${VM} run ${BC_FILE} < ${INPUT_FILE}"
            WORKING_DIRECTORY "${TEST_DIR}"
        )
        set_tests_properties(test_${TEST_NAME} PROPERTIES
            PASS_REGULAR_EXPRESSION ""
        )

        # ---------- OPTIONAL C++ TEST ----------
        if(EXISTS "${CPP_FILE}")

            add_executable(ref_${TEST_NAME} "${CPP_FILE}")

            add_test(
                NAME test_${TEST_NAME}_cpp
                COMMAND ${CMAKE_COMMAND} -E env bash -c "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ref_${TEST_NAME} < ${INPUT_FILE}"
                WORKING_DIRECTORY ${TEST_DIR}
            )

            # ---------- TIME COMPARISON ----------
            add_test(
                NAME time_compare_${TEST_NAME}
                COMMAND ${CMAKE_COMMAND}
                    -DTEST_NAME=${TEST_NAME}
                    -DVM_CMD=${VM}
                    -DBC_FILE=${BC_FILE}
                    -DCPP_EXEC=$<TARGET_FILE:ref_${TEST_NAME}>
                    -DINPUT_FILE=${INPUT_FILE}
                    -P ${CMAKE_SOURCE_DIR}/cmake/time_compare.cmake
            )
        endif()

    endif()

endforeach()
