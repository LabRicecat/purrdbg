#ifndef DEBUGGER_STATE_HPP
#define DEBUGGER_STATE_HPP

#include <vector>
#include <string>

#include "parser.hpp"

bool hit_breakpoint(std::vector<Section>& breakpoints, position_t idx) {
    for(auto i : breakpoints) 
        if(i.from <= idx && i.to >= idx) 
            return true;
    return false;
}

struct Debugger {
    std::vector<Section> breakpoints;
    DebugInfo debugfile;
    stack_nyachine::StackNyachine machine = stack_nyachine::StackNyachine(0,0);
    stack_nyachine::chuwunk* ptr = nullptr;
    enum State {
        STOPPED,
        RUNNING,
        EXITED,
        FAILED,
        IDLE
    } state = State::IDLE;

    void add_breakpoint(position_t line) {
        for(const auto& i : debugfile.data)
            if(i.line == line) breakpoints.push_back(i);
    }

    constexpr position_t index() const {
        if(&machine.memowory[0]-1 == ptr) return 0;
        return ptr - &machine.memowory[0];
    }

    position_t line() {
        for(auto i : debugfile.data) 
            if(i.from <= index() && i.to >= index()) 
                return i.line;
        return 0;
    }

    stack_nyachine::NyaSignal step() {
        stack_nyachine::NyaSignal sig = stack_nyachine::NYASIG_OK;
        auto ln = line();
        while(sig == stack_nyachine::NYASIG_OK && ln == line()) {
            sig = stack_nyachine::advance(ptr,&machine);
        }
        return sig;
    }

    void continue_prog() {
        if(state != STOPPED && state != RUNNING) return;
        stack_nyachine::NyaSignal signal = stack_nyachine::NYASIG_OK;
        while(signal == stack_nyachine::NYASIG_OK && !hit_breakpoint(breakpoints,index())) {
            signal = step();
        }
        if(signal == stack_nyachine::NYASIG_EXIT) state = EXITED;
        else if(signal == stack_nyachine::NYASIG_OK) state = STOPPED;
        else state = FAILED;
    }

    void run(position_t from = 0) {
        state = RUNNING;
        machine.stack_ptr = stack_nyachine::calc_stawkbegin(machine.heawp_size);
        ptr = &machine.memowory[from]-1;
        continue_prog();
    }
};

#endif