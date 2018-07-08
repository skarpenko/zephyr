add_custom_target(run
  COMMAND
  $ENV{ULTIPARC_HOME}/slm/sys_model/sys_model.elf -fw_image
  ${APPLICATION_BINARY_DIR}/zephyr/${KERNEL_BIN_NAME}
  USES_TERMINAL
  )
