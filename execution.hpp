#ifndef DEBUGGER_EXECUTION_HPP
#define DEBUGGER_EXECUTION_HPP

#include "ui.hpp"
#include "state.hpp"

template<typename T, typename U, typename TU>
inline static constexpr T clamp(const T& lhs, const U& lower, const TU& upper) {
    if(lhs < lower) return lower;
    if(lhs > upper) return upper;
    return lhs;
}

// return true -> reload the display
bool handle_input(int input, Screen& screen, DisplayPosition& position, Debugger& debugger) {
    static int test = 0;
    switch(input) {
        case 'k':
        case KEY_UP:
            if(position.line + position.offset == 0) return false;
            --position.line;
            return true;
        case 'j':
        case KEY_DOWN:
            if(position.line + position.offset > position.max-1) return false;
            ++position.line;
            return true;
        case KEY_END:
            if(position.line + position.offset == position.max-1) return false;
            position.line = position.max-1;
            return true;
        case KEY_BEG:
            if(position.line == 0) return false;
            position.line = 0;
            return true;
        case 'q':
            cleanup(screen);
            std::exit(0);
            break;
        case 'b':
            {
                auto input = prompt_shell(screen,"\nBreakpoint: ");
                try {
                    position_t line = std::stoll(input);
                    debugger.add_breakpoint(line);
                    app_content(screen.shell,"\nAdded!");
                }
                catch(...) {
                    app_content(screen.shell,"\nInvalid position!");
                }
            }; break;
        case 'v':
            {
                auto input = prompt_shell(screen,"\nHeapView: ");
                try {
                    position_t address = std::stoll(input);
                    app_content(screen.shell,"\n" + std::to_string(address) + ": " + std::to_string(debugger.machine.heawp[address]));
                }
                catch(...) {
                    app_content(screen.shell,"\nInvalid address!");
                }
            }; break;
        case 's':
            {
                app_content(screen.shell,"\n=== STACK ===");
                for(size_t i = 0; i < debugger.machine.stack_ptr - stack_nyachine::calc_stawkbegin(debugger.machine.heawp_size); ++i) 
                    app_content(screen.shell,"\n" + std::to_string(i+1) + " = " + std::to_string(debugger.machine.heawp[debugger.machine.stack_ptr-i]));
            }; break;
        case 'r':
            {
                app_content(screen.shell,"\nStarting execution...");
                debugger.run();
                position.line = clamp(debugger.line() - clamp(screen.display.resolution.y/2,0, debugger.line()),0,position.max);
                goto EXEC_INFO;
                return true;
            }; break;
        case 'c':
            {
                if(debugger.state != debugger.STOPPED) {
                    app_content(screen.shell,"\nProgram is not running at the moment");
                    return false;
                }
                debugger.step();
                debugger.continue_prog();
                position.line = clamp(debugger.line() - clamp(screen.display.resolution.y/2,0, debugger.line()),0,position.max);
EXEC_INFO:
                if(debugger.state == Debugger::STOPPED) 
                    app_content(screen.shell,"\nRun: hit breakpoint at line " + std::to_string(debugger.line()));
                else if(debugger.state == Debugger::FAILED) {
                    app_content(screen.shell,"\nFailed!");
                    debugger.state = debugger.EXITED;
                }
                else {
                    app_content(screen.shell,"\nFinished execution!");
                }
                return true;
            }; break;
        case 'n':
            {
                if(debugger.state == debugger.EXITED) {
                    app_content(screen.shell,"\nProgram is not running at the moment");
                    return false;
                }
                debugger.step();
                position.line = clamp(debugger.line() - clamp(screen.display.resolution.y/2,0, debugger.line()),0,position.max);
                if(debugger.state == Debugger::FAILED) {
                    app_content(screen.shell,"\nFailed!");
                    debugger.state = debugger.EXITED;
                }
                else if(debugger.state == Debugger::EXITED)
                    app_content(screen.shell,"\nFinished execution!");
                return true;
            }; break;
        case 'w':
            {
                screen.shell.buffer.clear();
                wclear(screen.shell.window.win);
                box(screen.shell.window.win,0,0);
                wmove(screen.shell.window.win,1,1);
                wrefresh(screen.shell.window.win);
            }; break;
    }
    return false;
}

// executes every frame
void update(Screen& screen, const lexed_kittens& lines, DisplayPosition& position, Debugger& data) {
    int input = getch();
    if(handle_input(input,screen,position,data)) 
        refill_window(screen.display,position,lines,data);
}

void run_debugger(std::string file) {
    KittenLexer lexer = KittenLexer()
        .add_linebreak('\n')
        .ignore_backslash_opts()
    ;
    auto lines = lexer.lex(read_file(file));

    Debugger debugger;
    std::string debugfile;
    try {
        debugger.machine = compile(read_file(file),true,&debugfile);
    }
    catch(FailedExecutionException& err) {
        std::cout << "\n\nPurrdbg: Unable to start debugging, errors occured.\n";
        return;
    }

    if(!debugger.debugfile.load(debugfile)) {
        std::cout << "Purrdbg: Unable to debug: Debug file loading unexpectedly failed.\nTHIS IS AN ERROR, please report by opening an issue on github! (https://github.com/LabRicecat/purrgbd)\n";
        return;
    }

    WINDOW* main_window = initscr();
    noecho();
    nodelay(main_window,TRUE);
    keypad(stdscr, TRUE);
    int x,y;
    getmaxyx(main_window,y,x);
    
    Resolution display_resolution = {y / 3 * 2 - 1,x - 1};
    Resolution shell_resolution = {y / 3 - 1,x - 1};

    if(shell_resolution.y < 3 || shell_resolution.x < 3 || display_resolution.y < 3 || display_resolution.x < 3) {
        endwin();
        return;
    }

    WINDOW* display_window = newwin(display_resolution.y,display_resolution.x,1,1);
    WINDOW* shell_window = newwin(shell_resolution.y,shell_resolution.x,display_resolution.y+1,1);
    refresh();
    box(display_window,0,0);
    box(shell_window,0,0);
    wrefresh(display_window);
    wrefresh(shell_window);
    
    Window display = {display_window, display_resolution};
    Window shell = {shell_window, shell_resolution};

    DisplayPosition position = {0,lines.size()-3,0};
    Screen picture = {display,shell};
    
    refill_window(picture.display,position,lines,debugger);
    app_content(picture.shell,"-- Purrdgb ---");
    while(true) 
        update(picture,lines,position,debugger);
    cleanup(picture);
}


#endif