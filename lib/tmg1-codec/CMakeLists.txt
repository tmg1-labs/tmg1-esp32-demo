cmake_minimum_required(VERSION 3.16)
project(tmg1-codec CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- ライブラリ本体 ---

add_library(tmg1codec
    src/rice_reader.cpp
    src/rice_writer.cpp
    src/range_decoder.cpp
    src/range_encoder.cpp
    src/prediction.cpp
    src/decoder.cpp
    src/encoder.cpp
)

target_include_directories(tmg1codec PUBLIC include)

# C API ラッパー (Rust FFI 用)
add_library(tmg1codec_c
    c_api/tmg1_c.cpp
)
target_link_libraries(tmg1codec_c PUBLIC tmg1codec)
target_include_directories(tmg1codec_c PUBLIC include c_api)

# --- テスト ---

enable_testing()

# Unity テストフレームワーク
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vendor/unity/src/unity.c")
    add_library(unity STATIC vendor/unity/src/unity.c)
    target_include_directories(unity PUBLIC vendor/unity/src)
else()
    find_path(UNITY_INCLUDE unity.h)
    find_library(UNITY_LIB unity)
    if(UNITY_INCLUDE AND UNITY_LIB)
        add_library(unity IMPORTED STATIC)
        set_target_properties(unity PROPERTIES
            IMPORTED_LOCATION ${UNITY_LIB}
            INTERFACE_INCLUDE_DIRECTORIES ${UNITY_INCLUDE}
        )
    else()
        message(WARNING "Unity not found. Tests will be skipped.")
        return()
    endif()
endif()

function(add_unity_test name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} tmg1codec unity)
    target_include_directories(${name} PRIVATE include)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_unity_test(test_rice       test/test_rice.cpp)
add_unity_test(test_range      test/test_range.cpp)
add_unity_test(test_prediction test/test_prediction.cpp)
add_unity_test(test_codec      test/test_codec.cpp)
