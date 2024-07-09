#include <iostream>
#include <cmath>
#include <SDL.h>
#include <SDL_image.h>
// Абстрактные классы

class Piece {
public:
	Piece(char Color) : PieceColor(Color) {}
	~Piece() {}
	bool firstMove = true;
	char GetColor() {
		return PieceColor;
	}
	virtual char GetType() = 0;
	virtual bool IsMoveLegal(int& posI, int& posJ, int dstI, int dstJ, Piece* tiles[8][8]) = 0;
private:
	char PieceColor;
};

// Инициализация

SDL_Window* window;
SDL_Renderer* ren;

Piece* tiles[8][8];

bool isRunning = true;
char currentPlayerColor = 'w';

int tileSize = 85;
int offset = 100;
int tileIndexI = -1;
int tileIndexJ = -1;

int mouseX;
int mouseY;

SDL_Event event;

void deInit(int error);

void Init();

// Отрисовка

SDL_Texture* LoadTexture(SDL_Rect* rect);

void DrawBoard(int startX, int startY, int tileSize);

void DrawPieces(SDL_Texture* piecesTexture, SDL_Rect rect, int tileSize, Piece* tiles[8][8]);

void DrawCheck(Piece* tiles[8][8], int tileSize);

void DrawSelectionAndPossibleMoves(Piece* tiles[8][8], int tileSize, int posI, int posJ, char currentPlayerColor);

// Функции

void StartGame(Piece* tiles[8][8]);
	
void Castle(int& posI, int& posJ, int dstI, int dstJ, Piece* tiles[8][8]);

int GetTileIndexI(Piece* tiles[8][8], int tileSize, int mouseY);

int GetTileIndexJ(Piece* tiles[8][8], int tileSize, int mouseX);

void MovePiece(int& posI, int& posJ, int dstI, int dstJ, char& currentPlayerColor, Piece* tiles[8][8]);

void SelectPiece(int& tileIndexI, int& tileIndexJ, int mouseX, int mouseY, int tileSize, char& currentTurn, Piece* tiles[8][8]);

bool LookForCheck(Piece* tiles[8][8], char kingColor);

void LookForMateAndStalemate(Piece* tiles[8][8], char kingColor);

bool CanMove(char currentPlayerColor, Piece* tiles[8][8]);

void LookForPromotion(Piece* tiles[8][8]);

// Классы

class Pawn : public Piece {
public:
	Pawn(char Color) : Piece(Color) {}
	~Pawn() {}
	char GetType() {
		return 'p';
	}
	bool IsMoveLegal(int& posI, int& posJ, int dstI, int dstJ, Piece* tiles[8][8]) {
		if (!(posI == dstI && posJ == dstJ)) {
			if (tiles[dstI][dstJ] == 0 || (tiles[posI][posJ]->GetColor() != tiles[dstI][dstJ]->GetColor())) {
				if (this->GetColor() == 'w') {
					if (firstMove && dstI == posI - 2 && dstJ == posJ && tiles[dstI][dstJ] == 0) {
						return true;
					}
					if (dstI == posI - 1 && dstJ == posJ && tiles[dstI][dstJ] == 0) {
						return true;
					}
					if (dstI == posI - 1 && (dstJ == posJ + 1 || dstJ == posJ - 1) && tiles[dstI][dstJ] != 0) {
						return true;
					}
				}
				else {
					if (firstMove && dstI == posI + 2 && dstJ == posJ && tiles[dstI][dstJ] == 0) {
						return true;
					}
					if (dstI == posI + 1 && dstJ == posJ && tiles[dstI][dstJ] == 0) {
						return true;
					}
					if (dstI == posI + 1 && (dstJ == posJ + 1 || dstJ == posJ - 1) && tiles[dstI][dstJ] != 0) {
						return true;
					}
				}
			}
		}
		return false;
	}
};

