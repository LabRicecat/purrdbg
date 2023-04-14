#ifndef DEBUGGER_PARSER_HPP
#define DEBUGGER_PARSER_HPP

#include "catpkgs/kittenlexer/kittenlexer.hpp"
#include "catpkgs/nyasm/nyasm.hpp"

struct Section {
    position_t line, from, to;
};

struct DebugInfo {
    std::vector<Section> data;
    size_t ptr = 0;

    bool load(std::string source) {
        data.clear();
        KittenLexer lexer = KittenLexer()
            .add_linebreak('\n')
            .add_ignore(' ')
            .add_ignore('\t')
            .add_extract('-')
            .add_extract(':')
            .ignore_backslash_opts()
            .erase_empty();
        auto lexed = lexer.lex(source);
        std::vector<std::vector<std::string>> lines;
        long line = 0;
        for(auto i : lexed) {
            if(line != i.line) {
                line = i.line;
                lines.push_back({});
            }
            lines.back().push_back(i.src);
        }

        for(auto i : lines) {
            if(i.size() != 5) return false;
            if(i[1] != ":") return false;
            if(i[3] != "-") return false;
            try {
                data.push_back(Section{std::stoull(i[0]),std::stoull(i[2]),std::stoull(i[4])});
            }
            catch(...) { return false; }
        }

        return true;
    }

    Section next() {
        return data[ptr++];
    }

    bool has_next() const { return ptr == data.size(); }
};

#endif