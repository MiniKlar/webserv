/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 23:06:49 by lomont            #+#    #+#             */
/*   Updated: 2026/04/05 13:26:26 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int main(int argc, char** argv) {
	if (argc > 3)
		ft_error("To start correctly a web server, please type \"./webserv your_configuration_file\"");
	else if (argc == 2)
		server webserv(argv[argc - 1]);
	else
		server webserv(DEFAULT_CONFIGURATION_FILE);
	return (0);
}
