/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 21:15:26 by lomont            #+#    #+#             */
/*   Updated: 2026/04/06 23:12:05 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <cstdlib>

struct config;
struct LocationConfig;

void		ft_error(const std::string&);
void		ft_crash(const std::string&);
void		ft_logs(const std::string&);
void		ft_warning(const std::string&);
void		ft_free_config(struct config*);

#endif
