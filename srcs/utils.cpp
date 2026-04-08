/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 21:15:00 by lomont            #+#    #+#             */
/*   Updated: 2026/04/08 01:21:23 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "utils.hpp"

void	ft_crash(const std::string& str) {
	std::cerr << "[CRASH]" << str << std::endl << strerror(errno) << std::endl;
	exit(1);
}

void	ft_error(const std::string& str) {
	std::cerr << "[ERROR]" << str << std::endl;
}

void ft_logs(const std::string& str) {
	std::cout << "[LOGS] [" + str + "]" << std::endl;
}

void ft_warning(const std::string& str) {
	std::cout << "[WARNING]" << str << std::endl << strerror(errno) << std::endl;
}

void ft_free_config(struct config* conf) {
	if (conf->locationConfig)
		delete[] conf->locationConfig;
	delete[] conf;
}
