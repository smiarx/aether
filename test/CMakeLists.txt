add_executable(test_dsp
  test_dsp.c
)

target_link_libraries(test_dsp
  dsp
  unity
)

add_test(NAME test_dsp COMMAND test_dsp)
