include_directories(${GTEST_DIR})
include_directories(${GTEST_DIR}/src)
include_directories(${GTEST_DIR}/include)
include_directories(${GTEST_DIR}/make)

ADD_DEFINITIONS(
	-DGTEST=true
)

set(APP_SRC 
  ${GTEST_DIR}/src/gtest-all.cc
	${GTEST_DIR}/src/gtest_main.cc
)

add_executable(client_test ${APP_SRC} client_test.cc)
target_link_libraries(client_test client ${SYS_LIBS}) 

add_executable(params_test ${APP_SRC} params_test.cc)
target_link_libraries(params_test client ${SYS_LIBS}) 


