/*
 * GameServer.cpp
 *
 *  Created on: Jan 25, 2018
 *      Author: Eghosa
 */

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <math.h>
#include <random>
#include <sstream>

using namespace std;

class GameBoard;
class Server;
class Cell;

class Cell {
	int bombCount;
	bool revealed;
	bool flagged;
	bool bomb;
	int rowPos;
	int colPos;
	string repr;

	Cell() : bombCount(0), revealed(false), flagged(false), bomb(false), rowPos(-1), colPos(-1), repr("-"){}
	Cell(bool isBomb, int row, int col) : bombCount(0), revealed(false), flagged(false), bomb(isBomb), rowPos(row), colPos(col), repr(isBomb ? "B": "-") {}

public:
	static Cell* createCell(bool isBomb, int row, int col);
	static Cell* emptyCell();
	void flag();
	void deflag();
	void reveal();
	bool isBomb();
	bool isRevealed();
	bool isFlagged();
	void bombHit() {repr = "X";}
	string getRepr() {
		if(!isRevealed() && !isFlagged()) {
			return "-";
		} else if(!isRevealed() && isFlagged()) {
			return "F";
		} else if(isRevealed() && isBomb()) {
			return repr;
		} else if(isRevealed() && getCount() == 0) {
			return " ";
		} else {
			return to_string(getCount()) ;
		}
	}
	void updateCount(int count);
	int getRow() {return rowPos;}
	int getCol() {return colPos;}
	int getCount() {
		return bombCount;
	}
	friend std::ostream &operator<<(std::ostream &os, const Cell* &c);

};



class GameBoard {
	int numCols;
	int numRows;
	int numBombs;
	vector<vector<Cell*>> board;
	string BOOM = "BOOM!";
	static constexpr double PROB_OF_BOMB = 0.10;
	void updateBombCounts();
	bool isValidCoord(int x, int y);
	vector<Cell*> getNeighbors(Cell *cell);
	void revealNeighbors(Cell *cell);
	bool bombDug = false;
	bool bombOrEmpty() {
		return (double)(rand()%100/100.0) >= PROB_OF_BOMB ?  false: true;
	}

 public:
	GameBoard() {
		numRows = 12;
		numCols = 12;
		numBombs = 0;
		for(int i = 0; i < numRows; ++i) {
			vector<Cell*> row;
			for(int j = 0; j < numCols; ++j) {
				Cell* c = Cell::createCell(bombOrEmpty(),i,j);
				c->isBomb() ? numBombs++ : numBombs+=0;
				row.emplace_back(c);
			}
			board.push_back(row);
		}
		updateBombCounts();

	}

	GameBoard(int rows, int cols) {
		numRows = rows;
		numCols = cols;
		numBombs = 0;
		for(int i = 0; i < numRows; ++i) {
			vector<Cell*> row;
			for(int j = 0; j < numCols; ++j) {
				Cell* c = Cell::createCell(bombOrEmpty(),i,j);
				c->isBomb() ? numBombs++ : numBombs+=0;
				row.push_back(c);
			}
			board.push_back(row);
		}
		updateBombCounts();

	}

	void reveal(Cell*);
	void flag(Cell*);
	void deflag(Cell*);
	Cell* getCell(int,int);
	bool bombHit() { return bombDug;}
	bool gameEnded();
	int getNumBombs() {return numBombs;}
	int nRows() { return numRows;}
	int nCols() {return numCols; }

	static void deleteCells(GameBoard game);
	friend std::ostream &operator<<(std::ostream &os, const GameBoard &gb);
	friend std::istream &operator>>(std::istream &is, GameBoard &gb);


};


bool GameBoard::isValidCoord(int row, int col) {
	bool validRow = 0 <= row && row < numRows;
	bool validCol = 0 <= col && col < numCols;
	return validRow && validCol;
}


void Cell::flag() {
	flagged = true;
}

void Cell::deflag() {
	flagged = false;
}

void Cell::reveal() {
	revealed = true;
}

bool Cell::isBomb() {
	return bomb;
}

bool Cell::isFlagged() {
	return flagged;
}

bool Cell::isRevealed() {
	return revealed;
}
void Cell::updateCount(int count) {
	bombCount = count;
}

Cell* Cell::createCell(bool isBomb, int row, int col) {
	Cell* cell = new Cell(isBomb,row,col);
	return cell;
}

Cell* Cell::emptyCell() {
	Cell* cell = new Cell();
	return cell;
}

std::ostream &operator<<(std::ostream &os,Cell *c) {

	os << c->getRepr() << ' ';
	return os;
}

void GameBoard::updateBombCounts() {
	for(int i = 0; i < numRows; ++i) {
		for(int j = 0; j < numCols; ++j) {
			if(board[i][j]->isBomb())
				continue;
			vector<Cell*> neighbors = getNeighbors(board[i][j]);
			int bombCount = 0;
			for(Cell* &c: neighbors) {
				if(c->isBomb())
					bombCount++;
			}
			board[i][j]->updateCount(bombCount);
		}
	}
}

