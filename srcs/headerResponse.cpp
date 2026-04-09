/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   headerResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/05 22:42:38 by lomont            #+#    #+#             */
/*   Updated: 2026/04/09 22:46:23 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headerResponse.hpp"
#include "webserv.hpp"

//Constructors

HeaderResponse::HeaderResponse(HeaderRequest& setRequest, struct config* setConfig) : cookie(false), error(false), parsed(false), isCGI(false), bodySize(0), indexLocationConfig(0), header(""), buffer(""), bodySizePrint("0"), pathfile(""), config(setConfig), request(setRequest){
	CreateResponse();
}

HeaderResponse::HeaderResponse(void) : cookie(false), error(false), parsed(false), isCGI(false), bodySize(0), indexLocationConfig(0), header(""), buffer(""), bodySizePrint(""), pathfile(""), config(NULL), request(HeaderRequest()) {
}

//Copy constructor & copy assignement

HeaderResponse::HeaderResponse(const HeaderResponse& other) {
	*this = other;
}

HeaderResponse& HeaderResponse::operator=(const HeaderResponse& other) {
	if (&other != this) {
		this->request = other.request;
		this->config = other.config;
		this->buffer = other.buffer;
		this->bodySizePrint = other.bodySizePrint;
		this->pathfile = other.pathfile;
		this->indexLocationConfig = other.indexLocationConfig;
		this->bodySize = other.bodySize;
		this->error = other.error;
		this->isCGI = other.isCGI;
		this->parsed = other.parsed;
		this->cookie = other.cookie;
	}
	return *this;
}

//Destructor

HeaderResponse::~HeaderResponse(void) {
}

//Member functions

void HeaderResponse::CreateResponse(void) {
	FindFileLocation(); //On cherche la location du fichier demandé grâce à son path et aux routes décrites dans le fichier de configuration webserv
	CheckMethod(); //On check que la méthode est bien implémenté et accepté par la route demandé

//Selon la route on va choisir les fonctions à utiliser
	if (isCGI)
		HandleCGI();
	else if (this->request.GetMethod() == GET && !isCGI)
		HandleGet();
	else if (this->request.GetMethod() == DELETE)
		HandleDelete();
	else if (this->request.GetMethod() == POST && !isCGI)
		HandlePost();
	if (!this->error && !isCGI)
		GetHeaderResponse();
	if (this->header.empty()) {
		if (pathfile == "ERROR") {
			SearchErrorPage();
			if (!pathfile.empty()) {
				SetFileSize();
				OpenFile();
			}
			else {
				this->buffer = default_error_page(this->request.GetError());
				this->bodySize = buffer.size();
				std::ostringstream oss;
				oss << this->bodySize;
				this->bodySizePrint = oss.str();
			}
		}
		this->header = CheckErrors();
	}
	if (cookie) {
		size_t pos;
		pos = header.find("\r\n\r\n");
		header.erase(pos, 2);
		header.append("Set-Cookie: session_auth=67; Path=/; Max-Age=3600");
		header.append("\r\n\r\n");
	}
	buffer = header + buffer;
	parsed = true;
}

void HeaderResponse::FindFileLocation(void) {
	struct LocationConfig* ptr = this->config->locationConfig;
	if (!ptr) {
		indexLocationConfig = -1;
		return ;
	}
	size_t pos = 0;
	std::string str = this->request.getPairs()["Request-Target:"];
	for (size_t i = 0; i < this->config->numbersOfLocation; i++) {
		if (str.compare(0, ptr[i].location.size(), ptr[i].location) == 0) {
			if (ptr[i].location.size() > pos) {
				pos = ptr[i].location.size();
				indexLocationConfig = i;
			}
		}
		else if (ptr[i].location.size() > 0 && ptr[i].location[ptr[i].location.size() - 1] == '/') {
			std::string match = ptr[i].location.substr(0, ptr[i].location.size() - 1);
			if (match == str) {
				if (ptr[i].location.size() > pos) {
					pos = ptr[i].location.size();
					indexLocationConfig = i;
					this->request.getPairs()["Request-Target:"] += "/";
				}
			}
		}
	}
	//checkCGI here
	for (std::map<std::string, std::string>::iterator it = ptr[indexLocationConfig].cgi_handlers.begin(); it != ptr[indexLocationConfig].cgi_handlers.end(); it++) {
		if (str.find(it->first) != std::string::npos)
			isCGI = true;
	}
}

void HeaderResponse::CheckMethod(void) {
	//check si la méthode fait parti des méthodes prises en charge par webserv
	std::string headerMethod = this->request.getPairs()["Method:"];
	if (headerMethod != "GET" && headerMethod != "POST" && headerMethod != "DELETE") {
		ft_warning("Method not implemented");
		this->request.SetError(NOT_IMPLEMENTED);
		error = true;
		return ;
	}
	//check si le header est pris en charge par la location
	bool methodAccepted = false;
	if (this->config->locationConfig) {
		std::vector<std::string> methods = this->config->locationConfig[indexLocationConfig].methods;
		for (std::vector<std::string>::iterator it = methods.begin(); it != methods.end(); it++) {
			if (*it == headerMethod)
				methodAccepted = true;
		}
		if (!methodAccepted) {
			if (!this->config->locationConfig[indexLocationConfig].location.empty()) {
				ft_warning("Location moved permanently");
				this->request.SetError(MOVED_PERMANENTLY);
				this->error = true;
				pathfile = "ERROR";
			}
			else {
				ft_warning("Method not allowed by the route");
				this->request.SetError(NOT_ALLOWED);
				this->request.SetDeleteRequest(true);
			}
			error = true;
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
}

void HeaderResponse::SearchErrorPage(void) {
	struct stat s;
	int errorCode = this->request.GetError();
	std::map<std::vector<int>, std::string>::iterator it = this->config->errorPage.begin();

	for (; it != this->config->errorPage.end(); it++) {
		const std::vector<int>& errorCodesKeys = it->first;
		if (std::find(errorCodesKeys.begin(), errorCodesKeys.end(), errorCode) != errorCodesKeys.end()) {
			pathfile = "." + it->second;
			if (access(pathfile.c_str(), F_OK) == 0) {
				if (stat(pathfile.c_str(), &s) == -1 || s.st_mode & S_IFDIR) {
					pathfile.clear();
					continue;
				}
				pathfile = it->second;
				return ;
			}
			pathfile.clear();
		}
	}
	pathfile.clear();
}

void HeaderResponse::OpenFile(void) {
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
	if (this->bodySize == 0) {
		close(file);
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
	delete[] buffer;
}

//This function searches if a pathname is available, or it finds the oldest file to overwrite it
std::string HeaderResponse::FindFileName(void) {
	std::string 		oldestFile;
	std::string 		filename;
	std::ostringstream 	oss;
	struct stat			s_stat;
	timespec			t_time;

	t_time.tv_sec = 0;
	t_time.tv_nsec = 0;
	for (int i = 0; i < 3; i++) {
		oss << "image" << i << ".png";
		filename = std::string(".") + UPLOAD_FOLDER + oss.str();
		if (access(filename.c_str(), F_OK) != 0)
			return (oss.str());
		if (stat(filename.c_str(), &s_stat) == 0) {
				if (s_stat.st_mtim.tv_sec < t_time.tv_sec
						|| (s_stat.st_mtim.tv_sec == t_time.tv_sec && s_stat.st_mtim.tv_nsec < t_time.tv_nsec)
							|| t_time.tv_sec == 0) {
						oldestFile = oss.str();
						t_time.tv_sec = s_stat.st_mtim.tv_sec;
						t_time.tv_nsec = s_stat.st_mtim.tv_nsec;
				}
		}
		oss.str("");
	}
	return oldestFile;
}

void HeaderResponse::FindPathFile(void) {
	if (error)
		return ;
	std::string str = this->request.getPairs()["Request-Target:"];
	std::string uri;
	std::string root;
	if (this->config->locationConfig) {
		uri = this->config->locationConfig[indexLocationConfig].location;
		root = this->config->locationConfig[indexLocationConfig].root;
	}
	this->pathfile = root + str;
}

std::string HeaderResponse::CheckErrors(void) {
	switch (this->request.GetError())
	{
		case OK:
			return "";
		case MOVED_PERMANENTLY:
			return code_301();
		case BAD_REQUEST:
			return code_400();
		case FORBIDDEN:
			return code_403();
		case NOT_FOUND:
			return code_404();
		case NOT_ALLOWED:
			return code_405();
		case LENGTH:
			return code_411();
		case BODY_TOO_LARGE:
			return code_413();
		case INTERNAL:
			return code_500();
		case NOT_IMPLEMENTED:
			return code_501();
		case VERSION:
			return code_505();
	}
	return std::string("");
}

void HeaderResponse::CleanHeader(void) {
	this->config = NULL;
	this->header.clear();
	this->buffer.clear();
	this->bodySizePrint.clear();
	this->pathfile.clear();
	this->indexLocationConfig = -1;
	this->bodySize = 0;
	this->error = false;
	this->parsed = false;
}

//Getters

std::string HeaderResponse::GetContentType(void) {
	// return (TEXT_TYPE);
	if (this->pathfile.find(".png") != std::string::npos)
		return ("image/png");
	else
		return (HTML_TYPE);
}

std::string HeaderResponse::GetBuffer(void) {
	return (this->buffer);
}

std::string HeaderResponse::GetConnectionStatut(void) {
	std::map<std::string, std::string>& map = this->request.getPairs();
	std::map<std::string, std::string>::iterator it;
	it = map.find("Connection:");
	if (it != map.end())
		return ("Connection: " + it->second + "\r\n");
	return ("");
}

std::string HeaderResponse::GetMethodAllowed(void) {
	//location doit être une référence trouvé
	std::vector<std::string>	tmp;
	std::string					methods;

	if (this->config->locationConfig) {
		tmp = this->config->locationConfig[indexLocationConfig].methods;
		for (size_t i = 0; i < tmp.size(); i++) {
			methods += tmp[i];
			if (i + 1 < tmp.size())
				methods += " ";
		}
	}
	return methods;
}

std::string HeaderResponse::GetMaxBodySize(void) {
	std::ostringstream oss;

	oss << this->config->maxBodySize;
	return oss.str();
}

bool HeaderResponse::IsParsed(void) {
	return this->parsed;
}

std::string HeaderResponse::GetCurrentTime( void ) {
	time_t currentTime;
	struct tm* localTime;
	char ptr[1024];
	currentTime = time(NULL);
	localTime = gmtime(&currentTime);
	if (strftime(ptr, 1024, "%a, %d %b %Y %X GMT", localTime) == 0) {
		std::cerr << "error time" << std::endl;
		exit(15);
	}
	return std::string(ptr);
}

std::string HeaderResponse::GetNewLocation(void) {
	return this->config->locationConfig[indexLocationConfig]._return.second;
}

//Setters

void HeaderResponse::SetBuffer(std::string str) {
	this->buffer = str;
}

void HeaderResponse::SetFileSize(void) {
	struct stat			s;
	std::ostringstream	oss;
	std::string			size;

	if (pathfile[0] != '.')
		pathfile = "." + pathfile;
	if (stat(pathfile.c_str(), &s) == -1) {
		ft_warning("Error stat");
		pathfile = "ERROR";
		error = true;
		this->request.SetError(NOT_FOUND);
		return ;
	}
	if (S_ISDIR(s.st_mode)) {
		pathfile = "ERROR";
		this->request.SetError(NOT_FOUND);
		error = true;
	}
	if (pathfile != "ERROR") {
		bodySize = s.st_size;
		oss << s.st_size;
		bodySizePrint = oss.str();
	}
}

void HeaderResponse::SetResponseError(Error err) {
	this->error = true;
	this->request.SetError(err);
	this->pathfile = "ERROR";
}
