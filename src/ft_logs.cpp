/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_logs.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 12:26:51 by root              #+#    #+#             */
/*   Updated: 2025/04/09 14:39:32 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>

const std::string GREEN = "\033[32m";
const std::string CYAN = "\033[36m";
const std::string YELLOW = "\033[33m";
const std::string RED = "\033[31m";
const std::string END = "\033[0m";
// const char spin[9] = {'⠁', '⠃', '⠇', '⠧', '⠷', '⠿', '⠷', '⠧'};

void ft_log(const std::string &str, const std::string &str_optional = "", int level = 0)
{
    if (level == 0)
        std::cout << "[ " << CYAN << "INFO" << END << " ] " << str;
    else if (level == 1)
        std::cout << "[ " << GREEN << "SUCCESS" << END << " ]    " << str;
    else if (level == 2)
        std::cout << "[ " << YELLOW << "WARNING" << END << " ] " << str;
    else if (level == 3)
        std::cerr << "[ " << RED << "ERROR" << END << " ]   " << str;

    if (!str_optional.empty())
        std::cout << " | " << str_optional << std::endl;
    else
        std::cout << std::endl;
}

void ft_log_sub(const std::string &str, const std::string &str_optional = "", int level = 0, int sub_level = 0)
{
    if (sub_level == 1)
        std::cout << "  |--->  ";
    else if (sub_level == 2)
        std::cout << "    |--->  ";

    if (level == 0)
        std::cout << "[ " << CYAN << "INFO" << END << " ] " << str;
    else if (level == 1)
        std::cout << "[ " << GREEN << "SUCCESS" << END << " ]    " << str;
    else if (level == 2)
        std::cout << "[ " << YELLOW << "WARNING" << END << " ] " << str;
    else if (level == 3)
        std::cerr << "[ " << RED << "ERROR" << END << " ]   " << str;

    if (!str_optional.empty())
        std::cout << " | " << str_optional << std::endl;
    else
        std::cout << std::endl;
}