/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 21:15:26 by lomont            #+#    #+#             */
/*   Updated: 2026/03/07 22:26:41 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>

void		ft_error(const std::string& str);
void		ft_crash(const std::string&, int);
void		logs(const std::string&);
void		warning();

#endif
