# The following functions contains all the flags passed to the different build stages.

set(PACK_REPO_PATH "C:/Users/clivi/.mchp_packs" CACHE PATH "Path to the root of a pack repository.")

function(ChipYourLuck_default_default_XC32_assemble_rule target)
    set(options
        "-g"
        "${ASSEMBLER_PRE}"
        "-mprocessor=ATSAME51J20A"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1"
<<<<<<< HEAD
        "-g,-I${CMAKE_CURRENT_SOURCE_DIR}/../../..,-I${CMAKE_CURRENT_SOURCE_DIR}/../../../inc"
=======
        "-g,-I${CMAKE_CURRENT_SOURCE_DIR}/../../..,-I${CMAKE_CURRENT_SOURCE_DIR}/../../../inc/"
>>>>>>> 89bfd5ea2651ea2864b6daccead835e4cc11de35
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAME51_DFP/3.9.267")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "__DEBUG=1")
<<<<<<< HEAD
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
=======
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
        PRIVATE "c:/Users/clivi/Desktop/ChipYourLuck/inc")
>>>>>>> 89bfd5ea2651ea2864b6daccead835e4cc11de35
endfunction()
function(ChipYourLuck_default_default_XC32_assembleWithPreprocess_rule target)
    set(options
        "-x"
        "assembler-with-cpp"
        "-g"
        "${MP_EXTRA_AS_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAME51_DFP/3.9.267"
        "-mprocessor=ATSAME51J20A"
        "-Wa,--defsym=__MPLAB_BUILD=1${MP_EXTRA_AS_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-I${CMAKE_CURRENT_SOURCE_DIR}/../../..")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
<<<<<<< HEAD
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
=======
    target_include_directories(${target}
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
        PRIVATE "c:/Users/clivi/Desktop/ChipYourLuck/inc")
>>>>>>> 89bfd5ea2651ea2864b6daccead835e4cc11de35
endfunction()
function(ChipYourLuck_default_default_XC32_compile_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "-x"
        "c"
        "-c"
        "-mprocessor=ATSAME51J20A"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAME51_DFP/3.9.267")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
<<<<<<< HEAD
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../inc"
=======
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../inc/"
>>>>>>> 89bfd5ea2651ea2864b6daccead835e4cc11de35
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/5.4.0/CMSIS/Core/Include")
endfunction()
function(ChipYourLuck_default_default_XC32_compile_cpp_rule target)
    set(options
        "-g"
        "${CC_PRE}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAME51J20A"
        "-frtti"
        "-fexceptions"
        "-fno-check-new"
        "-fenforce-eh-specs"
        "-fno-common"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAME51_DFP/3.9.267")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target}
        PRIVATE "__DEBUG"
        PRIVATE "XPRJ_default=default")
    target_include_directories(${target}
<<<<<<< HEAD
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../inc"
=======
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../inc/"
>>>>>>> 89bfd5ea2651ea2864b6daccead835e4cc11de35
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../.."
        PRIVATE "${PACK_REPO_PATH}/ARM/CMSIS/5.4.0/CMSIS/Core/Include")
endfunction()
function(ChipYourLuck_default_dependentObject_rule target)
    set(options
        "-mprocessor=ATSAME51J20A"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAME51_DFP/3.9.267")
    list(REMOVE_ITEM options "")
    target_compile_options(${target} PRIVATE "${options}")
endfunction()
function(ChipYourLuck_default_link_rule target)
    set(options
        "-g"
        "${MP_EXTRA_LD_PRE}"
        "${DEBUGGER_OPTION_TO_LINKER}"
        "${DEBUGGER_NAME_AS_MACRO}"
        "-mprocessor=ATSAME51J20A"
        "-Wl,--defsym=__MPLAB_BUILD=1${MP_EXTRA_LD_POST},--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-L${CMAKE_CURRENT_SOURCE_DIR}/../../..,-Map=mem.map,--memorysummary,memoryfile.xml"
        "-mdfp=${PACK_REPO_PATH}/Microchip/SAME51_DFP/3.9.267")
    list(REMOVE_ITEM options "")
    target_link_options(${target} PRIVATE "${options}")
    target_compile_definitions(${target} PRIVATE "XPRJ_default=default")
endfunction()
