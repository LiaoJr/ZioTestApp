cmake_minimum_required(VERSION 3.22)

project(ZioTestApp)

add_subdirectory(application)
add_subdirectory(ecat_master)
add_subdirectory(upgrade)
add_subdirectory(utils)

add_executable(MasterOfZio)

# 添加线程库查找（在project之后添加）
# find_package(Threads REQUIRED)
target_link_options(MasterOfZio PRIVATE -lpthread -lrt -ldl -lstdc++ -lm)
target_link_directories(MasterOfZio PRIVATE lib)
target_link_libraries(MasterOfZio PRIVATE 
                        app_zmcio
                        cmd_process
                        data_log
                        main
                        modbus_tcp_server
                        ecat_master
                        eeprom
                        slave
                        app_upgrade
                        log
                        ticks
                        zecm
                        ${CMAKE_CURRENT_SOURCE_DIR}/lib/libmodbus.a)

# cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="D:\\Tools\\ToolChains\\aarch64-linux\\bin\\aarch64-linux-gnu.cmake" ..
# cmake -DCMAKE_TOOLCHAIN_FILE="/home/ljw/tools/Toolchains/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu.cmake" ..
