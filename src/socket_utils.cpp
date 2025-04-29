/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket_utils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: root <root@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/29 15:10:08 by root              #+#    #+#             */
/*   Updated: 2025/04/29 16:58:17 by root             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include "../include/Server.hpp"
#include "../include/utils.hpp"
#include "../include/Message.hpp"
#include "../include/Channel.hpp"
#include "../include/Commands.hpp"
#include "../include/Headers.hpp"


int    wait_for_activity(int maxfd, fd_set &readfds)
{
    // wait for activity
    int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);
    if (activity < 0)
    {
        std::cout << ERROR << "Error in select" << std::endl;
        return (-1);
    }
    return (0);
}
