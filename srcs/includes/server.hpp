/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/05 12:51:45 by lomont            #+#    #+#             */
/*   Updated: 2026/04/15 16:22:44 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#ifndef SIZE_TEVENT
#define	SIZE_TEVENT 64
#endif

#include <map>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/epoll.h>
#include <cerrno>
#include <sys/time.h>
#include <netdb.h>
#include <strings.h>
#include "serverConfig.hpp"

class Client;

class server
{
	private:
		int 					*ServerSocket;
		int						EvenementQueue;
		bool					parsing_error;
		void					fillSockaddrStruct(int);
		size_t					serverConfigCount;
		protoent* 				f;
		struct epoll_event 		event; //event to monitor
		struct epoll_event 		tevent[SIZE_TEVENT]; //triggered event
		struct config*			config;
		struct sockaddr_in*		sa;
		std::map<int, Client*> 	map;

		//Private member functions
		int 	FindLocation(const std::string&, size_t&, size_t&, size_t);
		int		FindServerConfig(int);
		int 	ParseServerConfiguration(const std::string&);
		int		ParseServerDeclaration(const std::string&);
	public:
		//Constructors / Destructors
		server(const std::string&);
		~server(void);

		//Getters
		int				findServerSocket(epoll_data_t);

		//Setters

		//Public member functions
		void	CreateNewClient(int, struct sockaddr, socklen_t, int);
		void	WaitForConnection(void);
		int 	ConfigureServer(void);
		void	CheckTimestamp(void);
		Client* FindCurrentClient(int);
};

#endif
