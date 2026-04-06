/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/04 23:45:26 by lomont            #+#    #+#             */
/*   Updated: 2026/04/06 22:28:26 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>

struct LocationConfig {
	bool										autoindex;
	std::string									location;
	std::string									root;
	std::string									upload_store;
	std::map<std::string, std::string>			cgi_handlers;
	std::vector<std::string>					methods;
	std::vector<std::string>					index;
	std::pair<std::string, std::string> 		_return;
};

struct	config {
	size_t										numbersOfLocation;
	unsigned long 								maxBodySize;
	struct LocationConfig*						locationConfig;
	std::pair<std::string, long int>			interfacePort;
	std::map<std::vector<int>, std::string >	errorPage;
};

#endif
