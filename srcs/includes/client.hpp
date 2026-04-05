#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/socket.h>
#include "headerRequest.hpp"
#include "headerResponse.hpp"

struct config;

class Client
{
	private:
			int 				fd;
			int					timestamp;
			bool				headerFound;
			long				pos;
			ssize_t				bytesSent;
			socklen_t 			socklenClient;
			std::string			headerBuffer;
			std::string			headerBody;
			HeaderRequest		_request;
			HeaderResponse		_response;
			struct config*		config;
			struct sockaddr 	sockaddrClient;

	public:
		//constructor
		Client(int, struct config*);

		//destructor
		~Client(void);

		//getters
		int						GetTime(void);
		socklen_t*				getSockLenClient(void);
		struct sockaddr* 		getsockaddrClient(void);

		//setters
		void					SetSockaddrClient(struct sockaddr);
		void					SetSockLenClient(socklen_t);

		//member functions
		bool					CheckErrors(std::string&);
		void					ReceiveHeader(int&, std::map<int, Client*>& map);
		void					ResponseToClient(std::map<int, Client*>&, int&, struct config*);
		void					ChangeKeventState(int&, bool);
		void					InternalError(int&);
		void					CloseConnection(std::map<int, Client*>&, bool);
		void					ResizeBuffer(std::string&);
		void					CleanClient(void);
		void					RefreshTimestamp(void);
};

#endif
