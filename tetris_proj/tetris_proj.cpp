#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <stdio.h>
#include <Windows.h>

//Variables for the screen
int nScreenWidth = 120; //x
int nScreenHeight = 30; //y

//Tetris pieces
wstring tetromino[7];

//Playing field
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;


//Rotate piece
int Rotate(int px, int py, int r)
{
	int pi = 0;
	switch (r % 4)
	{
	case 0: pi = py * 4 + px;
		break;
	case 1: pi = 12 + py - (px * 4);  //90
		break;
	case 2: pi = 15 - (py * 4) - px;  //180 
		break;
	case 3: pi = 3 - py + (px * 4);   // 270
		break;

	}
	return pi;
}


//Tests to see if a piece fits into place

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	//Iterates across tetromino
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			//Get index piece
			int pi = Rotate(px, py, nRotation);

			//Get index field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			//PosY/PosX is top left of piece py/px is the index

			//Not going out of bounds
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					// In Bounds so do collision check
					if (tetromino[nTetromino][pi] != L'.' && pField[fi] != 0)
						return false;
				}
			}
		}

	return true;

}

//Main function
int main()
{
	//Create screen (Ya this like made no sense to me but I understood it a lil)
	wchar_t *screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//Assets
	/*Notes: Full stop symbol to represent empty space (.) Letter is used to represent the shape*/
	
	tetromino[0].append(L"..X...X...X...X."); 
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	//Setup field
	pField = new unsigned char[nFieldWidth * nFieldHeight];
	for (int x = 0; x < nFieldWidth; x++) //Boundary
		for (int y = 0; y < nFieldHeight; y++)
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	
	//Store keys
	bool bKey[4];
	

	//Piece 
	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	//Speed for game tick
	int nSpeed = 20;
	int nSpeedCount = 0;
	bool bForceDown = false;
	bool bRotateHold = true;
	int nPieceCount = 0;
	int nScore = 0;


	vector<int> vLines;

	//Game loop to see if the games over
	bool bGameOver = false;

	while (!bGameOver)
	{
		//Timing
		this_thread::sleep_for(50ms);
		nSpeedCount++;
		bForceDown = (nSpeedCount == nSpeed);

		//Input
		for (int k = 0; k < 4; k++)								
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		//Logic
		//Right Key
		if (bKey[0])
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY));
			{
				nCurrentX = nCurrentX + 1;
			}
		}
		//Left Key
		if (bKey[1])
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY));
			{
				nCurrentX = nCurrentX - 1;
			}
		}
		//Down Key
		if (bKey[2])
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1));
			{
				nCurrentY = nCurrentY + 1;
			}

		}
		//Rotation
		if (bKey[3])
		{
			if (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY));
			{
				nCurrentRotation = nCurrentRotation + 1;
			}
			bRotateHold = true;

		}
		else
		{
			bRotateHold = false;

		}

		//Makes piece move down
		if(bForceDown)
		{
			nSpeedCount = 0;
			nPieceCount++;
			if (nPieceCount % 50 == 0)
				if (nSpeed >= 10) nSpeed--;
			
			//Piece fits
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++;
			//Piece doesnt fit
			else
			{
				//Lock piece into place
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				//Increases the speed of the pieces
				nPieceCount++;
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10)
						nSpeed--;

				//Any lines created by a new piece
				/*Notes: Checks to see where the last piece was to see if a line was formed*/
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < nFieldHeight -1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						//If there is a line
						if (bLine)
						{
							//Remove the line
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;

							vLines.push_back(nCurrentY + py);
						}
					}

				//Score
				nScore += 25;
				if (!vLines.empty())	
					nScore += (1 << vLines.size()) * 100;

				//Next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				//Game Over if at the top
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nSpeedCount = 0;
		}
		//Render output

		//Draw Field
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

		// Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

		//Display Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		if (!vLines.empty())
		{
			//Display
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, & dwBytesWritten);
			this_thread::sleep_for(400ms);

			/*Notes: If a line exists the number for the line comes back. Iterates column by column across the row to move pieces down*/
			for (auto & v : vLines)
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}

			vLines.clear();
		}

		//Display
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, & dwBytesWritten);
	}

	// Game Over
	CloseHandle(hConsole);
	cout << "Game Over!! Score:" << nScore << endl;
	system("pause");

	return 0;
}