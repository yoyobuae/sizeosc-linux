# CMakeLists.txt
cmake_minimum_required(VERSION 3.0)
project(sizeosc VERSION 0.0.1)

# Find dependencies provided by vcpkg (via vcpkg.cmake)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# main target
add_executable(sizeosc)
target_sources(sizeosc PRIVATE main.cpp osc/OscOutboundPacketStream.cpp osc/OscPrintReceivedElements.cpp osc/OscReceivedElements.cpp osc/OscTypes.cpp ip/IpEndpointName.cpp)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  message(DEBUG "Linux")
  target_sources(sizeosc PRIVATE ip/posix/NetworkingUtils.cpp ip/posix/UdpSocket.cpp)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  message(DEBUG "Windows")
  target_sources(sizeosc PRIVATE ip/win32/NetworkingUtils.cpp ip/win32/UdpSocket.cpp)
  target_link_libraries(sizeosc PRIVATE wsock32 ws2_32 winmm)
else()
  message(FATAL "No OS detected!")
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

target_link_libraries(sizeosc PRIVATE imgui::imgui)
target_link_libraries(sizeosc PRIVATE OpenGL::GL)
target_include_directories(sizeosc PRIVATE .)
set_property(TARGET sizeosc PROPERTY CXX_STANDARD 20)

install(TARGETS sizeosc)
install(FILES "imgui.ini" TYPE BIN)
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	install(FILES "${CMAKE_BINARY_DIR}/Release/SDL2.dll" TYPE BIN)
endif(CMAKE_SYSTEM_NAME MATCHES "Windows")

include(CPack)
