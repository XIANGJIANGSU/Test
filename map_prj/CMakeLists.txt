CMAKE_MINIMUM_REQUIRED(VERSION 2.6)


 
# generate lib path
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build_map_freeglut_cmake/lib)

# 查找指定目录下的所有*.h头文件,*.c源文件,并将文件列表保存到指定变量
file(GLOB_RECURSE MAP_camera "./src/camera/*.h" "./src/camera/*.c")
file(GLOB_RECURSE MAP_define "./src/define/*.h" "./src/define/*.c")
file(GLOB_RECURSE MAP_engine "./src/engine/*.h" "./src/engine/*.c")
file(GLOB_RECURSE MAP_geometry "./src/geometry/*.h" "./src/geometry/*.c")
file(GLOB_RECURSE MAP_mapApi "./src/mapApi/*.h" "./src/mapApi/*.c")
file(GLOB_RECURSE MAP_mapApp "./src/mapApp/*.h" "./src/mapApp/*.c")
file(GLOB_RECURSE MAP_model "./src/model/*.h" "./src/model/*.c")
file(GLOB_RECURSE MAP_models "./src/models/*.h" "./src/models/*.c")
file(GLOB_RECURSE MAP_projection "./src/projection/*.h" "./src/projection/*.c")
file(GLOB_RECURSE MAP_utility "./src/utility/*.h" "./src/utility/*.c")


# 将指定目录下的头文件和源文件放到指定目录下
source_group("camera" FILES ${MAP_camera})
source_group("define" FILES ${MAP_define})
source_group("engine" FILES ${MAP_engine})
source_group("geometry" FILES ${MAP_geometry})
source_group("mapApi" FILES ${MAP_mapApi})
source_group("mapApp" FILES ${MAP_mapApp})
source_group("model" FILES ${MAP_model})
source_group("models" FILES ${MAP_models})
source_group("projection" FILES ${MAP_projection})
source_group("utility" FILES ${MAP_utility})

#add src files to make the lib
#aux_source_directory(./src/camera/ MAP_SRC)
#aux_source_directory(./src/define/ MAP_SRC)
#aux_source_directory(./src/engine/ MAP_SRC)
#aux_source_directory(./src/geometry/ MAP_SRC)
#aux_source_directory(./src/mapApi/ MAP_SRC)
##list(REMOVE_ITEM MAP_SRC ./src/mapApi/searchDir.c)
#aux_source_directory(./src/mapApp/ MAP_SRC)
#aux_source_directory(./src/model/ MAP_SRC)
#aux_source_directory(./src/models/ MAP_SRC)
#aux_source_directory(./src/projection/ MAP_SRC)
#aux_source_directory(./src/utility/ MAP_SRC)

#static lib --default
add_library(map_prj STATIC
            ${MAP_camera}
            ${MAP_define}
            ${MAP_engine}
            ${MAP_geometry}
            ${MAP_mapApi}
            ${MAP_mapApp}
            ${MAP_model}
            ${MAP_models}
            ${MAP_projection}
            ${MAP_utility})

#shared lib
#add_library(map_prj SHARED
#            ${MAP_camera}
#            ${MAP_define}
#            ${MAP_engine}
#            ${MAP_geometry}
#            ${MAP_mapApi}
#            ${MAP_mapApp}
#            ${MAP_model}
#            ${MAP_models}
#            ${MAP_projection}
#            ${MAP_utility})
 
