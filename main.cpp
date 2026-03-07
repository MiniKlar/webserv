/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 23:06:49 by lomont            #+#    #+#             */
/*   Updated: 2026/03/07 03:37:44 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>
#include <map>
#include <sys/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "webserv.hpp"
#include "client.hpp"

int main(int argc, char** argv) {
	if (argc > 1) {
		server webserv(argv[argc - 1]);
		return (254);
	}
	server webserv(DEFAULT_CONFIGURATION_FILE);
	std::map<int, Client*> map;
	while (true) {
		int socketToUse;
		int _accept;
		struct sockaddr 	sockaddrClient;
		socklen_t 			socklenClient;
		if (kevent(webserv.getEvenementQueue(), NULL, 0, webserv.getTevent(), 1, NULL) == -1)
			ft_error("kevent event retrieval error", 8);
		if (static_cast<int>(webserv.getTevent()->ident) == webserv.getServerSocket()) {
			_accept = accept(webserv.getServerSocket(), &sockaddrClient, &socklenClient);
			if (_accept == -1)
				ft_error("accept creation failed", 5);
			else {
				if (map.find(_accept) == map.end()) {
					Client* new_client = new Client(_accept);
					map.insert(std::pair<int, Client*>(_accept, new_client));
					fcntl(_accept, F_SETFL, O_NONBLOCK);
					EV_SET(webserv.getevent(), _accept, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
					if (kevent(webserv.getEvenementQueue(), webserv.getevent(), 1, NULL, 0, NULL) == -1)
						ft_error("kevent event addition failed", 10);
				}
			}
			continue;
		}
		else {
			socketToUse = webserv.getTevent()->ident;
			std::map<int, Client*>::iterator it = map.find(socketToUse);
			if (it != map.end()) {
				it->second->receive_header();
				it->second->ResponseToClient();
				close(socketToUse);
				delete it->second;
				map.erase(socketToUse);
			}
		}
	}
	return (0);
}
