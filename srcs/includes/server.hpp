/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/05 12:51:45 by lomont            #+#    #+#             */
/*   Updated: 2026/04/05 14:49:02 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#ifndef SIZE_TEVENT
#define	SIZE_TEVENT 64
#endif

#include <map>
#include <iostream>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netdb.h>
#include "serverConfig.hpp"

class Client;

class server
{
	private:
		int 					*ServerSocket;
		int						EvenementQueue;
		void					fillSockaddrStruct(int);
		size_t					serverConfigCount;
		protoent* 				f;
		struct kevent 			event; //event to monitor
		struct kevent 			tevent[SIZE_TEVENT]; //triggered event
		struct config*			config;
		struct sockaddr_in*		sa;
		std::map<int, Client*> 	map;

		//Private member functions
		int 	FindLocation(const std::string&, size_t&, size_t&, size_t);
		int		FindOneConfiguration(const std::string &, size_t, struct config *);
		int		FindErrorPages(const std::string &, size_t &, size_t, struct config *);
		int		FindMaxBody(const std::string &, struct config *);
		int		FindServerConfig(int);
		void	ParsingError(std::string);
		void 	ParseServerConfiguration(const std::string&);
		void	ParseServerDeclaration(const std::string&);
		void	printConfig(struct LocationConfig*);
		size_t	GetServerConfigCount(const std::string &);
		size_t 	SearchLastBracket(const std::string &, size_t);
		size_t	FindMethods(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindRoot(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindIndex(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindAutoIndex(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindReturn(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindUpload(const std::string&, size_t, struct LocationConfig*, size_t&);
		size_t	FindCGIPass(const std::string&, size_t, struct LocationConfig*, size_t&);
	public:
		//Constructors / Destructors
		server(const std::string&);
		~server(void);

		//Getters
		int				findServerSocket(uintptr_t &);

		//Setters

		//Public member functions
		void	CreateNewClient(int, struct sockaddr, socklen_t, int);
		void	WaitForConnection(void);
		int 	ConfigureServer(void);
		void	CheckTimestamp(void);
		Client* FindCurrentClient(int);
};

#endif
