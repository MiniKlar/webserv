/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   headerResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/05 22:42:38 by lomont            #+#    #+#             */
/*   Updated: 2026/04/08 00:58:22 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headerResponse.hpp"
#include "webserv.hpp"

//Constructors

HeaderResponse::HeaderResponse(HeaderRequest& setRequest, struct config* setConfig) : cookie(false), error(false), parsed(false), isCGI(false), bodySize(0), indexLocationConfig(0), header(""), buffer(""), bodySizePrint("0"), pathfile(""), config(setConfig), request(setRequest){
	//Create the HeaderResponse depending the method and/or the file is a CGI
	CreateResponse();
	return ;
}

HeaderResponse::HeaderResponse(void) : cookie(false), error(false), parsed(false), isCGI(false), bodySize(0), indexLocationConfig(0), header(""), buffer(""), bodySizePrint(""), pathfile(""), config(NULL), request(HeaderRequest()) {
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
		this->pathfile = other.pathfile;
		this->indexLocationConfig = other.indexLocationConfig;
		this->bodySize = other.bodySize;
		this->error = other.error;
		this->isCGI = other.isCGI;
		this->parsed = other.parsed;
		this->cookie = other.cookie;
	}
	return (*this);
}

//Destructor

HeaderResponse::~HeaderResponse(void) {
	return;
}

//Member functions

void HeaderResponse::CreateResponse(void) {
	//On cherche la location du fichier demandé grâce à son path et aux routes décrites dans le fichier de configuration webserv
	FindFileLocation();
	//On check que la méthode est bien implémenté et accepté par la route demandé
	CheckMethod();
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
	std::cout << "header size = " << header.size() << std::endl;
	buffer = header + buffer;
	parsed = true;
}

void HeaderResponse::HandleCGI(void) {
	size_t				pos;
	std::ostringstream	oss;
	std::string			header;
	std::string			buffer;
	std::string			cgi_headers;

	FindPathFile();
	buffer = PerformCGI();
	if (buffer.empty()) {
		this->error = true;
		this->request.SetError(INTERNAL);
		pathfile = "ERROR";
		return ;
	}
	pos = buffer.find("\r\n\r\n");
	if (pos == std::string::npos) {
		this->error = true;
		this->request.SetError(INTERNAL);
		pathfile = "ERROR";
		return ;
	}
	pos += 4;

	cgi_headers = buffer.substr(0, pos);
	this->buffer = buffer.substr(pos);

	this->bodySize = this->buffer.size();
	oss << bodySize;
	this->bodySizePrint = oss.str();

	this->header = "HTTP/1.1 200 OK\r\n";
    this->header += "Date: " + this->GetCurrentTime() + "\r\n";
    this->header += "Server: Webserv\r\n";
    this->header += "Content-Length: " + this->bodySizePrint + "\r\n";
	this->header += cgi_headers;
	parsed = true;
	return ;
}

void HeaderResponse::HandleGet(void) {
	struct stat 				s;
	std::stringstream			ss;
	std::vector<std::string>	index;
	std::string					path;
	bool						found;

	found = false;
	if (this->request.getPairs()["Request-Target:"] == "/get-cookie") {
		bodySize = 0;
		bodySizePrint = "0";
		cookie = true;
		return ;
	}
	FindPathFile(); //trouver le chemin complet du fichier
	if (this->config->locationConfig) {
		index = this->config->locationConfig[indexLocationConfig].index;
		if (this->pathfile[this->pathfile.length() - 1] == '/') {
			if (!index.empty()) {
				for (std::vector<std::string>::iterator it = index.begin(); it != index.end(); it++) {
					path = std::string(".") + this->pathfile + *it;
					std::cout << "voici patfile = " << pathfile << " et voici *it = " << *it << std::endl;
					std::cout << "voici index file = " << path << std::endl;
					if (access(path.c_str(), F_OK) == 0) {
						std::cout << "access autorisé" << std::endl;
						if (stat(path.c_str(), &s) != -1 && S_ISREG(s.st_mode)) {
							std::cout << "c'est un file'" << std::endl;
							pathfile = path;
							found = true;
							break;
						}
					}
				}
			}
			if (!found) {
				if (this->config->locationConfig[indexLocationConfig].autoindex == true) {
					HandleAutoIndex();
					this->bodySize = buffer.size();
					ss << bodySize;
					this->bodySizePrint = ss.str();
					return ;
				}
			}
		}
	}
	SetFileSize(); //Trouver et set la taille du fichier qu'on va renvoyer
	if (!this->error) //S'il n'y a pas eu d'erreur, alors on peut essayer d'ouvrir le fichier
		OpenFile();
	return ;
}

void HeaderResponse::HandleAutoIndex(void) {
	std::string	header;
	std::string	pathfile;
	std::string buffer;

	pathfile = this->pathfile;
	header = "<html>\n<head><title>Index of " + pathfile + "</title></head>\n<body>\n<h1>Index of " + pathfile + "</h1><hr>\n<table width=\"100%\">\n<tr style=\"text-align: left;\"><th>Name</th><th>Last Modified</th><th>Size</th></tr>\n";
	buffer = PerformListing(pathfile);
	if (buffer.empty()) {
		this->error = true;
		this->request.SetError(INTERNAL);
		this->pathfile = "ERROR";
		return ;
	}
	this->buffer = header + buffer + "</table>\n<hr>\n</body>\n</html>";
	std::cout << "size of buffer after listing = " << this->buffer.size() << std::endl;
}

void HeaderResponse::HandleDelete(void) {
	FindPathFile();
	DeleteFile();
	return ;
}

void HeaderResponse::HandlePost(void) {
	if (this->request.GetAuthorized() == false) {
		this->error = true;
		this->request.SetError(FORBIDDEN);
		pathfile = "ERROR";
		return ;
	}
	else if (this->request.GetError() != OK) {
		this->error = true;
		return ;
	}
	ParseBody();
	return ;
}

void HeaderResponse::DeleteFile(void) {
	std::string path = "." + pathfile;
	if (remove(path.c_str()) == -1) {
		ft_warning("Error: can't delete file; file not found");
		this->request.SetError(NOT_FOUND);
		pathfile = "ERROR";
		this->error = true;
		return;
	}
	ft_logs("File deleted");
	return ;
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

void HeaderResponse::FindFileLocation(void) {
	struct LocationConfig*	ptr;
	std::string 			str;
	size_t					pos;

	pos = 0;
	str = this->request.getPairs()["Request-Target:"];
	ptr = this->config->locationConfig;
	if (!ptr) {
		indexLocationConfig = -1;
		return ;
	}
	for (size_t i = 0; i < this->config->numbersOfLocation; i++) {
		if (str.compare(0, ptr[i].location.size(), ptr[i].location) == 0) {
			if (ptr[i].location.size() > pos) {
				std::cout << "location basique = " << ptr[i].location << std::endl;
				pos = ptr[i].location.size();
				indexLocationConfig = i;
			}
		}
		else if (ptr[i].location.size() > 0 && ptr[i].location[ptr[i].location.size() - 1] == '/') {
			std::string match = ptr[i].location.substr(0, ptr[i].location.size() - 1);
			std::cout << "voici match = " << match << std::endl;
			std::cout << "voici str = " << str << std::endl;
			if (match == str) {
				std::cout << "ça a bien match" << std::endl;
				if (ptr[i].location.size() > pos) {
					std::cout << "la nouvelle location est plus grande" << std::endl;
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
	return;
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
	return (oldestFile);
}

void HeaderResponse::ParseBody() {
	std::string	upload_path;

	if (this->config->locationConfig) {
		if (this->request.getPairs()["Content-Length:"] == "0") {
			this->buffer = "";
			return ;
		}
		upload_path = this->config->locationConfig[indexLocationConfig].upload_store;
		if (upload_path.empty()) {
			ft_warning("Upload path empty");
			this->request.SetError(FORBIDDEN);
			this->error = true;
			pathfile = "ERROR";
			return ;
		}
		//extraire boundary depuis Content-type
		std::string contentType = this->request.getPairs()["Content-Type:"];
		std::string	boundary = contentType.substr(contentType.find("boundary=") + 10, contentType.find("\r\n") - contentType.find("boundary=") + 10);
		CreateImage(this->request.GetBody(), boundary, upload_path);
	}
}

void HeaderResponse::CreateImage(const std::string& bufferBody, std::string& boundary, std::string& uploadPath) {
	if (boundary.length() == 0) {
		this->error = true;
		this->request.SetError(BAD_REQUEST);
		this->pathfile = "ERROR";
		return ;
	}
	if (bufferBody.length() > 4) {
		std::string name;
		std::string filename;
		std::string image_png;
		size_t startpos;
		size_t endpos;

		startpos = bufferBody.find("name=") + 6;
		endpos = bufferBody.find(";", startpos);
		name = bufferBody.substr(startpos, endpos - startpos - 1);
		startpos = bufferBody.find("filename=") + 10;
		endpos = bufferBody.find("\r\n", startpos);
		filename = bufferBody.substr(startpos, endpos - startpos - 1);
		startpos = bufferBody.find("Content-Type:") + 14;
		endpos = bufferBody.find("\r\n", startpos);
		image_png = bufferBody.substr(startpos, endpos - startpos);

		startpos = endpos + 4;
		std::string delimiter = "\r\n" + boundary + "--";
		endpos = bufferBody.find(delimiter, startpos);
		std::string image = bufferBody.substr(startpos, endpos - startpos);

		std::string img_filename = uploadPath + FindFileName();
		img_filename = "." + img_filename;
		ft_logs(img_filename);
		int image_fd = open(img_filename.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IROTH | S_IRUSR | S_IRGRP);
		if (image_fd == -1) {
			this->error = true;
			this->request.SetError(INTERNAL);
			ft_logs("Error while trying to create the new image");
			return ;
		}
		write(image_fd, image.data(), image.size());
		close(image_fd);
		pathfile = img_filename;
	};
}

void HeaderResponse::CheckMethod(void) {
	std::vector<std::string>	methods;
	std::string					headerMethod;
	bool						methodAccepted;

	methodAccepted = false;
	headerMethod = this->request.getPairs()["Method:"];
	std::cout << headerMethod << std::endl;
	//check si la méthode fait parti des méthodes prises en charge par webserv
	if (headerMethod != "GET" && headerMethod != "POST" && headerMethod != "DELETE") {
		ft_warning("Method not implemented");
		this->request.SetError(NOT_IMPLEMENTED);
		error = true;
		return ;
	}
	//check si le header est pris en charge par la location
	if (this->config->locationConfig) {
		methods = this->config->locationConfig[indexLocationConfig].methods;
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
	return;
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

void HeaderResponse::FindPathFile(void) {
	std::string uri;
	std::string root;
	if (error)
		return ;
	std::string str = this->request.getPairs()["Request-Target:"];
	if (this->config->locationConfig) {
		uri = this->config->locationConfig[indexLocationConfig].location;
		root = this->config->locationConfig[indexLocationConfig].root;
	}
	this->pathfile = root + str;
	std::cout << "voici le pathfile found = " << this->pathfile << std::endl;
	return ;
}

std::string HeaderResponse::CheckErrors(void) {
	switch (this->request.GetError())
	{
		case OK:
			return ("");
		case MOVED_PERMANENTLY :
			return (code_301());
		case BAD_REQUEST :
			return (code_400());
		case FORBIDDEN :
			return (code_403());
		case NOT_FOUND :
			return (code_404());
		case NOT_ALLOWED :
			return (code_405());
		case LENGTH :
			return (code_411());
		case BODY_TOO_LARGE:
			return (code_413());
		case INTERNAL :
			return (code_500());
		case NOT_IMPLEMENTED :
			return (code_501());
		case VERSION :
			return (code_505());
	}
	return ("");
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
	return (methods);
}

std::string HeaderResponse::GetMaxBodySize(void) {
	std::ostringstream oss;

	oss << this->config->maxBodySize;
	return (oss.str());
}

bool HeaderResponse::IsParsed(void) {
	return (this->parsed);
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
	return (std::string(ptr));
}

std::string HeaderResponse::GetNewLocation(void) {
	return (this->config->locationConfig[indexLocationConfig]._return.second);
}

//Setters

void HeaderResponse::SetBuffer(std::string str) {
	this->buffer = str;
	return ;
}

void HeaderResponse::SetFileSize(void) {
	struct stat			s;
	std::ostringstream	oss;
	std::string			size;

	if (pathfile[0] != '.')
		pathfile = "." + pathfile;
	std::cout << "Pahtfile = " << pathfile << std::endl;
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
	return ;
}

std::string HeaderResponse::PerformListing(std::string& path) {
	std::string			pathfile;
	std::string			buffer;
	std::string			href;
	std::stringstream	size;
	DIR*				directory;
	std::stringstream	ss;
	struct stat			s_stat;
	struct dirent*		s_dir;

	pathfile = "." + path;
	directory = opendir(pathfile.c_str());
	if (directory == NULL) {
		ft_warning("Directory null");
		this->error = true;
		this->request.SetError(NOT_FOUND);
		return ("");
	}
	while ((s_dir = readdir(directory)) != NULL) {
		pathfile = "." + path + s_dir->d_name;
		if (stat(pathfile.c_str(), &s_stat) != -1) {
			if (s_stat.st_mode & S_IFDIR) {
				pathfile = s_dir->d_name + std::string("/");
				size << "-";
				href = s_dir->d_name + std::string("/");
			}
			else if (s_stat.st_mode & S_IFREG) {
				size << s_stat.st_size;
				href = s_dir->d_name;
			}
			ss << "<tr><td style=\"text-align: left;\"><a href= "<< href << ">" << s_dir->d_name << "</a></td><td style=\"text-align: left;\">" << ctime(&s_stat.st_mtime) << "</td><td style=\"text-align: left;\">" << size.str() << "</td></tr>";
			size.str("");
		}
	}
	closedir(directory);
	buffer = ss.str();
	return (buffer);
}

void HeaderResponse::SetResponseError(Error err) {
	this->error = true;
	this->request.SetError(err);
	this->pathfile = "ERROR";
	return ;
}