class Bishop : public Piece {
public:
	Bishop(char Color) : Piece(Color) {}
	~Bishop() {}
	char GetType() {
		return 'b';
	}
	bool IsMoveLegal(int& posI, int& posJ, int dstI, int dstJ, Piece* tiles[8][8]) {
		if (!(posI == dstI && posJ == dstJ)) {
			if (tiles[dstI][dstJ] == 0 || (tiles[posI][posJ]->GetColor() != tiles[dstI][dstJ]->GetColor())) {
				if (fabs(dstI - posI) == 1 && fabs(dstJ - posJ) == 1) {
					return true;
				}
				if (fabs(dstI - posI) == fabs(dstJ - posJ)) {
					int offsetI = (dstI - posI > 0) ? 1 : -1;
					int offsetJ = (dstJ - posJ > 0) ? 1 : -1;
					for (int i = posI + offsetI, j = posJ + offsetJ; i != dstI; i += offsetI, j += offsetJ) {
						if (tiles[i][j] != 0) {
							return false;
						}
					}
					return true;
				}
			}
		}
		return false;
	}
};

class Knight : public Piece {
public:
	Knight(char Color) : Piece(Color) {}
	~Knight() {}
	char GetType() {
		return 'n';
	}
	bool IsMoveLegal(int& posI, int& posJ, int dstI, int dstJ, Piece* tiles[8][8]) {
		if (!(posI == dstI && posJ == dstJ)) {
			if (tiles[dstI][dstJ] == 0 || (tiles[posI][posJ]->GetColor() != tiles[dstI][dstJ]->GetColor())) {
				if ((fabs(dstI - posI) == 2 && fabs(dstJ - posJ) == 1) || (fabs(dstI - posI) == 1 && fabs(dstJ - posJ) == 2)) {
					return true;
				}
			}
		}
		return false;
	}
};

class Rook : public Piece {
public:
	Rook(char Color) : Piece(Color) {}
	~Rook() {}
	char GetType() {
		return 'r';
	}
	bool IsMoveLegal(int& posI, int& posJ, int dstI, int dstJ, Piece* tiles[8][8]) {
		if (!(posI == dstI && posJ == dstJ)) {
			if (tiles[dstI][dstJ] == 0 || (tiles[posI][posJ]->GetColor() != tiles[dstI][dstJ]->GetColor())) {
				if (dstI == posI) {
					int offsetJ = (dstJ - posJ > 0) ? 1 : -1;
					for (int j = posJ + offsetJ; j != dstJ; j += offsetJ) {
						if (tiles[posI][j] != 0) {
							return false;
						}
					}
					return true;
				}
				else if (dstJ == posJ) {
					int offsetI = (dstI - posI > 0) ? 1 : -1;
					for (int i = posI + offsetI; i != dstI; i += offsetI) {
						if (tiles[i][posJ] != 0) {
							return false;
						}
					}
					return true;
				}
			}
		}
		return false;
	}
};

class Queen : public Piece {
public:
	Queen(char Color) : Piece(Color) {}
	~Queen() {}
	char GetType() {
		return 'q';
	}
	bool IsMoveLegal(int& posI, int& posJ, int dstI, int dstJ, Piece* tiles[8][8]) {
		if (!(posI == dstI && posJ == dstJ)) {
			if (tiles[dstI][dstJ] == 0 || (tiles[posI][posJ]->GetColor() != tiles[dstI][dstJ]->GetColor())) {
				if (dstI == posI) {
					int offsetJ = (dstJ - posJ > 0) ? 1 : -1;
					for (int j = posJ + offsetJ; j != dstJ; j += offsetJ) {
						if (tiles[posI][j] != 0) {
							return false;
						}
					}
					return true;
				}
				else if (dstJ == posJ) {
					int offsetI = (dstI - posI > 0) ? 1 : -1;
					for (int i = posI + offsetI; i != dstI; i += offsetI) {
						if (tiles[i][posJ] != 0) {
							return false;
						}
					}
					return true;
				}
				else if (fabs(dstI - posI) == 1 && fabs(dstJ - posJ) == 1) {
					return true;
				}
				else if (fabs(dstI - posI) == fabs(dstJ - posJ)) {
					int offsetI = (dstI - posI > 0) ? 1 : -1;
					int offsetJ = (dstJ - posJ > 0) ? 1 : -1;
					for (int i = posI + offsetI, j = posJ + offsetJ; i != dstI; i += offsetI, j += offsetJ) {
						if (tiles[i][j] != 0) {
							return false;
						}
					}
					return true;
				}
			}
		}
		return false;
	}
};

