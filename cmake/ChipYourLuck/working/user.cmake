set(LOCAL_LINKER_SCRIPT
    "${CMAKE_CURRENT_LIST_DIR}/ATSAME51J20A.ld"
)

target_link_options(
    ChipYourLuck_working_working_toolchain_compile
    PRIVATE
    "-Wl,--script,${LOCAL_LINKER_SCRIPT}"
)