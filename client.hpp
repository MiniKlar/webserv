#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/socket.h>
#include <sys/event.h>
#include "HeaderRequest.hpp"
#include "HeaderResponse.hpp"
#include "utils.hpp"

struct config;

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
			ssize_t				bytesSent;
			bool				headerFound;
	public:
		//constructor
		Client(int);

		//destructor
		~Client(void);

		//getters
		struct sockaddr* 		getsockaddrClient(void);
		socklen_t*				getSockLenClient(void);

		//setters
		void					SetSockaddrClient(struct sockaddr);
		void					SetSockLenClient(socklen_t);

		//member functions
		void					ReceiveHeader(int&, std::map<int, Client*>&);
		int						ReceiveBody(int&);
		void					ResponseToClient(std::map<int, Client*>&, int&, struct config*);
		void					ChangeKeventState(int&, bool);
		void					InternalError(int&);
		bool					CheckErrors(std::string&);
		void					CloseConnection(std::map<int, Client*>&);
		void					ResizeBuffer(std::string&);
		void					CleanClient(void);
};

#endif