class King : public Piece {
public:
	King(char Color) : Piece(Color) {}
	~King() {}
	char GetType() {
		return 'k';
	}
	bool IsMoveLegal(int& posI, int& posJ, int dstI, int dstJ, Piece* tiles[8][8]) {
		if (!(posI == dstI && posJ == dstJ)) {
			if (tiles[dstI][dstJ] == 0 || (tiles[posI][posJ]->GetColor() != tiles[dstI][dstJ]->GetColor())) {
				if ((fabs(dstI - posI) == 1 && fabs(dstJ - posJ) == 0) || (fabs(dstI - posI) == 0 && fabs(dstJ - posJ) == 1) || (fabs(dstI - posI) == 1 && fabs(dstJ - posJ) == 1)) {
					return true;
				}
			}
			else if (this->firstMove && !LookForCheck(tiles, this->GetColor()) && tiles[dstI][dstJ] != 0 && tiles[dstI][dstJ]->GetColor() == this->GetColor() && tiles[dstI][dstJ]->GetType() == 'r') {
				int offsetJ = (dstJ - posJ > 0) ? 1 : -1;
				for (int j = posJ + offsetJ; j != dstJ; j += offsetJ) {
					if (tiles[posI][j] != 0) {
						return false;
					}
				}
				for (int m = 0; m < 8; m++) {
					for (int n = 0; n < 8; n++) {
						for (int j = posJ + offsetJ; j != dstJ; j += offsetJ) {
							if (tiles[m][n] != 0 && tiles[m][n]->GetColor() != this->GetColor() && tiles[m][n]->IsMoveLegal(m, n, posI, j, tiles)) {
								return false;
							}
						}
					}
				}
				return true;
			}
		}
		return false;
	}
};

int main(int argc, char* argv[])
{
	
	Init();
	StartGame(tiles);
	SDL_SetRenderDrawColor(ren, 105, 105, 105, 1);
	SDL_RenderClear(ren);
	DrawBoard(offset, offset, tileSize);
	SDL_Rect rect;
	SDL_Texture* piecesTexture = LoadTexture(&rect);
	rect.w = rect.h / 2;
	rect.h = rect.h / 2;
	DrawPieces(piecesTexture, rect, tileSize, tiles);

	while (isRunning) {
		SDL_PollEvent(&event);
		switch (event.type) {
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT) {
				mouseX = event.button.x;
				mouseY = event.button.y;
				SelectPiece(tileIndexI, tileIndexJ, mouseX, mouseY, tileSize, currentPlayerColor, tiles);				
			}
			break;
		}

		LookForPromotion(tiles);



		DrawBoard(offset, offset, tileSize);
		DrawCheck(tiles, tileSize);
		if (tileIndexI >= 0 && tileIndexJ >= 0) {
			DrawSelectionAndPossibleMoves(tiles, tileSize, tileIndexI, tileIndexJ, currentPlayerColor);
		}
		DrawPieces(piecesTexture, rect, tileSize, tiles);
		SDL_RenderPresent(ren);

		SDL_Delay(20);
	}

	SDL_DestroyTexture(piecesTexture);
	deInit(0);
	return 0;
}

// Инициализация

void deInit(int error) {
	if (ren != NULL)
		SDL_DestroyRenderer(ren);
	if (window != NULL)
		SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();

	exit(error);
}

void Init() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "error" << std::endl;
		system("pause");
		deInit(1);
	}

	if (IMG_Init(IMG_INIT_PNG) == 0) {
		std::cout << "error" << std::endl;
		system("pause");
		deInit(1);
	}

	window = SDL_CreateWindow("Chess", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, tileSize * 8 + offset * 2, tileSize * 8 + offset * 2, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		std::cout << "error" << std::endl;
		system("pause");
		deInit(1);
	}

	ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (ren == NULL) {
		std::cout << "error" << std::endl;
		system("pause");
		deInit(1);
	}
}

// Отрисовка

SDL_Texture* LoadTexture(SDL_Rect* rect) {
	SDL_Surface* surface = IMG_Load("pieces.png");
	SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
	*rect = { 0 , 0 , surface->w, surface->h };
	SDL_FreeSurface(surface);
	return texture;
}

