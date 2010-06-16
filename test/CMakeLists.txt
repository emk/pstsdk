file(GLOB sources *.cpp)
add_executable(pstsdk_test ${sources})
add_test(NAME pstsdk_test COMMAND pstsdk_test)
