/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 23:17:19 by lomont            #+#    #+#             */
/*   Updated: 2026/02/26 21:55:44 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

//Constructor server
server::server(void) : f(getprotobyname("TCP")) {
	const int opt = 1;
	logs("Web server started");
	if (!this->f)
		ft_error("getprotobyname failed", 1);
	fillSockaddrStruct();
	ServerSocket = socket(PF_INET, SOCK_STREAM, f->p_proto);
	if (ServerSocket == -1)
		ft_error("socket failed", 2);
	if (setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		ft_error("setsockopt failed", 3);
	BindServerSocket = bind(ServerSocket, (struct sockaddr*)&sa, sizeof(sa));
	if (BindServerSocket == -1)
		ft_error("bind failed", 4);
	ListenServerSocket = listen(ServerSocket, 1);
	if (ListenServerSocket == -1)
		ft_error("listen creation failed", 5);
	fcntl(ServerSocket, F_SETFL, O_NONBLOCK);
	EvenementQueue = kqueue();
	if (EvenementQueue == -1)
		ft_error("kqueue fd creation failed", 6);
	EV_SET(&event, ServerSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
	if (kevent(EvenementQueue, &event, 1, NULL, 0, NULL) == -1)
		ft_error("kevent event addition failed", 7);
}

//Destruct server
server::~server(void) {
	return ;
}

struct kevent* server::getTevent(void) {
	return (&this->tevent);
}

struct kevent* server::getevent(void) {
	return (&this->event);
}

int& server::getServerSocket(void) {
	return (this->ServerSocket);
}

int& server::getEvenementQueue(void) {
	return (this->EvenementQueue);
}

void server::fillSockaddrStruct(void) {
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
}
