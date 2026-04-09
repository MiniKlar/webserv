/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_location.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 17:15:49 by lomont            #+#    #+#             */
/*   Updated: 2026/04/09 17:52:12 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.hpp"
#include "utils.hpp"

static size_t FindMethods(const std::string&, size_t, struct LocationConfig*, size_t&);
static size_t FindRoot(const std::string&, size_t, struct LocationConfig*, size_t&);
static size_t FindIndex(const std::string&, size_t, struct LocationConfig*, size_t&);
static size_t FindAutoIndex(const std::string&, size_t, struct LocationConfig*, size_t&);
static size_t FindUpload(const std::string&, size_t, struct LocationConfig*, size_t&);
static size_t FindReturn(const std::string&, size_t, struct LocationConfig*, size_t&);
static size_t FindCGIPass(const std::string&, size_t, struct LocationConfig*, size_t&);
static size_t FindNumbersOfLocation(const std::string&, size_t, size_t);

int server::FindLocation(const std::string &buffer, size_t &positionLastBracket, size_t &index, size_t pos) {
	if ((pos = buffer.find("location", pos)) == std::string::npos)
		return -1;
	config[index].numbersOfLocation = FindNumbersOfLocation(buffer, positionLastBracket, pos);
	if (config[index].numbersOfLocation > 0) {
		config[index].locationConfig = new LocationConfig[config[index].numbersOfLocation];
		if (!config[index].locationConfig) {
			ft_error("Allocation failed");
			return -1;
		}
	}
	else
		config[index].locationConfig = NULL;
	size_t i = 0;
	while (pos <= positionLastBracket && config[index].locationConfig != NULL) {
		struct LocationConfig *ptr = &config[index].locationConfig[i];
		//Parse complete location
		size_t positionFirstBracket = buffer.find("{", pos);
		size_t positionLastBracketLocation = buffer.find("}", positionFirstBracket);
		if (positionFirstBracket == std::string::npos || positionLastBracketLocation == std::string::npos)
			return -1;
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
	return 0;
}

static size_t FindMethods(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp = pos;
	if ((pos = buffer.find("methods", pos)) > border)
		return tmp;
	pos += 8;
	size_t xpos = buffer.find(";", pos);
	if (xpos == std::string::npos || xpos > buffer.find("\n", pos))
		return tmp;
	while (pos <= xpos)
	{
		size_t ypos = buffer.find(" ", pos);
		if (ypos > border || ypos > xpos)
			ypos = xpos;
		if (pos < border || xpos < border)
			conf->methods.push_back(buffer.substr(pos, ypos - pos));
		pos = ypos + 1;
	}
	return pos;
}

static size_t FindRoot(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp = pos;
	if ((pos = buffer.find("root", pos)) > border)
		return tmp;
	pos += 5;
	size_t xpos = buffer.find(";", pos);
	if (xpos == std::string::npos || xpos > buffer.find("\n", pos))
		return tmp;
	conf->root = buffer.substr(pos, xpos - pos);
	return pos;
}

static size_t FindIndex(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp = pos;
	if ((pos = buffer.find("index ", pos)) > border)
		return tmp;
	pos += 6;
	size_t xpos = buffer.find(";", pos);
	if (xpos == std::string::npos || xpos > buffer.find("\n", pos))
		return tmp;
	while (pos <= xpos) {
		size_t ypos = buffer.find(" ", pos);
		if (ypos > xpos)
			ypos = xpos;
		if (pos < border || xpos < border)
			conf->index.push_back(buffer.substr(pos, ypos - pos));
		pos = ypos + 1;
	}
	return pos;
}

static size_t FindAutoIndex(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp = pos;
	if ((pos = buffer.find("autoindex", pos)) > border)
		return tmp;
	pos = pos + 10;
	size_t found = buffer.find("on", pos);
	if (found < buffer.find("\n", pos) && found != std::string::npos)
		conf->autoindex = true;
	else
		conf->autoindex = false;
	return pos;
}

static size_t FindUpload(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp = pos;
	if ((pos = buffer.find("upload_store", pos)) > border)
		return tmp;
	pos += 13;
	size_t xpos = buffer.find(";", pos);
	if (xpos == std::string::npos || xpos > buffer.find("\n", pos))
		return tmp;
	conf->upload_store = buffer.substr(pos, xpos - pos);
	return pos;
}

static size_t FindReturn(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp = pos;
	if ((pos = buffer.find("return", pos)) > border)
		return tmp;
	pos += 7;
	size_t xpos = buffer.find(" ", pos);
	if (xpos == std::string::npos || xpos > buffer.find("\n", pos))
		return tmp;
	conf->_return.first = buffer.substr(pos, xpos - pos);
	pos = xpos + 1;
	xpos = buffer.find(";", pos);
	if (xpos == std::string::npos || xpos > buffer.find("\n", pos)) {
		conf->_return.first = "";
		return tmp;
	}
	conf->_return.second = buffer.substr(pos, xpos - pos);
	return pos;
}

static size_t FindCGIPass(const std::string &buffer, size_t pos, struct LocationConfig *conf, size_t &border) {
	size_t tmp = pos;
	while ((pos = buffer.find("cgi_pass", pos)) < border) {
		size_t endpos = buffer.find(";", pos);
		if (endpos == std::string::npos || endpos > border)
			return tmp;
		pos += 9;
		size_t xpos = buffer.find(" ", pos + 1);
		if (xpos > endpos)
			return tmp;
		std::pair<std::string, std::string> pair(
			buffer.substr(pos, xpos - pos),
			buffer.substr(xpos + 1, endpos - (xpos + 1))
		);
		conf->cgi_handlers.insert(pair);
	}
	return pos;
}

static size_t FindNumbersOfLocation(const std::string &buffer, size_t positionLastBracket, size_t pos) {
	size_t	i = 0;
	while ((pos = buffer.find("location", pos)) <= positionLastBracket) {
		pos += 9;
		i++;
	};
	return i;
}