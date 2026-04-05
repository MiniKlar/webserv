/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/15 23:09:55 by lomont            #+#    #+#             */
/*   Updated: 2026/04/05 14:48:04 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#define DEFAULT_CONFIGURATION_FILE "webserv.conf"

#include <fcntl.h>
#include <unistd.h>
#include "utils.hpp"
#include "server.hpp"
#include "serverConfig.hpp"

class Client;

void				handler(int);

extern int errno;
extern volatile sig_atomic_t g_stop;

#endif

