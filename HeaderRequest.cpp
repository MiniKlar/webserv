/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:01:30 by lomont            #+#    #+#             */
/*   Updated: 2026/03/22 09:52:05 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderRequest.hpp"

//Constructor

HeaderRequest::HeaderRequest(void) : method(OTHER), body(NULL), error(OK) {
	return ;
}

HeaderRequest::HeaderRequest(std::string bufferHeader) : method(OTHER), body(NULL), error(OK) {
	ParseHeaderRequest(bufferHeader);
	return ;
}

HeaderRequest::HeaderRequest(Error err) : method(OTHER), body(NULL), error(err) {
	return ;
}

//Copy constructor & assignement operator

HeaderRequest::HeaderRequest(const HeaderRequest& other) {
	*this = other;
	return ;
}

HeaderRequest& HeaderRequest::operator=(const HeaderRequest& other) {
	if (&other != this) {
		this->headerPair = other.headerPair;
		this->method = other.method;
		this->body = other.body;
	}
	return (*this);
}

//Destructor

HeaderRequest::~HeaderRequest(void) {
	//ne pas oublier de détruire les objets pour libérer la mémoire
	return ;
}


void HeaderRequest::ParseHeaderRequest(std::string& bufferHeader) {
	std::pair<std::string, std::string> pair;
	size_t				pos;
	size_t				mid;
	size_t				npos;
	size_t 				lastPositionToCheck;

	ParseFirstLine(bufferHeader);		//parse la première ligne
	if (error != OK)
		return ;
	lastPositionToCheck = bufferHeader.find("\r\n\r\n");
	if (lastPositionToCheck == std::string::npos) {
		error = BAD_REQUEST;
		return ;
	}
	pos = bufferHeader.find("\r\n") + 2;
	while (pos < lastPositionToCheck) {
		mid = bufferHeader.find(" ", pos);
		npos = bufferHeader.find("\r\n", mid);
		if (pos > npos) {
			error = BAD_REQUEST;
			return ;
		}
		pair.first = bufferHeader.substr(pos, mid - pos);
		pair.second = bufferHeader.substr((mid + 1), npos - (mid + 1));
		if (pair.first == "Connection:") {
			if (pair.second == "keep-alive")
				this->_delete = false;
			else
				this->_delete = true;
		}
		this->headerPair.insert(pair);
		pos = npos + 2;
	}
}

void HeaderRequest::ParseFirstLine(std::string& bufferHeader) {
	std::string			key;
	std::string			value;
	size_t				pos;
	size_t				npos;

	pos = 0;
	for (int i = 0; i < 3; i++) {
		if (i == 2) {
			pos = bufferHeader.find("/", pos) + 1;
			npos = bufferHeader.find("\r\n", pos);
			value = bufferHeader.substr(pos, npos - pos);
		}
		else {
			npos = bufferHeader.find(" ", pos);
			value = bufferHeader.substr(pos, npos - pos);
		}
		if (pos == std::string::npos || npos == std::string::npos) {
			error = BAD_REQUEST;
			return ;
		}
		switch (i)
		{
			case 0:
				key = "Method:";
				break;
			case 1:
				key = "Request-Target:";
				break;
			case 2:
				key = "HTTP:";
				break;
		}
		this->headerPair.insert(std::pair<std::string, std::string>(key, value));
		pos = npos + 1;
	}
	SetMethod(headerPair["Method:"]);
}

void HeaderRequest::CleanHeader(void) {
	this->body = NULL;
	this->error = OK;
	this->headerPair.clear();
	this->method = OTHER;
	this->_delete = false;
}

//Getters

std::map<std::string, std::string>& HeaderRequest::getPairs(void) {
	return (this->headerPair);
}

Method HeaderRequest::GetMethod(void) {
	return (this->method);
}

Error HeaderRequest::GetError(void) {
	return (this->error);
}

bool HeaderRequest::GetDeleteSocket(void) {
	return (this->_delete);
}

std::string& HeaderRequest::GetImagePath(void) {
	return (this->pathImageCreated);
}

//Setters

void HeaderRequest::SetBody(const std::string body) {
	this->body = body.c_str();
	//extraire boundary depuis Content-type
	std::string contentType = this->headerPair["Content-Type:"];
	std::string	boundary = contentType.substr(contentType.find("boundary=") + 10, contentType.find("\r\n") - contentType.find("boundary=") + 10);
	logs(boundary);
	CreateImage(body, boundary);
}

void HeaderRequest::SetMethod(std::string& method) {
	if (method == "POST")
		this->method = POST;
	else if (method == "GET")
		this->method = GET;
	else if (method == "DELETE")
		this->method = DELETE;
	else
		this->method = OTHER;
}

void HeaderRequest::SetError(Error err) {
	this->error = err;
}

std::string HeaderRequest::FindFileName(void) {
	std::string filename = ".";
	std::ostringstream oss;
	for (int i = 0; i < 4; i++) {
		oss << "image" << i << ".png";
		filename += UPLOAD_FOLDER + oss.str();
		if (access(filename.c_str(), F_OK) != 0)
			return (oss.str());
		oss.str("");
		filename = ".";
	}
	return ("image0.png");
	//système selon date d'ancienneté pour savoir qui remplacer?
}

void HeaderRequest::CreateImage(const std::string& bufferBody, std::string& boundary) {
	if (boundary.length() == 0)
		return ; //bool pour error?
	if (bufferBody.length() > 4) {
		std::string name;
		std::string filename;
		std::string image_png;
		size_t startpos;
		size_t endpos;

		startpos = bufferBody.find("name=") + 6;
		endpos = bufferBody.find(";", startpos);
		name = bufferBody.substr(startpos, endpos - startpos - 1);
		logs(name);
		startpos = bufferBody.find("filename=") + 10;
		endpos = bufferBody.find("\r\n", startpos);
		filename = bufferBody.substr(startpos, endpos - startpos - 1);
		logs(filename);
		startpos = bufferBody.find("Content-Type:") + 14;
		endpos = bufferBody.find("\r\n", startpos);
		image_png = bufferBody.substr(startpos, endpos - startpos);
		logs(image_png);

		//récupérer index dernier \r\n + 4 (pour arriver apres fin de ligne) jusqu'à boundary end
		startpos = endpos + 4;
		std::string delimiter = "\r\n" + boundary + "--";
		logs(delimiter);
		endpos = bufferBody.find(delimiter, startpos);
		std::string image = bufferBody.substr(startpos, endpos - startpos);

		//securiser filename
		std::string img_filename = UPLOAD_FOLDER + FindFileName();
		pathImageCreated = img_filename;
		img_filename = "." + img_filename;
		int image_fd = open(img_filename.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IROTH | S_IRUSR | S_IRGRP);
		if (image_fd == -1)
			ft_crash("creating image failed", 31);
		write(image_fd, image.data(), image.size());
		close(image_fd);
	};
}
