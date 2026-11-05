if(WIN32)
    find_program(MIDL_EXECUTABLE midl
        HINTS
        "$ENV{WindowsSdkVerBinPath}/x64"
        "$ENV{WindowsSdkDir}/bin/$ENV{WindowsSDKVersion}/x64"
    )

    if(NOT MIDL_EXECUTABLE)
        message(FATAL_ERROR "MIDL compiler not found")
    endif()

#    set(DECKLINK_SDK_DIR "C:/Program Files/Blackmagic Design/Blackmagic DeckLink SDK")

    set(DECKLINK_IDL
        "${DECKLINK_DIR}/include/DeckLinkAPI.idl"
    )

    set(DECKLINK_GENERATED_DIR
        "${CMAKE_CURRENT_BINARY_DIR}/decklink_generated"
    )

    file(MAKE_DIRECTORY "${DECKLINK_GENERATED_DIR}")

    add_custom_command(
        OUTPUT
            "${DECKLINK_GENERATED_DIR}/DeckLinkAPI.h"
            "${DECKLINK_GENERATED_DIR}/DeckLinkAPI_i.c"

        COMMAND "${MIDL_EXECUTABLE}"
            /nologo
            /env x64
            /h "${DECKLINK_GENERATED_DIR}/DeckLinkAPI.h"
            /iid "${DECKLINK_GENERATED_DIR}/DeckLinkAPI_i.c"
            /tlb "${DECKLINK_GENERATED_DIR}/DeckLinkAPI.tlb"
            "${DECKLINK_IDL}"

        DEPENDS "${DECKLINK_IDL}"

        COMMENT "Generating DeckLink COM interfaces"
        VERBATIM
    )

    add_custom_target(decklink_idl
        DEPENDS
            "${DECKLINK_GENERATED_DIR}/DeckLinkAPI.h"
            "${DECKLINK_GENERATED_DIR}/DeckLinkAPI_i.c"
    )
endif()