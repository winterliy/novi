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

function(assert_semantic_failure name expected_message)
    execute_process(
        COMMAND "${NOVI_EXECUTABLE}" "${TEST_CASE_DIR}/${name}.nvi"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE actual
        ERROR_VARIABLE error_output
    )
    if(result EQUAL 0)
        message(FATAL_ERROR "${name} unexpectedly succeeded")
    endif()
    if(NOT actual STREQUAL "")
        message(FATAL_ERROR "${name} executed output before rejecting the program: [${actual}]")
    endif()
    if(NOT error_output MATCHES "${expected_message}")
        message(FATAL_ERROR "${name} diagnostic mismatch: [${error_output}]")
    endif()
endfunction()

foreach(name IN ITEMS tuple_equality finally_loop_local finally_continue finally_break finally_return
        finally_throw catch_finally catch_scope string_ascii_mixed function_return dynamic_declaration lambda_recursion catch_continue catch_break_finally returned_closure compound_assignment compound_index instance_property class_fields class_private class_static_methods)
    assert_case("${name}")
endforeach()

assert_semantic_failure(duplicate_parameter "Duplicate parameter 'value'")
assert_semantic_failure(duplicate_lambda_parameter "Duplicate parameter 'value'")
assert_semantic_failure(return_outside "return is only valid inside a function")
assert_semantic_failure(undefined_name "Undefined name 'missing_name'")
assert_semantic_failure(catch_out_of_scope "Undefined name 'caught_error'")
assert_semantic_failure(class_duplicate_field "Duplicate class field 'count'")
assert_semantic_failure(class_unsupported_field "class fields are not yet supported")
assert_semantic_failure(compound_constant "Cannot reassign constant 'count'")
assert_semantic_failure(constant_reassignment "Cannot reassign constant 'limit'")
assert_semantic_failure(fixed_type_reassignment "does not match fixed type 'string'")
assert_semantic_failure(fixed_type_initializer "does not match fixed type 'string'")

function(assert_semantic_success name expected)
    execute_process(
        COMMAND "${NOVI_EXECUTABLE}" "${TEST_CASE_DIR}/${name}.nvi"
        RESULT_VARIABLE result
        OUTPUT_VARIABLE actual
        ERROR_VARIABLE error_output
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${name} exited ${result}: ${error_output}")
    endif()
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${name} output mismatch. Expected [${expected}], got [${actual}]")
    endif()
endfunction()

assert_semantic_success(undefined_export "")

execute_process(
    COMMAND "${NOVI_EXECUTABLE}" "${TEST_CASE_DIR}/function_arity.nvi"
    RESULT_VARIABLE arity_result OUTPUT_VARIABLE arity_output ERROR_VARIABLE arity_error
)
string(FIND "${arity_error}" "expects 2 argument(s), got 1" arity_message_index)
if(arity_result EQUAL 0 OR arity_message_index EQUAL -1)
    message(FATAL_ERROR "function_arity failed to report the expected call error: [${arity_error}]")
endif()

execute_process(COMMAND "${NOVI_EXECUTABLE}" "${TEST_CASE_DIR}/class_field_type.nvi"
    RESULT_VARIABLE field_result ERROR_VARIABLE field_error)
string(FIND "${field_error}" "Assigned value does not match fixed type 'integer'" field_message_index)
if(field_result EQUAL 0 OR field_message_index EQUAL -1)
    message(FATAL_ERROR "class_field_type did not reject the wrong type: [${field_error}]")
endif()

execute_process(COMMAND "${NOVI_EXECUTABLE}" "${TEST_CASE_DIR}/class_constant_field.nvi"
    RESULT_VARIABLE constant_result OUTPUT_VARIABLE constant_output ERROR_VARIABLE constant_error)
string(FIND "${constant_error}" "Cannot reassign constant field 'count'" constant_message_index)
if(constant_result EQUAL 0 OR constant_message_index EQUAL -1 OR NOT constant_output STREQUAL "5\n5\n")
    message(FATAL_ERROR "class_constant_field did not reject reassignment: [${constant_error}]")
endif()

foreach(private_case IN ITEMS class_private_read class_private_write class_private_method)
    execute_process(COMMAND "${NOVI_EXECUTABLE}" "${TEST_CASE_DIR}/${private_case}.nvi"
        RESULT_VARIABLE private_result ERROR_VARIABLE private_error)
    if(private_result EQUAL 0 OR NOT private_error MATCHES "is not accessible")
        message(FATAL_ERROR "${private_case} did not reject external access: [${private_error}]")
    endif()
endforeach()

execute_process(COMMAND "${NOVI_EXECUTABLE}" "${TEST_CASE_DIR}/class_private_static_method.nvi"
    RESULT_VARIABLE private_static_result ERROR_VARIABLE private_static_error)
if(private_static_result EQUAL 0 OR NOT private_static_error MATCHES "Private static method 'secret' is not accessible")
    message(FATAL_ERROR "class_private_static_method did not reject external access: [${private_static_error}]")
endif()
