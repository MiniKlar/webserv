#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/socket.h>
#include "HeaderRequest.hpp"
#include "HeaderResponse.hpp"
#include "utils.hpp"

class Client
{
	private:
			int 				fd;
			struct sockaddr 	sockaddrClient;
			socklen_t 			socklenClient;
			HeaderRequest		_request;
			HeaderResponse		_response;
	public:
		Client(int);
		~Client(void);
		//getters
		struct sockaddr* 		getsockaddrClient(void);
		socklen_t*				getSockLenClient(void);

		//member functions
		void					receive_header(void);
		void					ResponseToClient(void);
};

#endif
