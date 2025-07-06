#include "../include/Outline.hpp"

Outline::Outline(std::string content, std::string outline_color, std::string content_color, std::string outline_text) : _content(content), _outline_color(outline_color), _content_color(content_color), _outline_text(outline_text)
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
    if (outline._outline_text == "")
    {
        // std::cout << INFO << "Choosen outline text" << std::endl;
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
    }
    else
    {
        // std::cout << INFO << "Choosen lack of outline text" << std::endl;
        os << outline._outline_color;
        os << "╭─ " << outline._outline_text << " ─";
        int delta = (int)outline._content.length() - 3 - (int)outline._outline_text.length();
        if (delta > 0)
        {
            for (int i = 0; i <= delta; i++)
                os << "─";
        }
        
        os << "╮\n";
        
        if (outline._outline_text.length() > outline._content.length())
        {
            os << "│ " << outline._content_color << outline._content_color << outline._content << END;
            for (int i = 0; i <= (int)outline._outline_text.length() - (int)outline._content.length() + 1; i++)
                os << " ";
           os << outline._outline_color << " │\n";
        }
        else
        {
            os << "│ " << outline._content_color << outline._content_color << outline._content << END << outline._outline_color << " │\n";
        }

        os << "╰─";

        if (outline._outline_text.length() > outline._content.length())
        {
            for (int i = 0; i <= (int)outline._outline_text.length() + 1; i++)
                os << "─";
        }
        else
        {
            for (int i = 0; i < (int)outline._content.length(); i++)
                os << "─";
        }
        
        os << "─╯\n" << END;
    }
    
    return (os);
}