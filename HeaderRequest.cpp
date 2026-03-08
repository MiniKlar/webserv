/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:01:30 by lomont            #+#    #+#             */
/*   Updated: 2026/03/08 03:31:44 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderRequest.hpp"

HeaderRequest::HeaderRequest(std::string bufferHeader) {
	ParseHeaderRequest(bufferHeader);
	return ;
}

HeaderRequest::HeaderRequest(void) {
	return ;
}

//function to parse body
	// if (_method == POST) {
	// 	if (!bufferBody.empty()) {
	// 		std::string boundary;
	// 		size_t posboundary;
	// 		size_t endboundary;
	// 		//Content-Type: multipart/form-data; boundary=----geckoformboundarye4f3cf8d9bb9dca8928f017894d523c8
	// 		posboundary = bufferHeader.find("boundary=") + 9;
	// 		endboundary = bufferHeader.find("\r\n", posboundary);
	// 		boundary = "--" + bufferHeader.substr(posboundary, endboundary - posboundary);
	// 		CreateImage(bufferBody, boundary);
	// 	}
	// 	else {
	// 		//renvoyer une réponse erreur...
	// 		;
	// 	}
	// }

HeaderRequest::HeaderRequest(const HeaderRequest& other) {
	*this = other;
	return ;
}

HeaderRequest& HeaderRequest::operator=(const HeaderRequest& other) {
	if (&other != this) {
		this->Map = other.Map;
		this->_body = other._body;
		this->_method = other._method;
		this->_requestTarget = other._requestTarget;
		this->_startLine = other._startLine;
	}
	return (*this);
}

HeaderRequest::~HeaderRequest(void) {
	//ne pas oublier de détruire les objets pour libérer la mémoire
	return ;
}

std::string HeaderRequest::FindFileName(void) {
	std::string filename;
	std::ostringstream oss;
	for (int i = 1; i < 4; i++) {
		oss << "image" << i << ".png";
		filename = oss.str();
		if (access(filename.c_str(), F_OK) != 0)
			return (filename);
	}
	return ("image0.png");
	//système selon date d'ancienneté pour savoir qui remplacer?
}

void HeaderRequest::CreateImage(std::string& bufferBody, std::string& boundary) {
	if (bufferBody.length() > 4) {
		//extraire boundary depuis Content-type

		//retirer header;
		std::string name;
		std::string filename;
		std::string image_png;
		size_t startpos;
		size_t endpos;
		startpos = bufferBody.find("name=") + 6;
		endpos = bufferBody.find(";", startpos);
		name = bufferBody.substr(startpos, endpos - startpos - 1);
		//std::cout << name << std::endl;
		startpos = bufferBody.find("filename=") + 10;
		endpos = bufferBody.find("\r\n", startpos);
		filename = bufferBody.substr(startpos, endpos - startpos - 1);
		//std::cout << filename << std::endl;
		startpos = bufferBody.find("Content-Type:") + 14;
		endpos = bufferBody.find("\r\n", startpos);
		image_png = bufferBody.substr(startpos, endpos - startpos);
		//std::cout << image_png << std::endl;
		//récupérer index dernier \r\n + 4 (pour arriver apres fin de ligne) jusqu'à boundary end
		startpos = endpos + 4;
		std::string delimiter = "\r\n" + boundary + "--";
		endpos = bufferBody.find(delimiter, startpos);
		std::string image = bufferBody.substr(startpos, endpos - startpos);
		pathFileCreated = filename;
		//securiser filename
		std::string img_filename = UPLOAD_FOLDER + FindFileName();
		int image_fd = open(img_filename.c_str(), O_CREAT | O_TRUNC | O_WRONLY);
		if (image_fd == -1)
			ft_crash("creating image failed", 31);
		write(image_fd, image.data(), image.size());
		close(image_fd);
	};
}

void HeaderRequest::ParseHeaderRequest(std::string& bufferHeader) {
	std::pair<std::string, std::string> pair;
	std::string::iterator 				space;
	std::string::iterator 				begin;
	std::string::iterator 				end;
	std::string::iterator 				lastPositionToCheck;

	begin = bufferHeader.begin();
	end = std::find(begin, bufferHeader.end(), '\r');
	this->_startLine = std::string(begin, end);
	ParseStartLine();
	lastPositionToCheck = std::find(bufferHeader.begin(), bufferHeader.end(), '{');
	while (true) {
		begin = end + 2;
		space = std::find(begin, bufferHeader.end(), ' ');
		end = std::find(begin, bufferHeader.end(), '\r');
		if (end >= lastPositionToCheck || std::find(begin, space, ':') == space)
			break;
		pair.first = std::string(begin, space);
		pair.second = std::string(space + 1, end);
		Map.insert(pair);
	}
}

void HeaderRequest::ParseStartLine(void) {
	std::istringstream str (_startLine);

	str >> method >> _requestTarget;
	if (method == "POST")
		_method = POST;
	else if (method == "GET")
		_method = GET;
	else if (method == "DELETE")
		_method = DELETE;
	else
		_method = OTHER;
}

void HeaderRequest::printDebug(void) {
	std::cout << "Here's the method: '" << this->method << "' and the Request Target: '" << this->_requestTarget << "'" << std::endl;
	for (std::map<std::string, std::string>::iterator it = this->Map.begin(); it != this->Map.end(); it++) {
		std::cout << it->first << " " << it->second << std::endl;
	}
}

std::map<std::string, std::string>& HeaderRequest::getPairs(void) {
	return (this->Map);
}

std::string& HeaderRequest::GetRequestTarget(void) {
	return (this->_requestTarget);
}

std::string& HeaderRequest::GetPathImageCreated(void) {
	return (this->pathFileCreated);
}

Method HeaderRequest::GetMethod(void) {
	return (this->_method);
}

void HeaderRequest::SetBody(const std::string body) {
	this->_body = body.c_str();
}
