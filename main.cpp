/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 23:06:49 by lomont            #+#    #+#             */
/*   Updated: 2026/02/19 04:08:40 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

void ft_error(const std::string& str, int exit_code) {
	std::cerr << str << std::endl;
	std::cerr << strerror(errno) << std::endl;
	exit(exit_code);
}

int main(void) {
	char buffer[] = {"GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/8.7.1\r\nAccept: */*\r\n{}"};
	HeaderRequest test (buffer, 79);
	test.printDebug();
	return (0);
}

