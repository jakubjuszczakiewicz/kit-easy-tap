cmake_minimum_required(VERSION 3.5)
project(kit-easy-tap C)

set(SOURCE_FILES
  src/main.c
  src/tap.c
  src/tools.c
  src/bridge.c
  src/version.c
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

install(TARGETS ${PROJECT_NAME} PERMISSIONS SETUID OWNER_READ OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE)
