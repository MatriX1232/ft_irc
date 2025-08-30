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
        std::cout << ERROR << "Select error: no activity detected" << std::endl;
        return (-1);
    }
    return (0);
}