void DrawBoard(int startX, int startY, int tileSize) {
	SDL_Rect rect;
	for (int i = 0; i < 8; i++) {
		SDL_SetRenderDrawColor(ren, 255, 228, 196, 1);
		rect = { startX + tileSize * i, startY + tileSize * i , tileSize, tileSize };
		SDL_RenderFillRect(ren, &rect);
		for (int j = 2; j < 8 - i; j += 2) {
			rect = { startX + tileSize * i + tileSize * j, startY + tileSize * i, tileSize, tileSize };
			SDL_RenderFillRect(ren, &rect);
			rect = { startX + tileSize * i , startY + tileSize * i + tileSize * j , tileSize, tileSize };
			SDL_RenderFillRect(ren, &rect);
		}
	}

	for (int i = 0; i < 8; i++) {
		SDL_SetRenderDrawColor(ren, 205, 133, 63, 1);
		rect = { startX + tileSize * i, startY + tileSize * 7 - tileSize * i , tileSize, tileSize };
		SDL_RenderFillRect(ren, &rect);
		for (int j = 2; j < 8 - i; j += 2) {
			rect = { startX + tileSize * i + tileSize * j, startY + tileSize * 7 - tileSize * i, tileSize, tileSize };
			SDL_RenderFillRect(ren, &rect);
			rect = { startX + tileSize * i , startY + tileSize * 7 - tileSize * i - tileSize * j , tileSize, tileSize };
			SDL_RenderFillRect(ren, &rect);
		}
	}
}

void DrawPieces(SDL_Texture* piecesTexture, SDL_Rect rect, int tileSize, Piece* tiles[8][8]) {
	SDL_Rect dst = { offset, offset, rect.w / 4, rect.h / 4 };
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			dst = { offset + tileSize * j, offset + tileSize * i, rect.w / 4, rect.h / 4 };
			if (tiles[i][j] != 0) {
				if (tiles[i][j]->GetType() == 'p') {
					rect.x = rect.w * 5;
					if (tiles[i][j]->GetColor() == 'w') {
						rect.y = rect.h * 0;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
					else if (tiles[i][j]->GetColor() == 'b') {
						rect.y = rect.h * 1;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
				}
				else if (tiles[i][j]->GetType() == 'b') {
					rect.x = rect.w * 2;
					if (tiles[i][j]->GetColor() == 'w') {
						rect.y = rect.h * 0;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
					else if (tiles[i][j]->GetColor() == 'b') {
						rect.y = rect.h * 1;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
				}
				else if (tiles[i][j]->GetType() == 'n') {
					rect.x = rect.w * 3;
					if (tiles[i][j]->GetColor() == 'w') {
						rect.y = rect.h * 0;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
					else if (tiles[i][j]->GetColor() == 'b') {
						rect.y = rect.h * 1;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
				}
				else if (tiles[i][j]->GetType() == 'r') {
					rect.x = rect.w * 4;
					if (tiles[i][j]->GetColor() == 'w') {
						rect.y = rect.h * 0;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
					else if (tiles[i][j]->GetColor() == 'b') {
						rect.y = rect.h * 1;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
				}
				else if (tiles[i][j]->GetType() == 'q') {
					rect.x = rect.w * 1;
					if (tiles[i][j]->GetColor() == 'w') {
						rect.y = rect.h * 0;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
					else if (tiles[i][j]->GetColor() == 'b') {
						rect.y = rect.h * 1;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
				}
				else if (tiles[i][j]->GetType() == 'k') {
					rect.x = rect.w * 0;
					if (tiles[i][j]->GetColor() == 'w') {
						rect.y = rect.h * 0;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
					else if (tiles[i][j]->GetColor() == 'b') {
						rect.y = rect.h * 1;
						SDL_RenderCopy(ren, piecesTexture, &rect, &dst);
					}
				}
			}
		}
	}
}

void DrawCheck(Piece* tiles[8][8], int tileSize) {
	SDL_SetRenderDrawColor(ren, 255, 0, 0, 1);
	SDL_Rect checkRect;
	if (LookForCheck(tiles, 'w')) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (tiles[i][j] != 0 && tiles[i][j]->GetType() == 'k' && tiles[i][j]->GetColor() == 'w') {
					checkRect = { tileSize * j + offset, tileSize * i + offset, tileSize, tileSize };
					SDL_RenderFillRect(ren, &checkRect);
				}
			}
		}
	}
	if (LookForCheck(tiles, 'b')) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (tiles[i][j] != 0 && tiles[i][j]->GetType() == 'k' && tiles[i][j]->GetColor() == 'b') {
					checkRect = { tileSize * j + offset, tileSize * i + offset, tileSize, tileSize };
					SDL_RenderFillRect(ren, &checkRect);
				}
			}
		}
	}
}

void DrawSelectionAndPossibleMoves(Piece* tiles[8][8], int tileSize, int posI, int posJ, char currentPlayerColor) {
	SDL_Rect rect = {tileSize * posJ + offset, tileSize * posI + offset, tileSize, tileSize};
	SDL_SetRenderDrawColor(ren, 46, 139, 87, 1);
	SDL_RenderFillRect(ren, &rect);
	Piece* temp;

	if (tiles[posI][posJ]->GetColor() == currentPlayerColor) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if (tiles[posI][posJ]->IsMoveLegal(posI, posJ, i, j, tiles)) {
					temp = tiles[i][j];
					tiles[i][j] = tiles[posI][posJ];
					tiles[posI][posJ] = 0;
					if (LookForCheck(tiles, currentPlayerColor)) {
						if (temp != 0 && tiles[i][j]->GetType() == 'k' && temp->GetColor() == currentPlayerColor && temp->GetType() == 'r') {
							tiles[posI][posJ] = tiles[i][j];
							tiles[i][j] = temp;
							for (int l = 0; l < 4; l++) {
								rect = { tileSize * j + offset + l, tileSize * i + offset + l, tileSize - l * 2, tileSize - l * 2 };
								SDL_RenderDrawRect(ren, &rect);
							}
						}
						else {
							tiles[posI][posJ] = tiles[i][j];
							tiles[i][j] = temp;
						}
					}
					else {
						
						tiles[posI][posJ] = tiles[i][j];
						tiles[i][j] = temp;
						for (int l = 0; l < 4; l++) {
							rect = { tileSize * j + offset + l, tileSize * i + offset + l, tileSize - l * 2, tileSize - l * 2 };
							SDL_RenderDrawRect(ren, &rect);
						}
					}
				}
			}
		}
	}
}

