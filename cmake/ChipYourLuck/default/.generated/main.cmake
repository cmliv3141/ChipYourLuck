include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(ChipYourLuck_default_library_list )

# Handle files with suffix s, for group default-XC32
if(ChipYourLuck_default_default_XC32_FILE_TYPE_assemble)
add_library(ChipYourLuck_default_default_XC32_assemble OBJECT ${ChipYourLuck_default_default_XC32_FILE_TYPE_assemble})
    ChipYourLuck_default_default_XC32_assemble_rule(ChipYourLuck_default_default_XC32_assemble)
    list(APPEND ChipYourLuck_default_library_list "$<TARGET_OBJECTS:ChipYourLuck_default_default_XC32_assemble>")

endif()

# Handle files with suffix S, for group default-XC32
if(ChipYourLuck_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
add_library(ChipYourLuck_default_default_XC32_assembleWithPreprocess OBJECT ${ChipYourLuck_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
    ChipYourLuck_default_default_XC32_assembleWithPreprocess_rule(ChipYourLuck_default_default_XC32_assembleWithPreprocess)
    list(APPEND ChipYourLuck_default_library_list "$<TARGET_OBJECTS:ChipYourLuck_default_default_XC32_assembleWithPreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(ChipYourLuck_default_default_XC32_FILE_TYPE_compile)
add_library(ChipYourLuck_default_default_XC32_compile OBJECT ${ChipYourLuck_default_default_XC32_FILE_TYPE_compile})
    ChipYourLuck_default_default_XC32_compile_rule(ChipYourLuck_default_default_XC32_compile)
    list(APPEND ChipYourLuck_default_library_list "$<TARGET_OBJECTS:ChipYourLuck_default_default_XC32_compile>")

endif()

# Handle files with suffix cpp, for group default-XC32
if(ChipYourLuck_default_default_XC32_FILE_TYPE_compile_cpp)
add_library(ChipYourLuck_default_default_XC32_compile_cpp OBJECT ${ChipYourLuck_default_default_XC32_FILE_TYPE_compile_cpp})
    ChipYourLuck_default_default_XC32_compile_cpp_rule(ChipYourLuck_default_default_XC32_compile_cpp)
    list(APPEND ChipYourLuck_default_library_list "$<TARGET_OBJECTS:ChipYourLuck_default_default_XC32_compile_cpp>")

endif()

# Handle files with suffix [cC], for group default-XC32
if(ChipYourLuck_default_default_XC32_FILE_TYPE_dependentObject)
add_library(ChipYourLuck_default_default_XC32_dependentObject OBJECT ${ChipYourLuck_default_default_XC32_FILE_TYPE_dependentObject})
    ChipYourLuck_default_default_XC32_dependentObject_rule(ChipYourLuck_default_default_XC32_dependentObject)
    list(APPEND ChipYourLuck_default_library_list "$<TARGET_OBJECTS:ChipYourLuck_default_default_XC32_dependentObject>")

endif()


# Main target for this project
add_executable(ChipYourLuck_default_image_UhetsaN_ ${ChipYourLuck_default_library_list})

set_target_properties(ChipYourLuck_default_image_UhetsaN_ PROPERTIES
    OUTPUT_NAME "default"
    SUFFIX ".elf"
    RUNTIME_OUTPUT_DIRECTORY "${ChipYourLuck_default_output_dir}")
target_link_libraries(ChipYourLuck_default_image_UhetsaN_ PRIVATE ${ChipYourLuck_default_default_XC32_FILE_TYPE_link})
# Add the link options from the rule file.
ChipYourLuck_default_link_rule( ChipYourLuck_default_image_UhetsaN_)

# Add bin2hex target for converting built file to a .hex file.
string(REGEX REPLACE [.]elf$ .hex ChipYourLuck_default_image_name_hex ${ChipYourLuck_default_image_name})
add_custom_target(ChipYourLuck_default_Bin2Hex ALL
    COMMAND ${MP_BIN2HEX} \"${ChipYourLuck_default_output_dir}/${ChipYourLuck_default_image_name}\"
    BYPRODUCTS ${ChipYourLuck_default_output_dir}/${ChipYourLuck_default_image_name_hex}
    COMMENT "Convert built file to .hex")
add_dependencies(ChipYourLuck_default_Bin2Hex ChipYourLuck_default_image_UhetsaN_)




