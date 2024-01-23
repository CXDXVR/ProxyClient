set(REDIRECTOR_SOURCES
	source/string.hpp
	source/hook.hpp
	source/basecore.h
	source/basesocks.h
	source/config.h
	source/config.cpp
	source/socks4.hpp
	source/socks5.hpp
	source/sockethook.h
	source/sockethook.cpp
	source/core.h
	source/core.cpp
	source/global.h
	source/main.cpp)
	
add_library(redirector SHARED ${REDIRECTOR_SOURCES})
target_link_libraries(redirector 
	spdlog 
	minhook
	winpipe
	common
	ws2_32.lib)