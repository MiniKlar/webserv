/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:01:30 by lomont            #+#    #+#             */
/*   Updated: 2026/02/19 04:09:49 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderRequest.hpp"

HeaderRequest::HeaderRequest(char buffer[1024], ssize_t bytes_read) {
	std::string _header (buffer, bytes_read);
	ParseHeaderRequest(_header);
	return ;
}

HeaderRequest::~HeaderRequest(void) {
	//ne pas oublier de détruire les objets pour libérer la mémoire
	return ;
}

void HeaderRequest::ParseHeaderRequest(std::string& _header) {
	std::pair<std::string, std::string> pair;
	std::string::iterator 				space;
	std::string::iterator 				begin;
	std::string::iterator 				end;
	std::string::iterator 				lastPositionToCheck;

	begin = _header.begin();
	end = std::find(begin, _header.end(), '\r');
	this->_startLine = std::string(begin, end);
	ParseStartLine();
	lastPositionToCheck = std::find(_header.begin(), _header.end(), '{');
	while (true) {
		begin = end + 2;
		space = std::find(begin, _header.end(), ' ');
		end = std::find(begin, _header.end(), '\r');
		if (end >= lastPositionToCheck || std::find(begin, space, ':') == space)
			break;

		pair.first = std::string(begin, space);
		pair.second = std::string(space + 1, end);
		Map.push_back(pair);
	}
	return ;
}

void HeaderRequest::ParseStartLine(void) {
	std::istringstream str (this->_startLine);
	str >> this->_method >> this->_requestTarget;
}

void HeaderRequest::printDebug(void) {
	std::cout << "Here's the method: '" << this->_method << "' and the Request Target: '" << this->_requestTarget << "'" << std::endl;
	for (std::vector<std::pair<std::string, std::string> >::iterator it = this->Map.begin(); it != this->Map.end(); it++) {
		std::cout << it->first << " " << it->second << std::endl;
	}
}
