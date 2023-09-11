#include <iostream>
#include <cassert>
#include "board.h"

Board::Board() {
    turn = WHITE;
    // these variables track whether or not either player is allowed to castle
    white_castle = true;
    black_castle = true;

    white_en_passant = NULLCOORD;
    black_en_passant = NULLCOORD;
    
    board[0][0] = new Rook(BLACK);
    board[0][1] = new Knight(BLACK);
    board[0][2] = new Bishop(BLACK);
    board[0][3] = new Queen(BLACK);
    board[0][4] = new King(BLACK);
    board[0][5] = new Bishop(BLACK);
    board[0][6] = new Knight(BLACK);
    board[0][7] = new Rook(BLACK);

    for (int i = 0; i < 8; i++) {
        board[1][i] = new Pawn(WHITE);
        board[6][i] = new Pawn(WHITE);
    }

    board[7][0] = new Rook(WHITE);
    board[7][1] = nullptr;//new Knight(WHITE);
    board[7][2] = nullptr;//new Bishop(WHITE);
    board[7][3] = nullptr;//new Queen(WHITE);
    board[7][4] = new King(WHITE);
    board[7][5] = nullptr;//new Bishop(WHITE);
    board[7][6] = nullptr;//new Knight(WHITE);
    board[7][7] = new Rook(WHITE);

    for (int i = 2; i < 6; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = nullptr;
        }
    }
}

Board::~Board() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            delete board[i][j];
        }
    }
}

Piece* Board::get_piece(Coord coord) const {
    return board[coord.row][coord.column];
}

Coord Board::input_to_coord(std::string input) { // it's magic!
    char rowchar = input[1];
    char colchar = input[0];

    const char first_row = '8';
    const char first_col = 'A';
    const char last_row = '1';
    const char last_col = 'H';

    if ((input.length() != 2) || 
    !(rowchar >= last_row && rowchar <= first_row) || 
    !(colchar >= first_col && colchar <= last_col)) {
        return NULLCOORD;
    }
    int row = first_row - rowchar;
    int column = toupper(colchar) - first_col;
    return Coord {row, column};
}

void Board::display(Coord selected_piece, std::unordered_set<Coord> possible_moves) const {
    std::system("clear");

    std::cout << C_BOLD;
    std::cout << "     A   B   C   D   E   F   G   H " << std::endl;

    int row, col;

    for (int i = 0; i < 8; i++) {
        row = i;
        std::cout << 8-row << "  ";
        for (int j = 0; j < 8; j++) {
            col = j;
            bool option = false;
            Coord square = Coord {row, col};
            Piece* p = get_piece(square);
            if (possible_moves.find(square) != possible_moves.end()) {
                option = true;
            }
            if (p == nullptr) {
                std::cout << "| ";
                if (option) {
                    std::cout << C_OPTION << "•";
                } else {
                std::cout << " ";
                }
            } else {
                std::cout << "| ";
                if (selected_piece != NULLCOORD && selected_piece == square) {
                    std::cout << C_SELECT;
                }
                else if (option) {
                    std::cout << C_OPTION;
                }
                else if (p->color == BLACK) {
                    std::cout << C_BLACK;
                } else {
                    std::cout << C_WHITE;
                }
                std::cout << p->get_symbol();
            }
            std::cout << " " << C_RESET << C_BOLD;
        }
        std::cout << std::endl << "---+---+---+---+---+---+---+---+---" << std::endl;
    }
    std::cout << C_RESET << std::endl;
}

void Board::display() const {
    display(NULLCOORD, {});
}

void Board::move_piece(Coord start_square, Coord end_square) {

    turn = !turn;
    Piece* start_piece = get_piece(start_square);
    Piece* end_piece = get_piece(end_square);
    handle_en_passant(start_square, end_square);

    bool castled = handle_castling(start_square, end_square);
    if (castled) {
        return;
    }

    board[start_square.row][start_square.column] = nullptr;
    if (end_piece != nullptr) {
        delete board[end_square.row][end_square.column];
    }
    board[end_square.row][end_square.column] = start_piece;
}

bool Board::handle_castling(Coord start_square, Coord end_square) {
    Piece* start_piece = get_piece(start_square);
    Piece* end_piece = get_piece(end_square);

    // disable castling if the rook or king are moved
    if (start_piece->get_piece_name() == "ROOK" || start_piece->get_piece_name() == "KING") {
        if (white_castle && start_piece->color == WHITE) {
            white_castle = false;
        }
        else if (black_castle && start_piece->color == BLACK) {
            black_castle = false;
        }
    }

    if (end_piece == nullptr || start_piece->color != end_piece->color) { // if the colors of the pieces match, a castle is taking place
        return false;
    }
    int row = start_piece->color ? 0 : 7; // row is 0 if BLACK, 7 if WHITE
    const Coord left_rook_square = Coord{row, 0};
    const Coord right_rook_square = Coord{row, 7};
    const Coord king_square = Coord{row, 4};

    Piece* rook;
    Piece* king;
    if (start_square == left_rook_square || start_square == right_rook_square) {
        rook = start_piece;
        king = end_piece;
    } else {
        rook = end_piece;
        king = start_piece;
    }


    if ((start_square == left_rook_square && end_square == king_square) 
    || (start_square == king_square && end_square == left_rook_square)) {
        board[row][2] = king;
        board[row][3] = rook;
        board[row][0] = nullptr;
        board[row][4] = nullptr;
    } else {
        board[row][5] = rook;
        board[row][6] = king;
        board[row][4] = nullptr;
        board[row][7] = nullptr;
    }
    return true;
}

