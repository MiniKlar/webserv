/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/19 04:02:28 by lomont            #+#    #+#             */
/*   Updated: 2026/03/22 13:23:08 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HeaderResponse.hpp"
#include "webserv.hpp"

//Constructors

HeaderResponse::HeaderResponse(HeaderRequest& setRequest, struct config* setConfig) : request(setRequest), config(setConfig), error(false), parsed(false) {
	FindLocation();
	CheckMethod();
	if (this->request.GetMethod() != POST) {
		FindPath(); //find path file
		SetBodySize(); //set size of the file requested
		if (!this->error)
			OpenBodyFile();
	}
	else {
		pathfile = this->request.GetImagePath();
	}
	//if location old page
	//if autoindex
	//if cgi pass
	if (!this->error)
		GetHeaderResponse();
	if (this->header.empty())
		this->header = CheckErrors();
	std::cout << "header = [" << header << "]" << std::endl;
	buffer = header + buffer;
	std::cout << "buffer après avoir rajouté header = [" << buffer.size() << "]" << std::endl;
	parsed = true;
	return ;
}

HeaderResponse::HeaderResponse(void) : request(HeaderRequest()), config(NULL) {
	return ;
}

//Copy constructor & copy assignement

HeaderResponse::HeaderResponse(const HeaderResponse& other) {
	*this = other;
	return ;
}

HeaderResponse& HeaderResponse::operator=(const HeaderResponse& other) {
	if (&other != this) {
		this->request = other.request;
		this->config = other.config;
		this->buffer = other.buffer;
		this->bodySizePrint = other.bodySizePrint;
		this->bodySize = other.bodySize;
		this->pathfile = other.pathfile;
		this->indexLocationConfig = other.indexLocationConfig;
		this->error = other.error;
		this->parsed = other.parsed;
	}
	return (*this);
}

//Destructor

HeaderResponse::~HeaderResponse(void) {
	return;
}

void HeaderResponse::FindLocation(void) {
	struct LocationConfig*	ptr;
	std::string str;
	size_t		pos;

	pos = 0;
	str = this->request.getPairs()["Request-Target:"];
	ptr = this->config->locationConfig;
	std::cout << "voici str = [" << str << "]" << std::endl;
	for (size_t i = 0; i < this->config->numbersOfLocation; i++) {
		std::cout << "location = [" << ptr[i].location << "]" << std::endl;
		if (str.compare(0, ptr[i].location.size(), ptr[i].location) == 0) {
			if (ptr[i].location.size() > pos) {
				pos = ptr[i].location.size();
				indexLocationConfig = i;
			}
		}
	}
}

void HeaderResponse::GetHeaderResponse(void) {
	Method method = request.GetMethod();
	if (method == GET)
		this->header = code_200();
	else if (method == POST)
		this->header = code_201();
	else if (method == DELETE)
		this->header = code_204();
	return ;
}

void HeaderResponse::OpenBodyFile(void) {
	int		file;
	int		bread;
	char 	*buffer;

	buffer = NULL;
	file = open(pathfile.c_str(), O_RDONLY);
	if (file == -1) {
		this->request.SetError(INTERNAL);
		error = true;
		return ;
	}
	buffer = new char[bodySize + 1];
	if (!buffer) {
		this->request.SetError(INTERNAL);
		error = true;
		close(file);
		return ;
	}
	bread = read(file, buffer, bodySize);
	std::cout << "bread[" << bread << "]" << std::endl;
	if (bread <= 0) {
		delete[] buffer;
		this->request.SetError(INTERNAL);
		error = true;
		close(file);
		return ;
	}
	buffer[bread] = '\0';
	close(file);
	this->buffer.append(buffer, bread);
	return;
}

void HeaderResponse::CheckMethod(void) {
	std::vector<std::string> methods;
	std::string				headerMethod;
	bool					methodAccepted;

	methodAccepted = false;
	headerMethod = this->request.getPairs()["Method:"];
	//check si le header fait parti des headers pris en charge par webserv
	if (headerMethod != "GET" && headerMethod != "POST" && headerMethod != "DELETE") {
		std::cout << "method not implemented" << std::endl;
		this->request.SetError(NOT_IMPLEMENTED);
		error = true;
		return ;
	}
	//check si le header est pris en charge par la location
	methods = this->config->locationConfig[indexLocationConfig].methods;
	for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
		std::cout << "method = [" << *it << "]" << std::endl;
		if (*it == headerMethod)
			methodAccepted = true;
	}
	if (methodAccepted == false) {
		std::cout << "method not implemented" << std::endl;
		this->request.SetError(NOT_ALLOWED);
		error = true;
	}
	return;
}

void HeaderResponse::CleanHeader(void) {
	this->bodySize = 0;
	this->bodySizePrint.clear();
	this->buffer.clear();
	this->config = NULL;
	this->error = false;
	this->header.clear();
	this->indexLocationConfig = -1;
	this->parsed = false;
	this->pathfile.clear();
}

void HeaderResponse::FindPath(void) {
	std::string str;
	std::string uri;
	std::string root;

	if (error)
		return ;
	str = this->request.getPairs()["Request-Target:"];
	uri = this->config->locationConfig[indexLocationConfig].location;
	std::cout << "uri = [" << uri << "]" << std::endl;
	root = this->config->locationConfig[indexLocationConfig].root;
	std::cout << "root = [" << root << "]" << std::endl;
	this->pathfile = root + str;
	std::cout << "pathfile = [" << pathfile << "]" << std::endl;
	return ;
}

std::string HeaderResponse::CheckErrors(void) {
	if (!this->header.empty())
		return (header);
	switch (this->request.GetError())
	{
		case OK:
			return ("");
		case BAD_REQUEST :
			return (code_400());
		case NOT_FOUND :
			return (code_404());
		case NOT_ALLOWED :
			return (code_405());
		case LENGTH :
			return (code_411());
		case INTERNAL :
			return (code_500());
		case NOT_IMPLEMENTED :
			return (code_501());
		case VERSION :
			return (code_505());
	}
	return ("");
}

std::string HeaderResponse::getCurrentTime( void ) {
	time_t currentTime;
	struct tm* localTime;
	char ptr[1024];
	currentTime = time(NULL);
	localTime = gmtime(&currentTime);
	if (strftime(ptr, 1024, "%a, %d %b %Y %X GMT", localTime) == 0) {
		std::cerr << "error time" << std::endl;
		exit(15);
	}
	return (std::string(ptr));
}

void HeaderResponse::SetBuffer(std::string str) {
	this->buffer = str;
	return ;
}

void HeaderResponse::SetBodySize(void) {
	struct stat s;
	std::ostringstream oss;
	std::string size;

	pathfile = "." + pathfile;
	if (error)
		return ;
	if (stat(pathfile.c_str(), &s) == -1) {
		if (stat(DEFAULT_ERROR_PAGE, &s) == -1)
			return; //peut etre hard code une réponse d'erreur?
		else {
			this->request.SetError(NOT_FOUND);
			pathfile = DEFAULT_ERROR_PAGE;
			error = true;
		}
	}
	if (s.st_mode & S_IFDIR) {
		if (stat(DEFAULT_ERROR_PAGE, &s) == -1)
			return; //peut etre hard code une réponse d'erreur?
		this->request.SetError(NOT_FOUND);
		pathfile = DEFAULT_ERROR_PAGE;
		error = true;
	}
	bodySize = s.st_size;
	oss << s.st_size;
	bodySizePrint = oss.str();
	std::cout << "bodySizePrint = [" << bodySizePrint << "]" << std::endl;
	std::cout << "error=" << error << std::endl;
	return ;
}

bool HeaderResponse::IsParsed(void) {
	return (this->parsed);
}

std::string HeaderResponse::getConnectionStatut(void) {
	std::map<std::string, std::string>& map = this->request.getPairs();
	std::map<std::string, std::string>::iterator it;
	it = map.find("Connection:");
	if (it != map.end())
		return ("Connection: " + it->second);
	return ("");
}

std::string HeaderResponse::GetMethodAllowed(void) {
	//location doit être une référence trouvé
	std::vector<std::string>	tmp;
	std::string					methods;

	tmp = this->config->locationConfig[indexLocationConfig].methods;
	for (size_t i = 0; i < tmp.size(); i++) {
		methods += tmp[i];
		if (i + 1 < tmp.size())
			methods += " ";
	}
	return (methods);
}

std::string HeaderResponse::code_200( void ) {
	return ("HTTP/1.1 200 OK\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\n" + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + getConnectionStatut() + "\r\n\r\n"); //choisir le type selon ce qu'on envoie => selon s'il y a un body.
}

std::string HeaderResponse::code_201(void) {
	return ("HTTP/1.1 201 Created\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\nLocation: " + pathfile + "\r\nContent-Type: " + GetContentType() + "\r\nContent-Length: 0" + "\r\n\r\n");
}

std::string HeaderResponse::code_204(void) {
	return ("HTTP/1.1 204 No Content\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_400(void) {
	return ("HTTP/1.1 400 Bad Request\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\n" + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_404(void) {
	return ("HTTP/1.1 404 Not Found\r\nDate: "
	+ this->getCurrentTime() + "\r\nServer: Webserv\r\n"+ GetContentType() +"\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + getConnectionStatut() + "\r\n\r\n"); //récupérer taille page erreur ?
}

std::string HeaderResponse::code_405(void) {
	return ("HTTP/1.1 405 Method Not Allowed\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\nContent-Length: 0\r\nAllow: " + GetMethodAllowed() + "\r\n\r\n");
}

std::string HeaderResponse::code_411(void) {
	return ("HTTP/1.1 411 Length Required\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\n" + GetContentType() + "\r\nContent-Length: "
	+ bodySizePrint + "\r\n" + getConnectionStatut() + "\r\n\r\n");
}

std::string HeaderResponse::code_500(void) {
	return ("HTTP/1.1 500 Internal Server Error\r\n" + GetContentType() + "\r\nContent-Length: " + bodySizePrint + "\r\n\r\n");
}

std::string HeaderResponse::code_501(void) {
	return ("HTTP/1.1 501 Not Implemented\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\n\r\n");
}

std::string HeaderResponse::code_505(void) {
	return ("HTTP/1.1 505 HTTP Version Not Supported\r\nDate: " + this->getCurrentTime() + "\r\nServer: Webserv\r\n\r\n");
}

//Getters

std::string HeaderResponse::GetContentType(void) {
	if (this->request.GetMethod() == POST)
		return (IMG_TYPE);
	else
		return (HTML_TYPE);
}

std::string HeaderResponse::GetBuffer(void) {
	return (this->buffer);
}

// HTTP/1.1 201 Created
// Date: Wed, 18 Feb 2026 22:01:00 GMT
// Server: nginx/1.18.0
// Location: /api/users/43
// Content-Type: application/json
// Content-Length: 51

// HTTP/1.1 200 OK
// Date: Wed, 18 Feb 2026 22:00:00 GMT
// Server: Apache/2.4.41 (Ubuntu)
// Content-Type: text/html; charset=UTF-8
// Content-Length: 138
// Connection: keep-alive

// HTTP/1.1 204 No Content
// Date: Wed, 18 Feb 2026 22:02:00 GMT
// Server: nginx/1.18.0
// Connection: keep-alive
