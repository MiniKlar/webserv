/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 23:49:51 by lomont            #+#    #+#             */
/*   Updated: 2026/04/05 22:01:26 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "utils.hpp"

static size_t FindNumbersOfLocation(const std::string &, size_t, size_t);

void server::ParseServerDeclaration(const std::string &buffer) {
	size_t 	positionLastBracket;
	size_t 	pos;

	positionLastBracket = 0;
	this->config = new struct config[serverConfigCount];
	for (size_t i = 0; i < serverConfigCount; i++)
	{
		pos = positionLastBracket;
		positionLastBracket = SearchLastBracket(buffer, positionLastBracket);
		if (FindOneConfiguration(buffer, pos, &config[i]) == -1)
			ParsingError("Error when trying to parse the ip/port for the server to listen");
		if (FindErrorPages(buffer, positionLastBracket, pos, &config[i]) == -1)
			ParsingError("Error when trying to parse error pages");
		if (FindMaxBody(buffer, &config[i]) == -1)
			ParsingError("Error when trying to parse the maximum body size that a client can send");
		if (FindLocation(buffer, positionLastBracket, i, pos) == -1)
			ParsingError("Error when trying to parse the locations of a server");
		positionLastBracket += 1;
	}
}

void server::ParsingError(std::string str) {
	ft_error(str);
	ft_free_config(this->config);
	ft_crash("Error when trying to parse the configuration file");
	return ;
}

int server::FindLocation(const std::string &buffer, size_t &positionLastBracket, size_t &index, size_t pos) {
	size_t i;
	size_t positionFirstBracket;
	size_t positionLastBracketLocation;
	struct LocationConfig *ptr;

	i = 0;
	if ((pos = buffer.find("location", pos)) == std::string::npos) //TODO rajouter une root générale hard code?
		return (-1);
	config[index].numbersOfLocation = FindNumbersOfLocation(buffer, positionLastBracket, pos);
	config[index].locationConfig = new LocationConfig[config[index].numbersOfLocation];
	if (!config[index].locationConfig) {
		ft_error("Allocation failed");
		return (-1);
	}
	while (pos <= positionLastBracket) {
		ptr = &config[index].locationConfig[i];
		//Parse complete location
		positionFirstBracket = buffer.find("{", pos);
		positionLastBracketLocation = buffer.find("}", positionFirstBracket);
		if (positionFirstBracket == std::string::npos || positionLastBracketLocation == std::string::npos)
			return (-1);
		pos += 9;
		ptr->location = buffer.substr(pos, positionFirstBracket - pos - 1);
		FindMethods(buffer, pos, ptr, positionLastBracketLocation);
		FindRoot(buffer, pos, ptr, positionLastBracketLocation);
		FindIndex(buffer, pos, ptr, positionLastBracketLocation);
		FindAutoIndex(buffer, pos, ptr, positionLastBracketLocation);
		FindUpload(buffer, pos, ptr, positionLastBracketLocation);
		FindReturn(buffer, pos, ptr, positionLastBracketLocation);
		FindCGIPass(buffer, pos, ptr, positionLastBracketLocation);
		//Finding next location if it exists
		if ((pos = buffer.find("location", pos)) == std::string::npos)
			break;
		i++;
	}
	return (0);
}

size_t server::GetServerConfigCount(const std::string &buffer) {
	size_t pos;
	size_t configServerCount;

	pos = 0;
	configServerCount = 0;
	while ((pos = buffer.find("server {", pos)) != std::string::npos) {
		pos += 9;
		configServerCount++;
	}
	return (configServerCount);
}

int server::FindOneConfiguration(const std::string &buffer, size_t pos, struct config *conf) {
	size_t xpos;

	pos = buffer.find("listen", pos);
	xpos = buffer.find(":", pos);
	if (pos == std::string::npos || xpos == std::string::npos)
		return (-1);
	pos += 7;
	conf->interfacePort.first = buffer.substr(pos, xpos - pos);
	pos = buffer.find(";", xpos);
	if (pos == std::string::npos || pos > buffer.find("\n", xpos))
		return(-1);
	conf->interfacePort.second = strtol(buffer.substr(xpos + 1, pos - xpos - 1).c_str(), NULL, 10);
	if (conf->interfacePort.second == 0L || errno == ERANGE)
		return (-1);
	else if (conf->interfacePort.second < 1024 || conf->interfacePort.second > 65535 || conf->interfacePort.second == 3306 || conf->interfacePort.second == 5432)
		return (-1);
	return (0);
}

size_t server::SearchLastBracket(const std::string &buffer, size_t i) {
	size_t	openedBracket;

	openedBracket = 0;
	while (buffer[i])
	{
		if (buffer[i] == '{')
			openedBracket++;
		else if (buffer[i] == '}')
			openedBracket--;
		if (openedBracket == 0 && buffer[i] == '}')
			break;
		i++;
	}
	return (i);
}

int server::FindErrorPages(const std::string &buffer, size_t &positionLastBracket, size_t pos, struct config *conf) {
	std::pair<std::vector<int>, std::string>	p;
	std::vector<int> 							errorCodes;
	std::string 								path;
	size_t										xpos;
	bool										brake;

	brake = false;
	while (pos < positionLastBracket || !buffer[pos])
	{
		pos = buffer.find("error_page", pos);
		if (pos == std::string::npos) //TODO créer page error simple si l'utilisateur n'en met pas
			return (0);
		pos += 11;
		while (!brake)
		{
			xpos = pos;
			while (!isspace(buffer[pos]))
			{
				if (!isnumber(buffer[pos])) {
					brake = true;
					break;
				}
				pos++;
			}
			if (!brake) {
				errorCodes.push_back(atoi(buffer.substr(xpos, pos - xpos).c_str())); //TODO vérifier atoi si grand nombre
				pos++;
			}
			else {
				xpos = buffer.find(";", pos);
				if (xpos == std::string::npos)
					return (-1);
				if (buffer[pos] == ' ')
					pos++;
				path = buffer.substr(pos, xpos - pos);
				pos = xpos + 1;
			}
		}
		brake = false;
		p.first = errorCodes;
		p.second = path;
		conf->errorPage.insert(p);
		errorCodes.clear();
		path.clear();
	}
	return (0);
}

int server::FindMaxBody(const std::string &buffer, struct config *conf) {
	int			sizeMemory;
	char		c;
	size_t 		pos;
	size_t 		xpos;
	std::string sizeMax;

	sizeMemory = 0;
	pos = buffer.find("client", 0);
	xpos = buffer.find(";", pos);
	if (pos == std::string::npos || xpos == std::string::npos)
		return (-1);
	pos += 21;
	sizeMax = buffer.substr(pos, xpos - pos);
	c = sizeMax[sizeMax.length() - 1];
	conf->maxBodySize = strtol(sizeMax.c_str(), NULL, 10);
	if (conf->maxBodySize == 0 && errno == EINVAL)
		return (-1);
	switch (c) {
		case 'K':
			sizeMemory = 1024;
			break;
		case 'M':
			sizeMemory = 1024 * 1024;
			break;
		case 'G':
			sizeMemory = 1024 * 1024 * 1024;
			break;
		default:
			ft_error("Size memory error, please choose a memory size between 'K', 'M, 'G''");
			return (-1);
	}
	conf->maxBodySize *= sizeMemory;
	return (0);
}

static size_t FindNumbersOfLocation(const std::string &buffer, size_t positionLastBracket, size_t pos) {
	size_t	i;

	i = 0;
	while ((pos = buffer.find("location", pos)) <= positionLastBracket) {
		pos += 9;
		i++;
	};
	return (i);
}

size_t server::FindMethods(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp;
	size_t xpos;
	size_t ypos;

	tmp = pos;
	if ((pos = buffer.find("methods", pos)) > border)
		return (tmp);
	pos += 8;
	xpos = buffer.find(";", pos);
	if (xpos == std::string::npos)
		return (tmp);
	while (pos <= xpos)
	{
		ypos = buffer.find(" ", pos);
		if (ypos > border || ypos > xpos)
			ypos = xpos;
		if (pos < border || xpos < border)
			conf->methods.push_back(buffer.substr(pos, ypos - pos));
		pos = ypos + 1;
	}
	return (pos);
}

size_t server::FindRoot(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp;
	size_t xpos;

	tmp = pos;
	if ((pos = buffer.find("root", pos)) > border)
		return (tmp);
	pos += 5;
	xpos = buffer.find(";", pos);
	if (xpos == std::string::npos)
		return (tmp);
	conf->root = buffer.substr(pos, xpos - pos);
	return (pos);
}

size_t server::FindIndex(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp;
	size_t xpos;
	size_t ypos;

	tmp = pos;
	if ((pos = buffer.find("index ", pos)) > border)
		return (tmp);
	pos += 6;
	xpos = buffer.find(";", pos);
	if (xpos == std::string::npos)
		return (tmp);
	while (pos <= xpos)
	{
		ypos = buffer.find(" ", pos);
		if (ypos > xpos)
			ypos = xpos;
		if (pos < border || xpos < border)
			conf->index.push_back(buffer.substr(pos, ypos - pos));
		pos = ypos + 1;
	}
	return (pos);
}

size_t server::FindAutoIndex(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp;
	size_t found;

	tmp = pos;
	if ((pos = buffer.find("autoindex", pos)) > border)
		return tmp;
	pos = pos + 10;
	found = buffer.find("on", pos);
	if (found < border && found != std::string::npos)
		conf->autoindex = true;
	else
		conf->autoindex = false;
	return (pos);
}

size_t server::FindReturn(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp;
	size_t xpos;

	tmp = pos;
	if ((pos = buffer.find("return", pos)) > border)
		return tmp;
	pos += 7;
	xpos = buffer.find(" ", pos);
	if (xpos == std::string::npos)
		return (tmp);
	conf->_return.first = buffer.substr(pos, xpos - pos);
	pos = xpos + 1;
	xpos = buffer.find(";", pos);
	if (xpos == std::string::npos) {
		conf->_return.first = "";
		return (tmp);
	}
	conf->_return.second = buffer.substr(pos, xpos - pos);
	return (pos);
}

size_t server::FindUpload(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp;
	size_t xpos;

	tmp = pos;
	if ((pos = buffer.find("upload_store", pos)) > border)
		return (tmp);
	pos += 13;
	xpos = buffer.find(";", pos);
	if (xpos == std::string::npos)
		return (tmp);
	conf->upload_store = buffer.substr(pos, xpos - pos);
	return (pos);
}

size_t server::FindCGIPass(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp;
	size_t xpos;

	tmp = pos;
	if ((pos = buffer.find("cgi_pass", pos)) > border)
		return (tmp);
	if ((pos = buffer.find(".php", pos)) > border)
		return (tmp);
	pos += 5;
	xpos = buffer.find(";", pos);
	if (xpos == std::string::npos)
		return (tmp);
	conf->pathPHPexecutable = buffer.substr(pos, xpos - pos);
	return (pos);
}

void server::printConfig(struct LocationConfig *conf) {
	std::cout << "location: \"" << conf->location << "\"" << std::endl;
	std::cout << "methods: " << std::endl;
	for (std::vector<std::string>::iterator it = conf->methods.begin(); it != conf->methods.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;
	std::cout << "root: " << conf->root << std::endl;
	std::cout << "upload_store: " << conf->upload_store << std::endl;
	std::cout << "index: ";
	for (std::vector<std::string>::iterator it = conf->index.begin(); it != conf->index.end(); it++)
		std::cout << *it << " ";
	std::cout << std::endl;
	std::cout << "autoindex: " << conf->autoindex;
	std::cout << std::endl;
	std::cout << "return: " << conf->_return.first << " " << conf->_return.second << std::endl;
	std::cout << "path PHP executable: " << conf->pathPHPexecutable << std::endl;
	std::cout << "------------------------" << std::endl;
}
