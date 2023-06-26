# unit test cmake

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DFMA_IN_UNIT_TEST -DNO_STACKTRACE")

option(ENABLE_ASAN "Enable asan." OFF)
if (ENABLE_ASAN)
    message("Asan is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -static-libasan -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -static-libasan ")
else (ENABLE_ASAN)
    message("Asan is disabled.")
endif (ENABLE_ASAN)

option(ENABLE_SNAPPY "Enable snappy." OFF)
if (ENABLE_SNAPPY)
    message("Snappy is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_SNAPPY=1")
    include_directories(${ROOT_DIR}/snappy)
    link_libraries(snappy)
    link_directories(${ROOT_DIR}/snappy)
else (ENABLE_SNAPPY)
    message("Snappy is disabled.")
endif (ENABLE_SNAPPY)

option(ENABLE_HDFS "Enable HDFS." OFF)
if (ENABLE_HDFS)
    message("HDFS is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_LIBHDFS=1")
    include_directories(${ROOT_DIR}/libhdfs)
    link_libraries(hdfs pthread jvm)
    link_directories(${ROOT_DIR}/libhdfs ${JAVA_HOME}/jre/lib/amd64/server)
else (ENABLE_HDFS)
    message("HDFS is disabled.")
endif (ENABLE_HDFS)

option(ENABLE_HDFS3 "Enable HDFS3." OFF)
if (ENABLE_HDFS3)
    message("HDFS3 is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_LIBHDFS3=1 -D_GLIBCXX_USE_CXX11_ABI=0")
    include_directories(${ROOT_DIR}/libhdfs3)
    link_libraries(hdfs3 pthread jvm lxml2 krb5 gsasl uuid protobuf)
else (ENABLE_HDFS3)
    message("HDFS3 is disabled.")
endif (ENABLE_HDFS3)

option(ENABLE_MYSQL "Enable MYSQL." OFF)
if (ENABLE_MYSQL)
    message("MYSQL is enabled.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFMA_HAS_LIBMYSQL=1")
    link_libraries(mysqlcppconn)
else (ENABLE_MYSQL)
    message("MYSQL is disabled.")
endif (ENABLE_MYSQL)

add_executable(fma_unit_test
        main.cpp
        test_arg_parse.cpp
        test_binary_read_write_helper.cpp
        test_bounded_queue.cpp
        test_cache_aligned_vector.cpp
        test_configuration.cpp
        test_data_file.cpp
        test_date.cpp
        test_encrypt.cpp
        test_file_stream.cpp
        test_file_system.cpp
        test_fma_stream.cpp
        test_hard_container.cpp
        test_hardware_info.cpp
        test_leveled_log_device.cpp
        test_logging.cpp
        test_many_streams.cpp
        test_multi_disk_stream.cpp
        test_piped_hdfs_stream.cpp
        test_pipeline.cpp
        test_rotating_file_log_device.cpp
        test_rotating_files.cpp
        test_rw_lock.cpp
        test_snappy_stream.cpp
        test_stream_buffer.cpp
        test_string_util.cpp
        test_text_dir_stream.cpp
        test_text_parser.cpp
        test_text_parser_utils.cpp
        test_text_writer.cpp
        test_thread_pool.cpp
        test_timed_task.cpp
        test_type_traits.cpp
        test_utils.cpp
        test_level_log.cpp)
