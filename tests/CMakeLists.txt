add_executable(unittests main.cpp)

target_include_directories(unittests
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

target_compile_definitions(unittests
    PRIVATE
        DATA_DIR="${CMAKE_CURRENT_LIST_DIR}/data/"
)

target_sources(unittests
    PUBLIC
        filter.hpp
        lfo.hpp
        modulation_envelope.hpp
        sampler.hpp
        synthesizer.hpp
        voice.hpp
        volume_envelope.hpp
)

# Run the unit tests
if (KNM_SYNTHESIZER_RUN_TESTS)
    add_custom_target(run-unittests
        ALL
        unittests
        DEPENDS unittests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Unit testing..." VERBATIM
    )
endif()
