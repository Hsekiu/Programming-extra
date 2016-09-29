/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <list>
#include <vector>

using namespace std;


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsIshape[2][4] = 
	{{vec2(-2, 0), vec2(-1, 0), vec2(0, 0), vec2(1, 0)}, // I up
	{vec2(0, 1), vec2(0, 0), vec2(0, -1), vec2(0, -2)}}; // I down

vec2 allRotationsSshape[2][4] = 
	{{vec2(-1, -1), vec2(0, 0), vec2(0, -1), vec2(1, 0)}, // S down   
	{vec2(0, 1), vec2(0, 0), vec2(1, 0), vec2(1, -1)}};   // S up

vec2 allRotationsLshape[4][4] = 
	{{vec2(-1, -1), vec2(-1, 0), vec2(0, 0), vec2(1, 0)}, // L down
	{vec2(1, -1), vec2(0, -1), vec2(0, 0), vec2(0, 1)},   // L left
	{vec2(-1, 0), vec2(0, 0), vec2(1, 1), vec2(0, 1)},    // L up
	{vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};   // L right

enum shapes {
	Ishape,
	Sshape,
	Lshape
};

// colors
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0);
vec4 grape = vec4(0.5, 0.2, 0.9, 1.0); 
vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
vec4 banana = vec4(1.0, 0.85, 0.0, 1.0);
vec4 pear = vec4(0.15, 1, 0.1, 1.0);
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0);

vec4 allColorsTiles[7] = {grape, red, banana, pear, orange};
vec4 tilecolors[4];

// Handles gamespeed in msec
int tickrate = 300;

// Game state representation
enum GameStates { CREATE, DROP, CHECK, DONE };
// Initial gamestate and current state due to glutTimerFunc int data limitation gameState
// must be an int, but the correct convention would be that gameState would be an enum GameStates

void gameLogic(int gameState);

//X,Y cordinates representation of location of tiles
//vector<vec2> location;
vec2 location[4];

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20];

vec2 rotation;

//Has the player pressed a key for movement
bool movement = false;
//Direction for movement left -1, right +1
int direction;

//Can't create new tile since location is occupied
bool done;

//Is the tile dropping or dropped
bool notcollided;
bool collided = false;

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// update location for current tile
		location[i] = (vec2((int)x, (int)y));

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
	int randx = (rand() % 7) + 2;
	tilepos = vec2(randx , 17); // Put the tile at the top of the board

	randx = (rand() % 3); // Choose a shape to drop
	switch(randx) {
		case 0: {
				randx = (rand() % 2);
				// Update the geometry VBO of current tile
				for (int i = 0; i < 4; i++) {
					// Get the 4 pieces of the new tile

					tile[i] = allRotationsSshape[randx][i];
				}
			}
			break;
		case 1: {
				randx = (rand() % 2);
				for (int i = 0; i < 4; i++) {
					tile[i] = allRotationsIshape[randx][i]; 
				}
			}
			break;
		case 2: {
				randx = (rand() % 4);
				for (int i = 0; i < 4; i++) {
					tile[i] = allRotationsLshape[randx][i];
				}
			}
			break;
		default:
			break;
	}

	updatetile(); 

	//Check if new tile occupying the place of another tile
	for (int i = 0; i < 4; i++) {
		if (board[(int)tilepos.x][16]){
			gameLogic(DONE);
		}
	}

	if (!done) {
		// Update the color VBO of current tile
		vec4 newcolours[24];
		int count = 0;
		for (int i = 1; i <= 4; i++) {
			vec4 tilecolor = allColorsTiles[rand() % 4];
			for (int j = (count); j < 6*i; j++) {
				newcolours[j] = tilecolor;
			}
			tilecolors[i-1] = tilecolor;
			count += 6;
		}
	
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
	}
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	//newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

bool rotateable() 
{
	bool roteable = true;
	for (int i = 0; i < 4; i++) {
		GLfloat x = tilepos.y + tile[i].x * (-1); 
		GLfloat y = tilepos.x + tile[i].y;
		rotation = vec2(x, y);
		bool coll = (board[(int)location[i].x][(int)location[i].y]);
		if (rotation.x >= 0 && rotation.x <= 9 && rotation.y > 0 && !coll)
			roteable = roteable && true;
	}
	if (roteable) 
		return true;
	else
		return false;
}

