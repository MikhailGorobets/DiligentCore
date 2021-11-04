cmake_minimum_required (VERSION 3.6)

project(Diligent-ArchiveJSON CXX)

macro(generate_cxx_file LIST_PATHS) 
    foreach(PATH_TO_FILE ${LIST_PATHS})
        get_filename_component(BASE_NAME ${PATH_TO_FILE} NAME)
        string(REGEX REPLACE "[.]h$" ".hpp" BASE_NAME_HPP ${BASE_NAME})
        set(FILE_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/include/Generated_${BASE_NAME_HPP}")
        list(APPEND GENERATED_FILES ${FILE_OUTPUT})

        add_custom_command(OUTPUT 
            ${FILE_OUTPUT}          
            COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cxx_parser.py" 
                --dir  "${CMAKE_CURRENT_SOURCE_DIR}/include"
                --file "${CMAKE_CURRENT_SOURCE_DIR}/${PATH_TO_FILE}"
            COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../../BuildTools/FormatValidation/clang-format_10.0.0.exe" -i ${FILE_OUTPUT}
        )
    endforeach()
endmacro()

set(INCLUDE
    include/Comporators.hpp
    include/EngineEnvironment.hpp
    include/DeviceObjectReflection.hpp
    include/Generated_Common.hpp
    include/pch.h
)

set(SOURCE
    src/EngineEnvironment.cpp
    src/DeviceObjectReflection.cpp
    src/Archive.cpp
)

set(REFLECTED
    ../GraphicsEngine/interface/BlendState.h
    ../GraphicsEngine/interface/DepthStencilState.h
    ../GraphicsEngine/interface/GraphicsTypes.h
    ../GraphicsEngine/interface/InputLayout.h
    ../GraphicsEngine/interface/PipelineResourceSignature.h
    ../GraphicsEngine/interface/PipelineState.h
    ../GraphicsEngine/interface/RasterizerState.h
    ../GraphicsEngine/interface/RenderPass.h
    ../GraphicsEngine/interface/Shader.h
    ../GraphicsEngine/interface/Sampler.h
)

find_package(PythonInterp REQUIRED 3.7.6)
execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install libclang)

set(GENERATED_FILES "" CACHE INTERNAL "Generated files")
#generate_cxx_file("${REFLECTED}")

add_custom_target(Diligent-ArchiveGeneratedFiles ALL ${GENERATED_FILES})

source_group("source" FILES ${SOURCE})
source_group("include" FILES ${INCLUDE} ${GENERATED_FILES})

add_executable(Diligent-ArchiveJSON 
    ${SOURCE}
    ${INCLUDE}
    ${GENERATED_FILES} 
)

add_dependencies(Diligent-ArchiveJSON Diligent-ArchiveGeneratedFiles)

target_include_directories(Diligent-ArchiveJSON
	PRIVATE
    	include
        ../ArchiveBuilder/include
    	../../ThirdParty/json/include
)

target_link_libraries(Diligent-ArchiveJSON PUBLIC Diligent-ArchiveBuilder)
set_common_target_properties(Diligent-ArchiveJSON)

set_target_properties(Diligent-ArchiveJSON PROPERTIES
    FOLDER DiligentCore/Graphics
)

set_target_properties(Diligent-ArchiveGeneratedFiles PROPERTIES
    FOLDER DiligentCore/Graphics
)
