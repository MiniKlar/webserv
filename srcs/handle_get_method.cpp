/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GET_answer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 22:08:14 by lomont            #+#    #+#             */
/*   Updated: 2026/04/09 22:09:35 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "headerResponse.hpp"
#include "webserv.hpp"

void HeaderResponse::HandleGet(void) {
	if (this->request.getPairs()["Request-Target:"] == "/get-cookie") {
		bodySize = 0;
		bodySizePrint = "0";
		cookie = true;
		return ;
	}
	FindPathFile(); //trouver le chemin complet du fichier
	if (this->config->locationConfig) {
		std::vector<std::string> index = this->config->locationConfig[indexLocationConfig].index;
		if (this->pathfile[this->pathfile.length() - 1] == '/') {
			bool found = false;
			if (!index.empty()) {
				struct stat s;
				for (std::vector<std::string>::iterator it = index.begin(); it != index.end(); it++) {
					std::string	 path = std::string(".") + this->pathfile + *it;
					if (access(path.c_str(), F_OK) == 0) {
						if (stat(path.c_str(), &s) != -1 && S_ISREG(s.st_mode)) {
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
					std::stringstream   ss;
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
}

void HeaderResponse::HandleAutoIndex(void) {
	std::string pathfile = this->pathfile;
	std::string header = "<html>\n<head><title>Index of " + pathfile + "</title></head>\n<body>\n<h1>Index of " + pathfile + "</h1><hr>\n<table width=\"100%\">\n<tr style=\"text-align: left;\"><th>Name</th><th>Last Modified</th><th>Size</th></tr>\n";
	std::string buffer = PerformListing(pathfile);
	if (buffer.empty()) {
		this->error = true;
		this->request.SetError(INTERNAL);
		this->pathfile = "ERROR";
		return ;
	}
	this->buffer = header + buffer + "</table>\n<hr>\n</body>\n</html>";
}

std::string HeaderResponse::PerformListing(std::string& path) {
	std::string	pathfile = "." + path;
	DIR* directory = opendir(pathfile.c_str());
	if (directory == NULL) {
		ft_warning("Directory null");
		this->error = true;
		this->request.SetError(NOT_FOUND);
		return ("");
	}
	std::stringstream	ss;
	struct dirent*		s_dir;
	while ((s_dir = readdir(directory)) != NULL) {
		pathfile = "." + path + s_dir->d_name;
		struct stat s_stat;
		if (stat(pathfile.c_str(), &s_stat) != -1) {
			std::string href;
			std::stringstream	size;
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
	std::string buffer = ss.str();
	return buffer;
}