add_executable(sample
  sample.cpp
)

target_link_libraries(sample
  ${PROJECT_NAME}
  )

install(TARGETS sample
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
  )