// Rotates the current tile, if there is room
void rotate()
{    
	if (rotateable()) {
		for (int i = 0; i < 4; i++) {
			GLfloat x = tile[i].y * (-1);
			GLfloat y = tile[i].x;
			rotation = vec2(x, y);
			tile[i] = rotation;
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------

void clearcolumn(int start, int stop) {
	// Go through column colors
	for (int j = 0; j < 6*stop; j++) {
		boardcolours[6*(10*start) + j] = boardcolours[6*(10*start + 10) + j];
		}
	// Set board tile states
	for (int k = 0; k < stop; k++) {
		board[k][start] = board[k][start+1];
	}
}

bool isfull(int row) {
	bool full = true;
	for (int i = 0; i < 10; i++) {
		full = full && board[i][row];
	}
	if (full) {
		return true;
	}
	return false;
}

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	if (isfull(row)) {
		//cout << "row is full" << endl;
		// Go through columns
		for (int i = row; i < 19; i++) {
			clearcolumn(i, 10);
		}
	} else {
		//int count = 0;
		//for (int i row i < 19; i++) {
		//
		//}
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);

	for(int i  = 0; i < 4; i++) {
		GLuint tileX = tilepos.x + tile[i].x;
		GLuint tileY = tilepos.y + tile[i].y;
		//cout << tileX << tileY << endl;
		board[tileX][tileY] = true;
		// Set the board colors to the saved tiles colors
		for (int j = 0; j < 6; j++) {
			boardcolours[tileX*6 + tileY*60 + j] = tilecolors[i];
		}
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boardcolours), boardcolours);
	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
	for (int i = 0; i < 4; i++) {
		bool lmax = location[i].x == 0;
		bool rmax = location[i].x == 9;
		bool ymax = location[i].y == 0;
		bool coll = (board[(int)location[i].x][(int)location[i].y -1]) == true;
		bool lrow = (board[(int)location[i].x -1][(int)location[i].y]) == true;
		bool rrow = (board[(int)location[i].x +1][(int)location[i].y]) == true;

		if ((direction.x > 0 && (rmax || rrow))) {
			return false;
		}
		if ((direction.x < 0 && (lmax || lrow))) {
			return false;
		}
		if ((direction.y = -1) && (ymax || coll)) {
			return false;
		}
	}
	return true;
}
//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	initBoard();
}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP:
			//cout << "Up" << endl;
			rotate();
			break;
		case GLUT_KEY_DOWN:
			//cout << "Down" << endl;
			//making tiles move down two instead of one square creates more problems
			//it is easier just to speed up the game itself then create new collision
			//cases and issues
			//droprate = 2;
			tickrate = 100;
			break;
		case GLUT_KEY_LEFT:
			movement = true;
			direction = -1;
			break;
		case GLUT_KEY_RIGHT:
			movement = true;
			direction = 1;
			break;
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
	}
	glutPostRedisplay();
}

// Handles gamelogic
void gameLogic(int gameState) {
	updatetile();
	// Update game every half second

	if (gameState == DONE) {
		cout << "Game over!" << endl;
		restart();
	}

	if (gameState == CHECK) {
		for (int i = 0; i < 19; i++) 
			checkfullrow(i);
	}

	if (movement) {
		if (movetile(vec2(direction,0)))
				tilepos.x += direction;
		movement = false;
	}


	// Make a new tile
	if (gameState == CREATE) {
		//cout << "Creating" << endl;
		newtile();
		glutPostRedisplay();
		glutTimerFunc(tickrate, gameLogic, DROP);
	}

	// If tile created drop the tile
	if (gameState == DROP) {
		//if (tilepos.y == 1) {
		
		if (!movetile(vec2(0,-1))) {
			settile();
			tickrate = 500;
			glutTimerFunc(0, gameLogic, CHECK);
			glutTimerFunc(tickrate, gameLogic, CREATE);
		} else {
			tilepos.y -= 1;
			updatetile();
			glutTimerFunc(tickrate, gameLogic, DROP);
		}

		/*int i = 0;
		notcollided = true;
		while (notcollided && (i < 4)) {
			//if (tilepos.y == 1) {
			bool collide = (board[(int)location[i].x][(int)location[i].y -1]) == true;
			if (location[i].y == 0 || collide) {
				cout << "collided" << endl;
				settile();
				//Tile is dropped reset rate
				droprate = 1;
				notcollided = false;
				glutTimerFunc(tickrate, gameLogic, CREATE);
			}
			i++;
		}

		if (notcollided) {
			tilepos.y -= droprate;
			updatetile();
			glutTimerFunc(tickrate, gameLogic, DROP);
		}*/
	}
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);

	// Start of initial game loop
	glutTimerFunc(0, gameLogic, CREATE);

	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}