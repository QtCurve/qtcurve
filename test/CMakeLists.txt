enable_testing()

add_definitions(-UNDEBUG)
remove_definitions(-DNDEBUG)

add_executable(test-version test-version.c)
target_link_libraries(test-version qtcurve-utils)
add_test(NAME test-version COMMAND test-version)

add_executable(test-buff test-buff.c)
target_link_libraries(test-buff qtcurve-utils)
add_test(NAME test-buff COMMAND test-buff)

add_executable(test-search test-search.c)
target_link_libraries(test-search qtcurve-utils)
add_test(NAME test-search COMMAND test-search)

add_executable(test-strmap test-strmap.c)
target_link_libraries(test-strmap qtcurve-utils)
add_test(NAME test-strmap COMMAND test-strmap)

add_executable(test-printf test-printf.c)
target_link_libraries(test-printf qtcurve-utils)
add_test(NAME test-printf COMMAND test-printf)
