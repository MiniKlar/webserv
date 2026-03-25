/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 23:06:49 by lomont            #+#    #+#             */
/*   Updated: 2026/03/07 23:39:21 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int main(int argc, char** argv) {
	if (argc > 3)
		ft_error("To start correctly a webserver, please write \"./webserv your_configuration_file\"");
	else if (argc == 2)
		server webserv(argv[argc - 1]);
	else
		server webserv(DEFAULT_CONFIGURATION_FILE);
	return (0);
}
