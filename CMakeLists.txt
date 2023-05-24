cmake_minimum_required(VERSION 3.0.0)

project(sdk)

# set (CMAKE_C_COMPILER "/usr/bin/gcc")
# set (CMAKE_CXX_COMPILER "/usr/bin/g++")

file(GLOB SRC_DIR
    ./src/*.c)

file(GLOB APP_DIR
    ./app/*.c)

file(GLOB MY_DIR
    ./my/*.c)

set(INC_DIR
    ./inc)

set(GTK_DIR 
    /usr/include/gtk-3.0)

set(ZLOG_DIR ${PROJECT_SOURCE_DIR}/inc/zlog)

# if(${SDK})
#     message(${SDK})
# endif()

include_directories(${INC_DIR})
include_directories(${GTK_DIR} ${ZLOG_DIR})

# LINK_LIBRARIES(-lm -lpthread )
LINK_LIBRARIES(-lm -lpthread -lzlog)
# include_directories(/home/ubuntu/EC20/EC20/sysroots/armv7a-vfp-neon-oe-linux-gnueabi/usr/include)
if(${SDK})
    message("start build sdk")
    add_library (sdk_shared SHARED ${SRC_DIR})
    add_library (sdk_static STATIC ${SRC_DIR} ${INC_DIR})

    set_target_properties (sdk_shared PROPERTIES OUTPUT_NAME "mysdk")
    set_target_properties (sdk_static PROPERTIES OUTPUT_NAME "mysdk")

    SET_TARGET_PROPERTIES(sdk_shared PROPERTIES VERSION 1.1 SOVERSION 1)
    SET_TARGET_PROPERTIES(sdk_static PROPERTIES VERSION 1.2 SOVERSION 1)

    set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
    message("end build sdk")
endif()

if(${APP})
    message("start build app")
    #find lib
    #mysdk
    find_library(SDK_LIB libmysdk.a ${PROJECT_SOURCE_DIR}/lib)

    #gtk3
    find_package (PkgConfig REQUIRED)
    pkg_check_modules (GTK3 REQUIRED gtk+-3.0)
    # set(CMAKE_C_STANDARD 11)
    include_directories (${GTK3_INCLUDE_DIRS} ${INC_DIR})
    link_directories (${GTK3_LIBRARY_DIRS} ${INC_DIR}) 

    #zlog
    # find_library(ZLOG_LIB libzlog.a ${PROJECT_SOURCE_DIR}/lib/zlog)

    add_executable (sdk_app ${APP_DIR} ${MY_DIR})

    add_definitions (${GTK3_CFLAGS_OTHER})

    target_link_libraries (sdk_app ${SDK_LIB} ${GTK3_LIBRARIES})
    # target_link_libraries (main ${SDK_LIB} ${GTK3_LIBRARIES} ${ZLOG_LIB})

    set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
    message("end build app")
endif()

if(${CDCS})
    message("start build cdcs")
    file(GLOB CDCS_DIR
    ./cdcs/*.c)
    #find lib
    #mysdk
    # find_library(SDK_LIB libmysdk.a ${PROJECT_SOURCE_DIR}/lib)

    #gtk3
    # find_package (PkgConfig REQUIRED)
    # pkg_check_modules (GTK3 REQUIRED gtk+-3.0)
    # # set(CMAKE_C_STANDARD 11)
    # include_directories (${GTK3_INCLUDE_DIRS} ${INC_DIR})
    # link_directories (${GTK3_LIBRARY_DIRS} ${INC_DIR}) 
    link_directories (${INC_DIR}) 

    #zlog
    # find_library(ZLOG_LIB libzlog.a ${PROJECT_SOURCE_DIR}/lib/zlog)

    add_executable (sdk_cdcs ${CDCS_DIR} ${MY_DIR})

    # add_definitions (${GTK3_CFLAGS_OTHER})

    # target_link_libraries (sdk_app ${SDK_LIB} ${GTK3_LIBRARIES})
    # target_link_libraries (sdk_app ${SDK_LIB} ${GTK3_LIBRARIES})
    # target_link_libraries (main ${SDK_LIB} ${GTK3_LIBRARIES} ${ZLOG_LIB})

    set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
    message("end build cdcs")
endif()




if(${DEAMON})
    message("start build deamon")
    # #find lib
    # #mysdk
    # find_library(SDK_LIB libmysdk.a ${PROJECT_SOURCE_DIR}/lib)

    # #gtk3
    # find_package (PkgConfig REQUIRED)
    # pkg_check_modules (GTK3 REQUIRED gtk+-3.0)
    # # set(CMAKE_C_STANDARD 11)
    # include_directories (${GTK3_INCLUDE_DIRS} ${INC_DIR})
    # link_directories (${GTK3_LIBRARY_DIRS} ${INC_DIR}) 

    #zlog
    # find_library(ZLOG_LIB libzlog.a ${PROJECT_SOURCE_DIR}/lib/zlog)

    file(GLOB DEAMON_DIR
    ./deamon/*.c)

    add_executable (sdk_deamon ${DEAMON_DIR})

    # add_definitions (${GTK3_CFLAGS_OTHER})

    # target_link_libraries (main ${SDK_LIB} ${GTK3_LIBRARIES})
    # target_link_libraries (main ${SDK_LIB} ${GTK3_LIBRARIES} ${ZLOG_LIB})

    set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
    message("end build deamon")
endif()
