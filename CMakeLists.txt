project(rogue)

cmake_minimum_required(VERSION 2.8.12)

if(NOT ${CMAKE_VERSION} VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
else()
    set(CMAKE_CXX_FLAGS "-std=c++11")
endif()

# Add warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(NOT APPLE AND NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
endif()

set(SRCS
	armor.cpp    daemon.cpp   fight.cpp  jedi.cpp      main.cpp    monsters.cpp   options.cpp   potions.cpp  rooms.cpp    state.cpp   vers.cpp
	chase.cpp    daemons.cpp  init.cpp   mdport.cpp  move.cpp       pack.cpp      rings.cpp    save.cpp     sticks.cpp  weapons.cpp
	command.cpp  extern.cpp   io.cpp     mach_dep.cpp  misc.cpp    new_level.cpp  passages.cpp  rip.cpp      scrolls.cpp  things.cpp  wizard.cpp
    line_of_sight.cpp
)

add_executable(rogue ${SRCS})

include(GNUInstallDirs)
install(TARGETS rogue DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CPackConfig.cmake)

