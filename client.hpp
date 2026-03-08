#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/socket.h>
#include <sys/event.h>
#include "HeaderRequest.hpp"
#include "HeaderResponse.hpp"
#include "utils.hpp"

class Client
{
	private:
			int 				fd;
			long				pos;
			struct sockaddr 	sockaddrClient;
			socklen_t 			socklenClient;
			std::string			headerBuffer;
			std::string			headerBody;
			HeaderRequest		_request;
			HeaderResponse		_response;
			int					bytesSent;
			bool				headerFound;
	public:
		Client(int);
		~Client(void);
		//getters
		struct sockaddr* 		getsockaddrClient(void);
		socklen_t*				getSockLenClient(void);

		//setters
		void					SetSockaddrClient(struct sockaddr);
		void					SetSockLenClient(socklen_t);

		//member functions
		void					receive_header(struct kevent*, int&);
		int						receive_body(void);
		void					ResponseToClient(std::map<int, Client*>&, struct kevent*, int&);
};

#endif