// Функции

void StartGame(Piece* tiles[8][8]) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			tiles[i][j] = 0;
		}
	}

	tiles[0][0] = new Rook('b');
	tiles[0][7] = new Rook('b');
	tiles[7][0] = new Rook('w');
	tiles[7][7] = new Rook('w');

	tiles[0][1] = new Knight('b');
	tiles[0][6] = new Knight('b');
	tiles[7][1] = new Knight('w');
	tiles[7][6] = new Knight('w');

	tiles[0][2] = new Bishop('b');
	tiles[0][5] = new Bishop('b');
	tiles[7][2] = new Bishop('w');
	tiles[7][5] = new Bishop('w');

	tiles[0][3] = new Queen('b');
	tiles[0][4] = new King('b');
	tiles[7][3] = new Queen('w');
	tiles[7][4] = new King('w');

	for (int j = 0; j < 8; j++) {
		tiles[1][j] = new Pawn('b');
	}
	for (int j = 0; j < 8; j++) {
		tiles[6][j] = new Pawn('w');
	}

	currentPlayerColor = 'w';
}

int GetTileIndexI(Piece* tiles[8][8], int tileSize, int mouseY) {
	return (mouseY - offset) / tileSize;
}

int GetTileIndexJ(Piece* tiles[8][8], int tileSize, int mouseX) {
	return (mouseX - offset) / tileSize;
}

