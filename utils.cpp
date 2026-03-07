/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 21:15:00 by lomont            #+#    #+#             */
/*   Updated: 2026/03/03 23:47:56 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

void	ft_error(const std::string& str, int exit_code) {
	std::cerr << "[CRASH]" << str << strerror(errno) << std::endl;;
	exit(exit_code);
}

void logs(const std::string& str) {
	std::cout << "[LOGS] " + str << std::endl;
}

void warning(void) {
	std::cout << "[WARNING] " << strerror(errno) << std::endl;
}
