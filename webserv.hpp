/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 23:09:55 by lomont            #+#    #+#             */
/*   Updated: 2026/03/04 01:58:29 by lomont           ###   ########.fr       */
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
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <vector>
#include "utils.hpp"

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
		std::pair<std::string, int> interfacePort;
		std::map<std::vector<int>, std::string > errorPage;
		void fillSockaddrStruct(void);
	public:
		//Constructors / Destructors
		server(void);
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
		void 	ConfigureServer(void);
		size_t	GetServerConfigCount(const std::string&);
};

extern int errno;

#endif

