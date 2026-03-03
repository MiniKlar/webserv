/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 23:09:55 by lomont            #+#    #+#             */
/*   Updated: 2026/03/02 00:39:05 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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
		void fillSockaddrStruct(void);
	public:
		//Constructors / Destructors
		server(void);
		~server(void);
		//getters
		struct kevent* getevent(void);
		struct kevent* getTevent(void);
		int&	getServerSocket(void);
		int&	getEvenementQueue(void);
};

extern int errno;

#endif

