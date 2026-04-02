/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 21:15:26 by lomont            #+#    #+#             */
/*   Updated: 2026/04/02 16:27:17 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>
#include <cerrno>

void		ft_error(const std::string& str);
void		ft_crash(const std::string&, int);
void		logs(const std::string&);
void		warning();

#endif
