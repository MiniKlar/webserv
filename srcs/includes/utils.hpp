/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lomont <lomont@student.42lehavre.fr>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/26 21:15:26 by lomont            #+#    #+#             */
/*   Updated: 2026/04/06 20:13:53 by lomont           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>

struct config;
struct LocationConfig;

void		ft_error(const std::string&);
void		ft_crash(const std::string&);
void		ft_logs(const std::string&);
void		ft_warning(void);
void		ft_free_config(struct config*);

#endif
