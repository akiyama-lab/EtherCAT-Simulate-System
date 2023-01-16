set (SOES_DEMO applications/raspberry_lan9252demo)
set(HAL_SOURCES
${SOES_SOURCE_DIR}/soes/hal/raspberrypi-lan9252/esc_hw.c
${SOES_SOURCE_DIR}/soes/hal/raspberrypi-lan9252/esc_hw.h
)

include_directories(
  ${SOES_SOURCE_DIR}/soes/include/sys/gcc
  ${SOES_SOURCE_DIR}/${SOES_DEMO}
  )

# Common compile flags
add_compile_options(-Wall -Wextra -Wno-unused-parameter -Werror)
