#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>

int main(){
	std::cout << "Daemon started. Boost Version: " << BOOST_LIB_VERSION << std::endl;
	return 0;
}
