set(DEPENDENT_MP_BIN2HEXChipYourLuck_default_UhetsaN_ "c:/Program Files/Microchip/xc32/v6.00/bin/xc32-bin2hex.exe")
set(DEPENDENT_DEPENDENT_TARGET_ELFChipYourLuck_default_UhetsaN_ ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ChipYourLuck/default.elf)
set(DEPENDENT_TARGET_DIRChipYourLuck_default_UhetsaN_ ${CMAKE_CURRENT_LIST_DIR}/../../../../out/ChipYourLuck)
set(DEPENDENT_BYPRODUCTSChipYourLuck_default_UhetsaN_ ${DEPENDENT_TARGET_DIRChipYourLuck_default_UhetsaN_}/${sourceFileNameChipYourLuck_default_UhetsaN_}.c)
add_custom_command(
    OUTPUT ${DEPENDENT_TARGET_DIRChipYourLuck_default_UhetsaN_}/${sourceFileNameChipYourLuck_default_UhetsaN_}.c
    COMMAND ${DEPENDENT_MP_BIN2HEXChipYourLuck_default_UhetsaN_} --image ${DEPENDENT_DEPENDENT_TARGET_ELFChipYourLuck_default_UhetsaN_} --image-generated-c ${sourceFileNameChipYourLuck_default_UhetsaN_}.c --image-generated-h ${sourceFileNameChipYourLuck_default_UhetsaN_}.h --image-copy-mode ${modeChipYourLuck_default_UhetsaN_} --image-offset ${addressChipYourLuck_default_UhetsaN_} 
    WORKING_DIRECTORY ${DEPENDENT_TARGET_DIRChipYourLuck_default_UhetsaN_}
    DEPENDS ${DEPENDENT_DEPENDENT_TARGET_ELFChipYourLuck_default_UhetsaN_})
add_custom_target(
    dependent_produced_source_artifactChipYourLuck_default_UhetsaN_ 
    DEPENDS ${DEPENDENT_TARGET_DIRChipYourLuck_default_UhetsaN_}/${sourceFileNameChipYourLuck_default_UhetsaN_}.c
    )
