execute_process(
    COMMAND ${VM_CMD} run ${BC_FILE}
    INPUT_FILE ${INPUT_FILE}
    TIMEOUT 5
    RESULT_VARIABLE VM_RET
    OUTPUT_VARIABLE VM_OUT
    ERROR_VARIABLE VM_ERR
    TIME_OUTPUT_VARIABLE VM_TIME
)

execute_process(
    COMMAND ${CPP_EXEC}
    INPUT_FILE ${INPUT_FILE}
    TIMEOUT 5
    RESULT_VARIABLE CPP_RET
    OUTPUT_VARIABLE CPP_OUT
    ERROR_VARIABLE CPP_ERR
    TIME_OUTPUT_VARIABLE CPP_TIME
)

message("---- Time comparison for ${TEST_NAME} ----")
message("VM:   ${VM_TIME}s")
message("C++:  ${CPP_TIME}s")

if(VM_RET)
    message(FATAL_ERROR "VM failed")
endif()

if(CPP_RET)
    message(FATAL_ERROR "C++ reference failed")
endif()

if(NOT "${VM_OUT}" STREQUAL "${CPP_OUT}")
    message(FATAL_ERROR "Output mismatch!")
endif()
