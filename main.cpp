#include <iostream>
#include <unistd.h>
#include "board.h"

void slow_print(std::string text) {
    for (char c : text) {
        usleep(15000); // 0.015 seconds
        std::cout << c << std::flush;
    }
}

int main() {
    Board game;
    bool input_invalid;
    while (true) {
        game.display();

        // print this on reset if you screwed up the input
        if (input_invalid) {
            slow_print("INPUT INVALID, TRY AGAIN\n");
            input_invalid = false;
        }

        slow_print("CURRENT TURN: ");
        std::cout << C_BOLD;
        if (game.turn == WHITE) {
            std::cout << C_WHITE;
            slow_print("WHITE\n");
        } else {
            std::cout << C_BLACK;
            slow_print("BLACK\n");
        }
        std::cout << C_RESET;
        
        slow_print("SELECT A PIECE (SUCH AS 'E2') TO VIEW ITS MOVEMENT OPTIONS\n > ");
        std::string position;
        std::cin >> position;

        Coord selected_square = game.input_to_coord(position);
        // check if input is valid
        if (selected_square == NULLCOORD) {
            input_invalid = true;
            continue;
        }

        // check if selected piece is a valid piece
        Piece* selected_piece = game.get_piece(selected_square);
        if (selected_piece == nullptr || selected_piece->color != game.turn) {
            input_invalid = true;
            continue;
        }
        std::unordered_set<Coord> options = game.get_possible_moves(selected_square);


        std::string piece_name = selected_piece->get_piece_name();
        Coord target_square;
        while (true) {
            game.display(selected_square, options);
            if (input_invalid) {
                slow_print("INPUT INVALID, TRY AGAIN\n");
                input_invalid = false;
            }
            slow_print(piece_name + " SELECTED\n");
            slow_print("ENTER A SQUARE TO MOVE, OR PRESS 'Q' TO QUIT\n > ");
            std::string target;
            std::cin >> target;
            if (target.length() == 1 && (target[0] == 'Q' || target[0] == 'q')) {
                break;
            }
                
            target_square = game.input_to_coord(target);
            if (target_square == NULLCOORD || options.count(target_square) == 0) {
                input_invalid = true;
                continue;
            }
            game.move_piece(selected_square, target_square);
            break;
        }
        if (game.get_possible_promo(target_square)) {
            while (true) {
                game.display();
                if (input_invalid) {
                    slow_print("INPUT INVALID, TRY AGAIN\n");
                    input_invalid = false;
                }
                slow_print("YOUR PAWN MAY BE PROMOTED. SELECT A PIECE\n");
                slow_print("QUEEN / BISHOP / KNIGHT / ROOK \n");
                slow_print(" > ");
                std::string promo;
                std::cin >> promo;
                bool input_success = game.promote_pawn(target_square, promo);
                if (!input_success) {
                    input_invalid = true;
                } else {
                    break;
                }
            }
        }
    }
}