void MovePiece(int& posI, int& posJ, int dstI, int dstJ, char& currentPlayerColor, Piece* tiles[8][8]) {
	Piece* temp;
	if (tiles[posI][posJ]->IsMoveLegal(posI, posJ, dstI, dstJ, tiles)) {
		temp = tiles[dstI][dstJ];
		tiles[dstI][dstJ] = tiles[posI][posJ];
		tiles[posI][posJ] = 0;
		if (LookForCheck(tiles, currentPlayerColor)) {
			tiles[posI][posJ] = tiles[dstI][dstJ];
			tiles[dstI][dstJ] = temp;
		}
		else {
			if (tiles[dstI][dstJ]->GetType() == 'p' && tiles[dstI][dstJ]->firstMove) {
				tiles[dstI][dstJ]->firstMove = false;
			}
			if (tiles[dstI][dstJ]->GetType() == 'r' && tiles[dstI][dstJ]->firstMove) {
				tiles[dstI][dstJ]->firstMove = false;
			}
			if (tiles[dstI][dstJ]->GetType() == 'k' && tiles[dstI][dstJ]->firstMove) {
				tiles[dstI][dstJ]->firstMove = false;
			}
			if (currentPlayerColor == 'w') {
				currentPlayerColor = 'b';
				LookForMateAndStalemate(tiles, currentPlayerColor);
			}
			else {
				currentPlayerColor = 'w';
				LookForMateAndStalemate(tiles, currentPlayerColor);
			}
		}
	}
}

void Castle(int& posI, int& posJ, int dstI, int dstJ, Piece* tiles[8][8]) {
	Piece* tempK = tiles[posI][posJ];
	Piece* tempR = tiles[dstI][dstJ];
	if (tiles[posI][posJ]->IsMoveLegal(posI, posJ, dstI, dstJ, tiles)) {
		if (tiles[posI][posJ]->GetColor() == 'w') {
			if (dstJ == 0) {
				tiles[posI][posJ] = 0;
				tiles[7][2] = tempK;
				tiles[dstI][dstJ] = 0;
				tiles[7][3] = tempR;
				if (LookForCheck(tiles, 'w')) {
					tiles[posI][posJ] = tempK;
					tiles[dstI][dstJ] = tempR;
					tiles[7][2] = 0;
					tiles[7][3] = 0;
					return;
				}
				else {
					tiles[7][2]->firstMove = false;
					tiles[7][3]->firstMove = false;
				}
			}
			else if (dstJ == 7) {
				tiles[posI][posJ] = 0;
				tiles[7][6] = tempK;
				tiles[dstI][dstJ] = 0;
				tiles[7][5] = tempR;
				if (LookForCheck(tiles, 'w')) {
					tiles[posI][posJ] = tempK;
					tiles[dstI][dstJ] = tempR;
					tiles[7][6] = 0;
					tiles[7][5] = 0;
					return;
				}
				else {
					tiles[7][6]->firstMove = false;
					tiles[7][5]->firstMove = false;
				}
			}
		}
		else {
			if (dstJ == 0) {
				tiles[posI][posJ] = 0;
				tiles[0][2] = tempK;
				tiles[dstI][dstJ] = 0;
				tiles[0][3] = tempR;
				if (LookForCheck(tiles, 'b')) {
					tiles[posI][posJ] = tempK;
					tiles[dstI][dstJ] = tempR;
					tiles[0][2] = 0;
					tiles[3][3] = 0;
					return;
				}
				else {
					tiles[0][2]->firstMove = false;
					tiles[0][3]->firstMove = false;
				}
			}
			else if (dstJ == 7) {
				tiles[posI][posJ] = 0;
				tiles[0][6] = tempK;
				tiles[dstI][dstJ] = 0;
				tiles[0][5] = tempR;
				if (LookForCheck(tiles, 'b')) {
					tiles[posI][posJ] = tempK;
					tiles[dstI][dstJ] = tempR;
					tiles[0][6] = 0;
					tiles[3][5] = 0;
					return;
				}
				else {
					tiles[0][6]->firstMove = false;
					tiles[0][5]->firstMove = false;
				}
			}
		}
	}
	if (currentPlayerColor == 'w') {
		currentPlayerColor = 'b';
		LookForMateAndStalemate(tiles, currentPlayerColor);
	}
	else {
		currentPlayerColor = 'w';
		LookForMateAndStalemate(tiles, currentPlayerColor);
	}
}

