function(assert_case name)
    execute_process(
        COMMAND "${NOVI_EXECUTABLE}" "${TEST_CASE_DIR}/${name}.nvi"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE actual
        ERROR_VARIABLE error_output
    )
    file(READ "${TEST_CASE_DIR}/${name}.out" expected)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${name} exited ${result}: ${error_output}")
    endif()
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${name} output mismatch. Expected [${expected}], got [${actual}]")
    endif()
endfunction()

foreach(name IN ITEMS tuple_equality finally_loop_local finally_continue finally_break finally_return
        finally_throw catch_finally catch_scope)
    assert_case("${name}")
endforeach()
