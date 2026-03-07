/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 23:09:55 by lomont            #+#    #+#             */
/*   Updated: 2026/03/07 03:34:08 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#define DEFAULT_CONFIGURATION_FILE "webserv.conf"

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/errno.h>
#include <string>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <vector>
#include "utils.hpp"
#include <cstdlib>

struct LocationConfig {
			std::string									location;
			std::vector<std::string>					methods;
			std::string									root;
			std::string									upload_store;
			std::vector<std::string>					index;
			bool										autoindex;
			std::pair<std::string, std::string> 		_return;
			std::string									pathPHPexecutable;
};

struct	config {
	std::pair<std::string, int>							interfacePort;
	std::map<std::vector<int>, std::string >			errorPage;
	unsigned long 										maxBodySize;
	LocationConfig*										locationConfig;
};

class server
{
	private:
		struct sockaddr_in 	sa;
		protoent* 			f;
		struct kevent 		event;
		struct kevent 		tevent;
		int 				ServerSocket;
		int 				BindServerSocket;
		int 				ListenServerSocket;
		//int 				AcceptClientConnexion;
		int					EvenementQueue;
		size_t				serverConfigCount;
		struct config*		config;
		void fillSockaddrStruct(void);
	public:
		//Constructors / Destructors
		server(const std::string&);
		~server(void);

		//getters
		struct kevent* 	getevent(void);
		struct kevent* 	getTevent(void);
		int&			getServerSocket(void);
		int&			getEvenementQueue(void);

		//setters

		//member functions
		void 	ParseServerConfiguration(const std::string&);
		void	ParseServerDeclaration(const std::string&);
		size_t	SearchLastAccolade(const std::string&, size_t);
		void	FindOneConfiguration(const std::string&, size_t, struct config*);
		void	FindErrorPages(const std::string&, size_t&, size_t, struct config*);
		void	FindMaxBody(const std::string&, struct config*);
		void	FindLocation(const std::string&, size_t&, size_t&, size_t);
		void 	ConfigureServer(void);
		size_t	GetServerConfigCount(const std::string&);
		void	printConfig(struct LocationConfig*);

		//parsing functions
		size_t	FindMethods(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindRoot(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindIndex(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindAutoIndex(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindReturn(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindUpload(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindCGIPass(const std::string&, size_t, struct LocationConfig*, size_t&);
};

extern int errno;

#endif

