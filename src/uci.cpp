/*
MIT License

Copyright (c) 2017 CPirc
Copyright (c) 2018 CPirc
Copyright (c) 2019 CPirc

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

#include "uci.h"
#include "position.h"
#include "move.h"
#include "search.h"

static SearchController sc;

namespace UCI
{
    namespace Extension
    {
        void print()
        {
            print_position(sc.pos);
        }

        void perft(std::stringstream& ss)
        {
            int depth = 0;
            ss >> depth;
            if(depth < 1) {depth = 1;}

            std::uint64_t nodes = 0ULL;
            for(int i = 1; i <= depth; ++i)
            {
                auto start = std::chrono::high_resolution_clock::now();
                nodes = perft(sc.pos, i);
                auto finish = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = finish - start;

                std::cout << "info"
                          << " depth " << i
                          << " nodes " << nodes
                          << " time " << static_cast<int>(elapsed.count()*1000)
                          << " nps " << static_cast<int>(nodes/elapsed.count())
                          << std::endl;
            }

            std::cout << "nodes " << nodes << std::endl;
        }

        void ttperft(std::stringstream& ss)
        {
            int depth = 0;
            ss >> depth;
            if(depth < 1) {depth = 1;}

            std::uint64_t nodes = 0ULL;
            for(int i = 1; i <= depth; ++i)
            {
                auto start = std::chrono::high_resolution_clock::now();
                nodes = perft_tt(&sc.tt, sc.pos, i);
                auto finish = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed = finish - start;

                std::cout << "info"
                          << " depth " << i
                          << " nodes " << nodes
                          << " time " << static_cast<int>(elapsed.count()*1000)
                          << " nps " << static_cast<int>(nodes/elapsed.count())
                          << std::endl;
            }

            std::cout << "nodes " << nodes << std::endl;
        }
    }

    void ucinewgame()
    {
        parse_fen_to_position(
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            sc.pos
        );
        tt_clear(&sc.tt);
    }

    void isready()
    {
        std::cout << "readyok" << std::endl;
    }

    // setoption name Some Name value Some Value
    void setoption(std::stringstream& ss)
    {
        std::string word;
        std::string name = "";
        std::string value = "";

        if (ss >> word && word != "name") {
            return;
        }

        // Collect the name
        while (ss >> word && word != "value") {
            if (name != "") {name += " ";}
            name += word;
        }

        // Collect the value
        while (ss >> word) {
            if (value != "") {value += " ";}
            value += word;
        }

        // Set the option
        if (name != "" && value != "") {
            // TODO:
            // Set the option if found
        }
    }

    void go(std::stringstream& ss)
    {
        sc.max_depth = MAX_PLY;
        sc.moves_per_session = 0;
        sc.increment = 0;
        sc.search_start_time = 0;
        sc.search_end_time = 0;
        sc.movetime = 0;

        int wtime = -1;
        int btime = -1;
        int winc = -1;
        int binc = -1;

        std::string word;
        while (ss >> word) {
            if (word == "wtime")          {ss >> wtime;}
            else if (word == "btime")     {ss >> btime;}
            else if (word == "winc")      {ss >> winc;}
            else if (word == "binc")      {ss >> binc;}
            else if (word == "depth")     {ss >> sc.max_depth;}
            else if (word == "movetime")  {ss >> sc.movetime;}
            else if (word == "movestogo") {ss >> sc.moves_per_session;}
        }

        if (sc.pos.flipped) {
            sc.our_clock = btime;
            sc.increment = binc;
        } else {
            sc.our_clock = wtime;
            sc.increment = winc;
        }

        std::thread search(start_search, std::ref(sc));
        search.detach();
    }

    void moves(std::stringstream& ss)
    {
        std::string word;
        while (ss >> word) {
            Move move;
            if (!lan_to_move(sc.pos, word.c_str(), move)) {
                return;
            }

            char move_str[5];
            move_to_lan(move_str, move);

            if (sc.pos.flipped) {
                move = flip_move(move);
            }

            make_move(sc.pos, move);
        }
    }

    void position(std::stringstream& ss)
    {
        std::string fen = "";

        std::string word;
        while (ss >> word && word != "moves") {
            if (word == "startpos") {
                fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
            } else if (word != "fen") {
                if (fen != "") {fen += " ";}
                fen += word;
            }
        }

        if (fen != "") {
            parse_fen_to_position(fen.c_str(), sc.pos);
        }

        moves(ss);
    }

    void listen()
    {
        std::cout << "id name Monochrome" << std::endl;
        std::cout << "id author flok Gikoskos kz04px mkchan ZirconiumX" << std::endl;
        std::cout << "uciok" << std::endl;

        std::string word;
        std::string line;
        while (true) {
            std::getline(std::cin, line);
            std::stringstream ss {line};
            ss >> word;

            if (word == "isready")         {isready(); break;}
            else if (word == "setoption")  {setoption(ss);}
            else if (word == "quit")       {return;}
        }

        tt_create(&sc.tt, 128);
        ucinewgame();

        bool quit = false;
        while (!quit) {
            std::getline(std::cin, line);
            std::stringstream ss {line};
            ss >> word;

            if (word == "isready")         {isready();}
            else if (word == "ucinewgame") {ucinewgame();}
            else if (word == "position")   {position(ss);}
            else if (word == "go")         {go(ss);}
            else if (word == "print")      {Extension::print();}
            else if (word == "perft")      {Extension::perft(ss);}
            else if (word == "ttperft")    {Extension::ttperft(ss);}
            else if (word == "moves")      {moves(ss);}
            else if (word == "quit")       {quit = true;}
        }
    }
}
