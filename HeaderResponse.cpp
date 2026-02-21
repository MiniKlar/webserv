/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:28 by lomont            #+#    #+#             */
/*   Updated: 2026/02/21 04:55:45 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderResponse.hpp"

HeaderResponse::HeaderResponse(HeaderRequest& _request) : _request(_request) {
	return;
}

HeaderResponse::~HeaderResponse(void)
{
	return;
}

std::string HeaderResponse::getCurrentTime( void ) {
	time_t currentTime;
	struct tm* localTime;
	char ptr[1024];
	currentTime = time(NULL);
	localTime = localtime(&currentTime);
	if (strftime(ptr, 1024, "%a, %d %b %G %T %Z", localTime) == 0) {
		std::cerr << "error time" << std::endl;
		exit(15);
	}
	std::cout << ptr << std::endl;
	return (std::string(ptr));
}

std::string HeaderResponse::getContentLength(void) {
	struct stat s;
	char ptr[1024];
	if (stat("test.html", &s) == -1) {
		std::cerr << "error stat" << std::endl;
		exit(16);
	}
	if (sprintf(ptr, "%lld", s.st_size) < 0)
	{
		std::cerr << "error stat" << std::endl;
		exit(17);
	}
	return (std::string(ptr));
}

std::string HeaderResponse::getConnectionStatut(void) {
	std::vector<std::pair<std::string, std::string> >& map = this->_request.getPairs();
	std::vector<std::pair<std::string, std::string> >::iterator it;
	it = std::find_if(map.begin(), map.end(), [](const std::pair<std::string, std::string>& p) {return p.first == "Connection:";});
	if (it != map.end())
		return (it->first + " " + it->second);
	return ("");
}

std::string HeaderResponse::code_200( void ) {
	return ("HTTP/1.1 200 OK\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: " + getContentLength() + "\r\n" + getConnectionStatut() + "\r\n");
}

// HTTP/1.1 200 OK
// Date: Wed, 18 Feb 2026 22:00:00 GMT
// Server: Apache/2.4.41 (Ubuntu)
// Content-Type: text/html; charset=UTF-8
// Content-Length: 138
// Connection: keep-alive
