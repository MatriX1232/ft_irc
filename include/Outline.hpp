#ifndef OUTLINE_HPP
# define OUTLINE_HPP

#include <iostream>
#include <string>
#include "Headers.hpp"

class Outline
{
    private:
        std::string _content;
        std::string _outline_color;
        std::string _content_color;
        std::string _outline_text;
    
    public:
        Outline(std::string content, std::string outline_color, std::string content_color, std::string outline_text);

        std::string getContent();
        std::string getOutlineC();
        std::string getContentC();

        friend std::ostream &operator<<(std::ostream &os, const Outline &outline);

};

#endif