set(dsp_sources
    tapedelay.c
    springreverb.c
    filter.c)

add_library(dsp STATIC ${dsp_sources})

target_compile_options(dsp PRIVATE
       -Wall -Werror
       $<$<CONFIG:RELEASE>:-Ofast -march=native -mtune=native>
       $<$<CONFIG:RELEASE>:-fopenmp-simd>
       $<$<CONFIG:DEBUG>:-O0>
       $<$<CONFIG:DEBUG>:-g>
   )
