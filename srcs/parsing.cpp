/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 23:49:51 by lomont            #+#    #+#             */
/*   Updated: 2026/04/09 17:49:54 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "utils.hpp"

static size_t SearchLastBracket(const std::string &buffer, size_t i);
static void ParsingError(const std::string&, struct config*);
static int FindOneConfiguration(const std::string &buffer, size_t pos, struct config *conf);
static int FindErrorPages(const std::string &buffer, size_t &positionLastBracket, size_t pos, struct config *conf);
static int FindMaxBody(const std::string &buffer, struct config *conf);


void server::ParseServerDeclaration(const std::string &buffer) {
	size_t 	pos = 0;
	size_t 	positionLastBracket = pos;

	this->config = new struct config[serverConfigCount];
	for (size_t i = 0; i < this->serverConfigCount; i++)
	{
		pos = positionLastBracket;
		positionLastBracket = SearchLastBracket(buffer, positionLastBracket);
		if (FindOneConfiguration(buffer, pos, &config[i]) == -1)
			ParsingError("Error when trying to parse the ip/port for the server to listen", this->config);
		if (FindErrorPages(buffer, positionLastBracket, pos, &config[i]) == -1)
			ParsingError("Error when trying to parse error pages", this->config);
		if (FindMaxBody(buffer, &config[i]) == -1)
			ParsingError("Error when trying to parse the maximum body size that a client can send", this->config);
		if (FindLocation(buffer, positionLastBracket, i, pos) == -1)
			ParsingError("Error when trying to parse the locations of a server", this->config);
		positionLastBracket += 1;
	}
}

static size_t SearchLastBracket(const std::string &buffer, size_t i) {
	size_t	openedBracket = 0;

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
	return i;
}

static int FindOneConfiguration(const std::string &buffer, size_t pos, struct config *conf) {
	pos = buffer.find("listen", pos);
	size_t xpos = buffer.find(":", pos);
	if (pos == std::string::npos || xpos == std::string::npos)
		return -1;
	pos += 7;
	conf->interfacePort.first = buffer.substr(pos, xpos - pos);
	pos = buffer.find(";", xpos);
	if (pos == std::string::npos || pos > buffer.find("\n", xpos))
		return -1;
	conf->interfacePort.second = strtol(buffer.substr(xpos + 1, pos - xpos - 1).c_str(), NULL, 10);
	if (conf->interfacePort.second == 0L || errno == ERANGE)
		return -1;
	else if (conf->interfacePort.second < 1024 || conf->interfacePort.second > 65535 || conf->interfacePort.second == 3306 || conf->interfacePort.second == 5432)
		return -1;
	return 0;
}

static int FindErrorPages(const std::string &buffer, size_t &positionLastBracket, size_t pos, struct config *conf) {
	std::string 								path;
	std::vector<int> 							errorCodes;
	std::pair<std::vector<int>, std::string>	p;

	while (pos < positionLastBracket || !buffer[pos]) {
		pos = buffer.find("error_page", pos);
		if (pos == std::string::npos)
			return 0;
		pos += 11;
		bool brake = false;
		while (!brake) {
			size_t xpos = pos;
			while (!isspace(buffer[pos])) {
				if (!isdigit(buffer[pos])) {
					brake = true;
					break;
				}
				pos++;
			}
			if (!brake) {
				errorCodes.push_back(atoi(buffer.substr(xpos, pos - xpos).c_str()));
				pos++;
			}
			else {
				xpos = buffer.find(";", pos);
				if (xpos == std::string::npos || xpos > buffer.find("\n", pos))
					return -1;
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
	return 0;
}

static int FindMaxBody(const std::string &buffer, struct config *conf) {
	size_t pos = buffer.find("client", 0);
	size_t xpos = buffer.find(";", pos);
	if (pos == std::string::npos || xpos == std::string::npos || xpos > buffer.find("\n", pos))
		return -1;
	pos += 21;
	std::string sizeMax = buffer.substr(pos, xpos - pos);
	char c = sizeMax[sizeMax.length() - 1];
	std::stringstream iss(sizeMax);
	iss >> conf->maxBodySize;
	if (conf->maxBodySize == 0 && errno == EINVAL)
		return -1;
	long sizeMemory = 0;
	switch (c) {
		case 'B':
			sizeMemory = 1;
			break;
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
			if (isdigit(c))
				sizeMemory = 1;
			else {
				ft_error("Size memory error, please choose a memory size between 'B', 'K', 'M, 'G''");
				return -1;
			}
			break;
	}
	conf->maxBodySize *= sizeMemory;
	return 0;
}

static void ParsingError(const std::string& str, struct config* conf) {
	ft_error(str);
	ft_free_config(conf);
	ft_crash("Error when trying to parse the configuration file");
}

// static void printConfig(struct LocationConfig *conf) {
// 	std::cout << "location: \"" << conf->location << "\"" << std::endl;
// 	std::cout << "methods: " << std::endl;
// 	for (std::vector<std::string>::iterator it = conf->methods.begin(); it != conf->methods.end(); it++)
// 		std::cout << *it << " ";
// 	std::cout << std::endl;
// 	std::cout << "root: " << conf->root << std::endl;
// 	std::cout << "upload_store: " << conf->upload_store << std::endl;
// 	std::cout << "index: ";
// 	for (std::vector<std::string>::iterator it = conf->index.begin(); it != conf->index.end(); it++)
// 		std::cout << *it << " ";
// 	std::cout << std::endl;
// 	std::cout << "autoindex: " << conf->autoindex;
// 	std::cout << std::endl;
// 	std::cout << "return: " << conf->_return.first << " " << conf->_return.second << std::endl;
// 	std::cout << "------------------------" << std::endl;
// }
