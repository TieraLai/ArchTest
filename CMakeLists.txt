cmake_minimum_required(VERSION 3.19.6)
project(PCI_Scanner VERSION 1.16.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(ENABLE_RT "Enable Real Time." ON)

if(ENABLE_RT)
 # 設置 Xenomai 的安裝目錄，根據你的安裝路徑修改
set(XENOMAI_DIR /usr/xenomai)
set(XENO_CONFIG "/usr/xenomai/bin/xeno-config")
set(XENO_CFLAGS_PARAMS "--posix" "--alchemy" "--cflags")
set(XENO_LDFLAGS_PARAMS "--posix" "--alchemy" "--ldflags")

# 使用 xeno-config 指令來取得 CFLAGS 和 LDFLAGS
#execute_process(COMMAND /usr/xenomai/bin/xeno-config --posix --alchemy --cflags
execute_process(COMMAND ${XENO_CONFIG} ${XENO_CFLAGS_PARAMS}
                OUTPUT_VARIABLE CFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

#execute_process(COMMAND /usr/xenomai/bin/xeno-config --posix --alchemy --ldflags
execute_process(COMMAND ${XENO_CONFIG} ${XENO_LDFLAGS_PARAMS}
                OUTPUT_VARIABLE LDFLAGS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

message("CFLAGS : ${CFLAGS}")
message("LDFLAGS : ${LDFLAGS}")

# 設定編譯標誌
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CFLAGS}")
# 設定連結標誌
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LDFLAGS}")

message("CMAKE_C_FLAGS : ${CMAKE_C_FLAGS}")
message("CMAKE_CXX_FLAGS : ${CMAKE_CXX_FLAGS}")
message("CMAKE_EXE_LINKER_FLAGS : ${CMAKE_EXE_LINKER_FLAGS}")

endif()


add_subdirectory(PF_NexRTOS)
add_subdirectory(Ecm)
add_subdirectory(User)
