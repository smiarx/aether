set(dsp_sources
    tapedelay.c
    springreverb.c
    filter.c)

add_library(dsp STATIC ${dsp_sources})

target_link_libraries(dsp PUBLIC m)

target_include_directories(dsp PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(dsp PRIVATE
       -Wall -Werror -fopenmp-simd
       $<$<CONFIG:RELEASE>:-Ofast -march=native -mtune=native>
       $<$<CONFIG:DEBUG>:-O0>
       $<$<CONFIG:DEBUG>:-g>
   )
