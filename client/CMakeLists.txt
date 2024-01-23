set(CLIENT_SOURCES
	source/process.h
	source/process.cpp
	source/basesession.h
	source/baseserver.h
	source/basecore.h
	source/session.h
	source/session.cpp
	source/server.h
	source/server.cpp
	source/core.h
	source/core.cpp
	source/global.h
	source/main.cpp)
	
add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client 
	spdlog 
	argparse
	winpipe
	common
	ws2_32.lib
	shlwapi.lib)