vector<Cell*> GameBoard::getNeighbors(Cell *cell) {
	vector<Cell*> validN;
	for(int dr = -1; dr < 2; ++dr) {
		for(int dc = -1; dc < 2; ++dc) {
			if(dr == 0 && dc == 0)
				continue;
			int deltaRow = cell->getRow()+dr;
			int deltaCol = cell->getCol()+dc;
			if(isValidCoord(deltaRow,deltaCol)) {
				validN.push_back(board[deltaRow][deltaCol]);
			}
		}
	}
	return validN;
}
void GameBoard::revealNeighbors(Cell *cell) {

	if(!cell->isRevealed()) {
		cell->reveal();
		if(cell->getCount() == 0) {
			vector<Cell*> neighbors = getNeighbors(cell);
			for(Cell* &n: neighbors) {
				if(!n->isBomb())
					revealNeighbors(n);
			}
		}
	}

}

void GameBoard::reveal(Cell *cell) {
	if(cell->isFlagged()) {

	} else if(cell->isBomb()) {
		cell->bombHit();
		bombDug = true;
		cell->reveal();
	} else {
		revealNeighbors(cell);
	}

}
void GameBoard::flag(Cell *cell) {
	if(!cell->isRevealed()) {
		cell->flag();
	}
}

void GameBoard::deflag(Cell *cell) {
	if(!cell->isRevealed() && cell->isFlagged()) {
		cell->deflag();
	}
}

Cell* GameBoard::getCell(int row, int col) {

	if(isValidCoord(row,col)) {
		return board[row][col];
	} else {
		return Cell::emptyCell();
	}
}


void GameBoard::deleteCells(GameBoard game) {
	for(auto &row: game.board) {
		for(Cell* &c: row) {
			delete c;
		}
	}
}

bool GameBoard::gameEnded() {

	vector<bool> gameOngoing;
	for(auto &row: board) {
		for(Cell* &c: row) {
			if(c->isBomb()) {
				if(!c->isRevealed() || c->isFlagged()) {
					gameOngoing.push_back(false);
				} else {
					gameOngoing.push_back(true);
				}
			} else {
				gameOngoing.push_back(c->isRevealed() ? false: true);
			}
		}
	}
	return all_of(gameOngoing.begin(),gameOngoing.end(), [](bool b){return b==false;});
}
std::ostream &operator<<(std::ostream &os, const GameBoard &gb) {

	if(gb.numRows > 10) {
		os << "   ";
	} else {
		os << "  ";
	}

	for(int i = 0; i < gb.numCols; ++i) {
		if(gb.numCols > 10) {
			os << i << "  ";
		} else {
			os << i << "  ";
		}

	}
	os << endl;
	for(int i = 0; i < gb.numRows; ++i) {
		if(i > 10) {
			os << i << "  ";
		} else {
			os << i << " ";
		}
		for(int j = 0; j < gb.numCols; ++j) {
			if(j > 10) {
				os << gb.board[i][j] << "  ";
			} else {
				os << gb.board[i][j] << " ";
			}
		}
		os << endl;
	}
	return os;
}



std::istream &operator>>(std::istream &is, GameBoard &gb) {

	int row, col;
	string commandLine;
	getline(is,commandLine);
	std::istringstream iss(commandLine);
	std::vector<std::string> tokens; // done!

	for(std::string s; iss >> s; ) {
	    tokens.push_back(s);
	}
	if(tokens.size() < 3) {
		return is;
	}
	string command;
	if(tokens[0] == "dig") {
		row = stoi(tokens[1]);
		col = stoi(tokens[2]);
		Cell* c = gb.getCell(row,col);
		gb.reveal(c);
	} else if(tokens[0] == "flag") {
		row = stoi(tokens[1]);
		col = stoi(tokens[2]);
		Cell* c = gb.getCell(row,col);
		gb.flag(c);
	} else if(tokens[0] == "deflag") {
		row = stoi(tokens[1]);
		col = stoi(tokens[2]);
		Cell* c = gb.getCell(row,col);
		gb.deflag(c);
	}
	return is;

}



int main() {

	GameBoard game;
	cout << "Welcome to Minesweeper Enter a size(<rows> <columns>) for your board or press 'enter' to continue!" << endl;
	if(cin.peek() != '\n' || cin.peek() != '\r') {
		int row, col;
		cin >> row >> col;
		game = GameBoard(row,col);
	} else {
		game = GameBoard();
	}
	cout << "New Game Beginning, " << "Num bombs: " << game.getNumBombs() << endl;
	cout << "Supported commands are formatted as follows: 'dig X Y';'flag X Y';'deflag X Y';" << endl;
//	cout << game;

	bool gameOver = false;
	while(!gameOver) {
		cin >> game;
		gameOver = game.gameEnded();
		cout << game;
	}
	if(game.bombHit()) {
		cout << "Game Over! You Lost!" << endl;
	} else {
		cout << "You Won!" << endl;
	}
	GameBoard::deleteCells(game);
	return 0;
}

