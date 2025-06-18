cmake_minimum_required(VERSION 3.0)
project(chat)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置最终的可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/thridparty)
# 加载子目录
add_subdirectory(src)













# # 配置头文件搜索路径
# # include_directories()
# # 配置库文件搜索路径
# # link_directories()

# # 设置需要编译的源文件列表
# set(SRC_LIST muduo_server.cpp)

# # 将.指定路径下的所有源文件名字放到变量名SRC_LIST里面
# aux_source_directory(. SRC_LIST)

# # 设置可执行文件最终存储的路径----PROJECT_SOURCE_DIR工程的根目录
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# # 生成执行文件
# add_executable(server muduo_server.cpp)

# # 需要连接的库文件,server表示目标程序
# target_link_libraries(server muduo_net muduo_base pthread)