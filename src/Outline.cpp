#include "../include/Outline.hpp"

Outline::Outline(std::string content, std::string outline_color, std::string content_color) : _content(content), _outline_color(outline_color), _content_color(content_color)
{

}

std::string Outline::getContent()
{
    return (_content);
}

std::string Outline::getContentC()
{
    return (_content_color);
}

std::string Outline::getOutlineC()
{
    return (_outline_color);
}

std::ostream &operator<<(std::ostream &os, const Outline &outline)
{
    os << outline._outline_color;
    os << "╭─";
    for (int i = 0; i <= (int)outline._content.length(); i++)
        os << "─";
    os << "╮\n";
    
    os << "│ " << outline._content_color << outline._content_color << outline._content << END << outline._outline_color << " │\n";

    os << "╰─";
    for (int i = 0; i < (int)outline._content.length(); i++)
        os << "─";
    os << "─╯\n" << END;
    
    return (os);
}