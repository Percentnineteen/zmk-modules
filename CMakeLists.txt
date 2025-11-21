if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_sources(app PRIVATE behaviors/behavior_sponge_lock.c)
  if(CONFIG_ZMK_DISABLE_DONGLE_SOFT_OFF)
          set_source_files_properties(
                  ${APPLICATION_SOURCE_DIR}/src/behaviors/behavior_soft_off.c
                  TARGET_DIRECTORY app
                  PROPERTIES HEADER_FILE_ONLY ON)
          target_sources(app PRIVATE behaviors/behavior_soft_off.c)
  endif()
endif()
