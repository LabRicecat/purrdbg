#ifndef DEBUGGER_UI_HPP
#define DEBUGGER_UI_HPP

#include <ncurses.h>
#include <fstream>
#include "catpkgs/kittenlexer/kittenlexer.hpp"
#include "parser.hpp"
#include "state.hpp"

struct Resolution {
    int y, x;
};

struct DisplayPosition {
    unsigned long line, max;
    long offset;
};

struct Window {
    WINDOW* win;
    Resolution resolution;
};

struct BufferedWindow {
    Window window;
    std::vector<std::string> buffer;

    std::string& next() {
        buffer.erase(buffer.begin());
        buffer.push_back("");
        return buffer.back();
    }
};

struct Screen {
    Window display;
    BufferedWindow shell;
};

std::string read_file(const std::string& s) {
    std::ifstream ifile(s);
    std::string v;
    while(ifile.good()) v += ifile.get();
    if(v != "") v.pop_back();
    return v;
}

std::vector<std::string> wrap(const Resolution& res, std::string source) {
    std::vector<std::string> v;
    if(source.empty()) return {""};
    for(size_t i = 0; i < source.size(); ++i) {
        if(i% (res.x-10) == 0) v.push_back("");
        v.back() += source[i];
    }
    return v;
}

void refill_window(Window& window, DisplayPosition position, lexed_kittens lines, Debugger& debugger) {
    wclear(window.win);
    box(window.win,0,0);
    wmove(window.win,1,1);

    for(size_t i = 0; i < window.resolution.y-2; ++i)
        if(lines.size() > position.line + i - position.offset) {
            auto token = lines[position.line + i - position.offset];
            auto wraped = wrap(window.resolution,token.src);
            int offset = 0;
            for(size_t j = 0; j < wraped.size(); ++j) {
                mvwprintw(window.win,i+1 + (offset++),1,"%4ld %c %s",token.line, debugger.line() == token.line ? '>' : '|',wraped[j].c_str());
            }
        }

    wrefresh(window.win);
}

void app_content(BufferedWindow& buffered, std::string source) {
    if(!source.empty() && buffered.buffer.empty() && source.front() == '\n') source.erase(source.begin());
    wclear(buffered.window.win);
    box(buffered.window.win,0,0);
    if(buffered.buffer.empty())
        buffered.buffer.push_back("");
    long wline = buffered.buffer.size();
    for(auto i : source) {
        if(i == '\n') ++wline;
        if(wline > buffered.window.resolution.y-2) {
            buffered.next();
            --wline;
        }
        if(wline > buffered.buffer.size()) {
            buffered.buffer.push_back("");
        }
        if(i != '\n')
            buffered.buffer.back() += i;
        if(buffered.buffer.back().size() > buffered.window.resolution.x-1) {
            ++wline;
            std::string bf = buffered.buffer.back();
            buffered.next();
            while(bf.size() > buffered.window.resolution.x-1) {
                buffered.buffer.back().push_back(bf.back());
                bf.pop_back();
            }
        }
    }
    wline = 0;
    for(auto i : buffered.buffer) {
        mvwprintw(buffered.window.win,wline+1,1,"%s",i.c_str());
        ++wline;
    }
    wrefresh(buffered.window.win);
}

std::string prompt_shell(Screen& screen, std::string text) {
    app_content(screen.shell,text);
    std::string ret;
    int input = 0;
    const int xmax = screen.shell.window.resolution.x-1;
    int xmin = screen.shell.buffer.back().size();
    int xpos = screen.shell.buffer.back().size();
    int ypos = screen.shell.buffer.size();
    while((input = getch()) != '\n') {
        if(isascii(input)) {
            ++xpos;
            if(xpos > xmax) {
                xpos = 1;
                xmin = 1;
                ++ypos;
                if(screen.shell.buffer.size() > screen.shell.window.resolution.y-1) 
                    screen.shell.next();
                else
                    screen.shell.buffer.push_back("");
            }
            mvwprintw(screen.shell.window.win,ypos,xpos,"%c",(char)input);
            wrefresh(screen.shell.window.win);
            screen.shell.buffer.back().insert(screen.shell.buffer.back().begin()+xpos-1,(char)input);
            ret.insert(ret.begin()+xpos-xmin-1,(char)input);
        }
        else if(input == KEY_LEFT) {
            if(xpos <= xmin) continue;
            --xpos;
            wmove(screen.shell.window.win,ypos,xpos+1);
            wrefresh(screen.shell.window.win);
        }
        else if(input == KEY_RIGHT) {
            if(xpos >= xmax) continue;
            ++xpos;
            wmove(screen.shell.window.win,ypos,xpos+1);
            wrefresh(screen.shell.window.win);
        }
        else if(input == KEY_BACKSPACE) {
            if(xpos <= xmin) continue;
            screen.shell.buffer.back().erase(screen.shell.buffer.back().begin() + xpos - 1);
            ret.erase(ret.begin()+xpos-xmin-1);
            mvwprintw(screen.shell.window.win,ypos,xpos," ");
            --xpos;
            wmove(screen.shell.window.win,ypos,xpos+1);
            wrefresh(screen.shell.window.win);
        }
    }
    return ret;
}

void cleanup(Screen screen) {
    delwin(screen.display.win);
    delwin(screen.shell.window.win);
    endwin();
}

#endif