void Board::handle_en_passant(Coord start_square, Coord end_square) {

    if (end_square == white_en_passant) {
        Piece* captured_pawn = board[end_square.row-1][end_square.column];
        assert(captured_pawn->get_piece_name() == "PAWN");
        delete board[end_square.row+1][end_square.column];
        board[end_square.row+1][end_square.column] = nullptr;
    } 
    else if (end_square == black_en_passant) {
        Piece* captured_pawn = board[end_square.row+1][end_square.column];
        assert(captured_pawn->get_piece_name() == "PAWN");
        delete board[end_square.row+1][end_square.column];
        board[end_square.row+1][end_square.column] = nullptr;
    }

    white_en_passant = NULLCOORD;
    black_en_passant = NULLCOORD;

    Piece* start_piece = get_piece(start_square);
    Piece* end_piece = get_piece(end_square);

    if (start_piece->get_piece_name() == "PAWN" && end_piece == nullptr) {
        if (start_piece->color == WHITE && (start_square.row - end_square.row == 2)) {
            white_en_passant = Coord {end_square.row+1, end_square.column};
        }
        else if (start_piece->color == BLACK && (end_square.row - start_square.row == 2)) {
            black_en_passant = Coord {end_square.row-1, end_square.column};
        }
    }
}

bool Board::get_possible_promo(Coord end_square) {
    Piece* piece = get_piece(end_square);
    // if a pawn has reached the end row
    return (piece->get_piece_name() == "PAWN" && end_square.row == (piece->color ? 7 : 0));
}

std::unordered_set<Coord> Board::get_possible_moves(Coord square) {
    Piece* piece = get_piece(square);
    std::unordered_set<Coord> possible_moves = piece->possible_moves(board, square);

    std::unordered_set<Coord> possible_castles = get_possible_castles(square);
    for (Coord c : possible_castles) {
        possible_moves.insert(c);
    }

    Coord en_passant = get_possible_en_passant(square);
    if (en_passant != NULLCOORD) {
        possible_moves.insert(en_passant);
    }

    // if the piece is a King, any moves that result in the King 
    // being captured are removed from the possible moves set
    if (piece->get_piece_name() == "KING") {
        std::unordered_set<Coord> barred_squares;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Piece* potential_enemy = board[i][j];
                if (potential_enemy != nullptr && potential_enemy->color != piece->color) {
                    barred_squares = potential_enemy->possible_moves(board, Coord {i, j});
                    for (Coord c : barred_squares) {
                        if (possible_moves.count(c) == 1) {
                            possible_moves.erase(c);
                        }
                    }
                }
            }
        }
    }
    return possible_moves;
}

std::unordered_set<Coord> Board::get_possible_castles(Coord square) {
    std::unordered_set<Coord> possible_moves;
    Piece* piece = get_piece(square);
    if (!(((piece->color == WHITE && white_castle) || (piece->color == BLACK && black_castle)) &&
    (piece->get_piece_name() == "ROOK" || piece->get_piece_name() == "KING")))  {
        return possible_moves;
    }

    int row = piece->color ? 0 : 7; // row is 0 if BLACK, 7 if WHITE

    int left_side_clear = true;
    int right_side_clear = true;
    for (int i = 1; i <= 3; i++) {
        if (board[row][i] != nullptr) {
            left_side_clear = false;
        }
    }
    for (int i = 5; i <= 6; i++) {
        if (board[row][i] != nullptr) {
            right_side_clear = false;
        }
    }

    const Coord left_rook_square = Coord{row, 0};
    const Coord right_rook_square = Coord{row, 7};
    const Coord king_square = Coord{row, 4};

    if ((square == left_rook_square && left_side_clear) ||
        (square == right_rook_square && right_side_clear)) { 
        possible_moves.insert(king_square); // Add King's position 
        return possible_moves;
    }
    if (square == king_square && left_side_clear) {
        possible_moves.insert(left_rook_square);
    }
    if (square == king_square && right_side_clear) {
        possible_moves.insert(right_rook_square);
    }
    return possible_moves;
}

Coord Board::get_possible_en_passant(Coord square) {
    Piece* piece = get_piece(square);
    if (black_en_passant != NULLCOORD && piece->color == WHITE) {
        Coord top_left = Coord{square.row - 1, square.column - 1};
        Coord top_right = Coord{square.row - 1, square.column + 1};
        if (top_left == black_en_passant) {
            return top_left;
        } else if (top_right == black_en_passant) {
            return top_right;
        }
    }
    else if (white_en_passant != NULLCOORD && piece->color == BLACK) {
        Coord top_left = Coord{square.row + 1, square.column - 1};
        Coord top_right = Coord{square.row + 1, square.column + 1};
        if (top_left == white_en_passant) {
            return top_left;
        } else if (top_right == white_en_passant) {
            return top_right;
        }
    }
    return NULLCOORD;
}

bool Board::promote_pawn(Coord square, std::string promo_piece) {
    Piece* piece = get_piece(square);
    bool color = piece->color;
    assert(piece->get_piece_name() == "PAWN");
    delete board[square.row][square.column];
    if (promo_piece == "QUEEN") {
        board[square.row][square.column] = new Queen(color);
    }
    else if (promo_piece == "BISHOP") {
        board[square.row][square.column] = new Bishop(color);
    }
    else if (promo_piece == "KNIGHT") {
        board[square.row][square.column] = new Knight(color);
    }
    else if (promo_piece == "ROOK") {
        board[square.row][square.column] = new Rook(color);
    }
    else {
        return false;
    }
    return true;
}
