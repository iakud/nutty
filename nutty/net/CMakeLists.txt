cmake_minimum_required(VERSION 2.8)

set(NUTTY_NET_SRCS
	InetAddress.cpp
	Socket.cpp
	Acceptor.cpp
	Connector.cpp
	Buffer.cpp
	TcpConnection.cpp
	TcpServer.cpp
	TcpClient.cpp
	)

add_library(nutty_net ${NUTTY_NET_SRCS})
target_link_libraries(nutty_net nutty_base)

# install(TARGETS nutty_net DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/nutty/net)

if (NOT CMAKE_BUILD_NO_EXAMPLES)
	add_subdirectory(tests)
endif()