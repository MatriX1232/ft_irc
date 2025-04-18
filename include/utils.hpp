/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 11:56:28 by root              #+#    #+#             */
/*   Updated: 2025/04/09 19:29:18 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <vector>

void ft_log(const std::string &str, const std::string &str_optional, int level);
void ft_log_sub(const std::string &str, const std::string &str_optional, int level, int sub_level);
std::vector<std::string> split(const std::string &str, char delimiter);

#endif