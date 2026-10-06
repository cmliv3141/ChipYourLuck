# The following variables contains the files used by the different stages of the build process.
set(ChipYourLuck_default_default_XC32_FILE_TYPE_assemble)
set_source_files_properties(${ChipYourLuck_default_default_XC32_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${ChipYourLuck_default_default_XC32_FILE_TYPE_assemble})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(ChipYourLuck_default_default_XC32_FILE_TYPE_assembleWithPreprocess)
set_source_files_properties(${ChipYourLuck_default_default_XC32_FILE_TYPE_assembleWithPreprocess} PROPERTIES LANGUAGE ASM)

# For assembly files, add "." to the include path for each file so that .include with a relative path works
foreach(source_file ${ChipYourLuck_default_default_XC32_FILE_TYPE_assembleWithPreprocess})
        set_source_files_properties(${source_file} PROPERTIES INCLUDE_DIRECTORIES "$<PATH:NORMAL_PATH,$<PATH:REMOVE_FILENAME,${source_file}>>")
endforeach()

set(ChipYourLuck_default_default_XC32_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/led.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/main.c")
set_source_files_properties(${ChipYourLuck_default_default_XC32_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(ChipYourLuck_default_default_XC32_FILE_TYPE_compile_cpp)
set_source_files_properties(${ChipYourLuck_default_default_XC32_FILE_TYPE_compile_cpp} PROPERTIES LANGUAGE CXX)
set(ChipYourLuck_default_default_XC32_FILE_TYPE_link)
set(ChipYourLuck_default_image_name "default.elf")
set(ChipYourLuck_default_image_base_name "default")

# The output directory of the final image.
set(ChipYourLuck_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/ChipYourLuck")

# The full path to the final image.
set(ChipYourLuck_default_full_path_to_image ${ChipYourLuck_default_output_dir}/${ChipYourLuck_default_image_name})
