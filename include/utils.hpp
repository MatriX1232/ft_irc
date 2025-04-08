/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 11:56:28 by root              #+#    #+#             */
/*   Updated: 2025/03/29 00:08:06 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <string>
#include <stdio.h>

void ft_log(const std::string &str, const std::string &str_optional, int level);
void ft_log_sub(const std::string &str, const std::string &str_optional, int level, int sub_level);

#endif