void SelectPiece(int& tileIndexI, int& tileIndexJ, int mouseX, int mouseY, int tileSize, char& currentTurn, Piece* tiles[8][8]) {
	if ((mouseX - offset > 0 && mouseX - offset < tileSize * 8) && (mouseY - offset > 0 && mouseY - offset < tileSize * 8)) {
		if (tileIndexI >= 0 && tileIndexJ >= 0) {

			if (tiles[tileIndexI][tileIndexJ] != 0 && tiles[GetTileIndexI(tiles, tileSize, mouseY)][GetTileIndexJ(tiles, tileSize, mouseX)] != 0) {
				if (tiles[tileIndexI][tileIndexJ]->firstMove && tiles[GetTileIndexI(tiles, tileSize, mouseY)][GetTileIndexJ(tiles, tileSize, mouseX)]->firstMove) {
					if (tiles[tileIndexI][tileIndexJ]->GetType() == 'k' && tiles[GetTileIndexI(tiles, tileSize, mouseY)][GetTileIndexJ(tiles, tileSize, mouseX)]->GetType() == 'r') {
						if (tiles[tileIndexI][tileIndexJ]->GetColor() == tiles[GetTileIndexI(tiles, tileSize, mouseY)][GetTileIndexJ(tiles, tileSize, mouseX)]->GetColor()) {
							if (!LookForCheck(tiles, tiles[tileIndexI][tileIndexJ]->GetColor())) {
								Castle(tileIndexI, tileIndexJ, GetTileIndexI(tiles, tileSize, mouseY), GetTileIndexJ(tiles, tileSize, mouseX), tiles);
								tileIndexI = -1;
								tileIndexJ = -1;
								return;
							}
							else {
								return;
							}
						}
					}
				}
			}
			MovePiece(tileIndexI, tileIndexJ, GetTileIndexI(tiles, tileSize, mouseY), GetTileIndexJ(tiles, tileSize, mouseX), currentTurn, tiles);
			tileIndexI = -1;
			tileIndexJ = -1;
		}
		else {
			if (tiles[GetTileIndexI(tiles, tileSize, mouseY)][GetTileIndexJ(tiles, tileSize, mouseX)] != 0 && tiles[GetTileIndexI(tiles, tileSize, mouseY)][GetTileIndexJ(tiles, tileSize, mouseX)]->GetColor() == currentTurn) {
				tileIndexI = GetTileIndexI(tiles, tileSize, mouseY);
				tileIndexJ = GetTileIndexJ(tiles, tileSize, mouseX);
			}
		}
	}
	else {
		tileIndexI = -1;
		tileIndexJ = -1;
	}
}

bool LookForCheck(Piece* tiles[8][8], char kingColor) {
	int kingI;
	int kingJ;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (tiles[i][j] != 0 && tiles[i][j]->GetType() == 'k' && tiles[i][j]->GetColor() == kingColor) {
				kingI = i;
				kingJ = j;
			}
		}
	}
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (tiles[i][j] != 0 && tiles[i][j]->GetColor() != kingColor && tiles[i][j]->IsMoveLegal(i, j, kingI, kingJ, tiles)) {
				return true;
			}
		}
	}
	return false;
}

void LookForMateAndStalemate(Piece* tiles[8][8], char kingColor) {
	if (!CanMove(kingColor, tiles) && LookForCheck(tiles, kingColor)) {
		std::cout << "Checkmated : " << kingColor << std::endl;
	}
	else if (!CanMove(kingColor, tiles)) {
		std::cout << "Stalemate " << std::endl;
	}
}

bool CanMove(char currentPlayerColor, Piece* tiles[8][8]) {
	Piece* temp;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (tiles[i][j] != 0 && tiles[i][j]->GetColor() == currentPlayerColor) {
				for (int m = 0; m < 8; m++) {
					for (int n = 0; n < 8; n++) {
						if (tiles[i][j]->IsMoveLegal(i, j, m, n, tiles)) {
							temp = tiles[m][n];
							tiles[m][n] = tiles[i][j];
							tiles[i][j] = 0; 
							if (LookForCheck(tiles, currentPlayerColor)) {
								tiles[i][j] = tiles[m][n];
								tiles[m][n] = temp;
							}

							else {
								tiles[i][j] = tiles[m][n];
								tiles[m][n] = temp;
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

void LookForPromotion(Piece* tiles[8][8]) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (tiles[i][j] != 0 && tiles[i][j]->GetType() == 'p') {
				if (tiles[i][j]->GetColor() == 'w' && i == 0) {
					tiles[i][j] = new Queen('w');
				}
				else if (tiles[i][j]->GetColor() == 'b' && i == 7){
					tiles[i][j] = new Queen('b');
				}
			}
		}
	}
}
