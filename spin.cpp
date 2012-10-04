#include <GL/glfw.h>
#include <iostream>
#include <cstdio>
#include <cmath>
#include <cassert>
#include <vector>
#include <ctime>    // For time()
#include <cstdlib>  // For srand() and rand()


const GLuint
	LEFT_COLOR = 0xFFFF0000,
	LEFTCENTER_COLOR = 0xFFFFFF00,
	RIGHTCENTER_COLOR = 0xFF00FFFF,
	RIGHT_COLOR = 0xFF0000FF;

using namespace std;

enum { ATTRIB_POS };

struct Vtx {
	GLfloat x, y, z;
//	GLuint color;
};

GLuint mainProgram = 0;
const size_t nVertices = 128;

Vtx spawnVtx[nVertices*2];
Vtx vertices[nVertices];
Vtx cannonVtx[nVertices/2];
float cannonAgi = 0.03;

float shots[100][6];
int currentShot = 0;
int fullShots = 0; //boolean
Vtx currentShotVtx[6];
int shotInterval = 0;

float beasts[20][6];
int currentBeast = 0;
int fullBeasts = 0; //boolean
Vtx currentBeastVtx[5];

const GLfloat PIE = 3.14159265f;
void generatePolygon(GLfloat radius, GLfloat angularOffset, GLfloat x, GLfloat y, GLvoid *pointer, GLsizei nVertices, GLuint stride) {
	assert(nVertices >= 3);
	GLfloat *vtx = (GLfloat*)pointer;
	vtx[0] = x;
	vtx[1] = y;
	
	const GLfloat n = nVertices - 2;
	const GLfloat factor = 2.0f * PIE/n;
	for ( GLsizei i = 1; i <= n + 1; ++i ) {
		GLfloat theta = i * factor;
		
		GLfloat *vtx = (GLfloat*)(((GLubyte*)pointer) + stride * i);
		vtx[0] = radius * std::cos(angularOffset + theta) + x;
		vtx[1] = radius * std::sin(angularOffset + theta) + y;
	}
}

bool loadShaderSource(GLuint shader, const char *path) {
	FILE *f = fopen(path, "r");
	if ( !f ) {
		std::cerr << "ERROR: shader source not found: " << path << '\n';
		return false;
	}
	fseek(f, 0, SEEK_END);
	vector<char> sauce(ftell(f) + 1);
	fseek(f, 0, SEEK_SET);
	fread(&sauce[0], 1, sauce.size(), f);
	fclose(f);
	const GLchar *ptr = &sauce[0];
	glShaderSource(shader, 1, &ptr, 0);
	if ( glGetError() != GL_NO_ERROR ) {
		std::cerr << "ERROR: Unable to load shader\n";
		return false;
	}
	return true;
}

void checkShaderStatus(GLuint shader) {
	GLint logLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		GLchar *log = (GLchar *)malloc(logLength);
		glGetShaderInfoLog(shader, logLength, &logLength, log);
		std::cout << "Shader compile log:\n" << log << endl;
		free(log);
	}
}

bool initShader() {
	GLuint vtx = glCreateShader(GL_VERTEX_SHADER),
	       frag = glCreateShader(GL_FRAGMENT_SHADER);
	
	if ( !loadShaderSource(vtx, "circle.vsh") ) {
		std::cerr << "Error in compiling the vertex shader\n";
		glDeleteShader(vtx);
		glDeleteShader(frag);
		return false;
	}
	if ( !loadShaderSource(frag, "circle.fsh") ) {
		std::cerr << "Error in compiling the fragment shader\n";
		glDeleteShader(vtx);
		glDeleteShader(frag);
		return false;
	}
	
	glCompileShader(vtx);
	checkShaderStatus(vtx);

	glCompileShader(frag);
	checkShaderStatus(frag);

	mainProgram = glCreateProgram();
	glAttachShader(mainProgram, vtx);
	glAttachShader(mainProgram, frag);

	glBindAttribLocation(mainProgram, ATTRIB_POS, "position");

	glLinkProgram(mainProgram);

	GLint logLength;
	glGetProgramiv(mainProgram, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0) {
		GLchar *log = (GLchar *)malloc(logLength);
		glGetProgramInfoLog(mainProgram, logLength, &logLength, log);
		std::cout << "Program link log:\n" << log << endl;
		free(log);
	}

	glDeleteShader(vtx);
	glDeleteShader(frag);

	return true;
}

void initSpawn() {
	generatePolygon(sqrt(2), 0, 0, 0, spawnVtx, nVertices*2, sizeof(Vtx));
	spawnVtx[0].z = 0;
	for ( size_t i = 1; i < nVertices*2-1; ++i ) {
		spawnVtx[i].z = (sin((double)i/(nVertices*2-1.0) * 7 * PIE) + 1.0)/2.0;
	}
	spawnVtx[nVertices*2-1].z = spawnVtx[1].z;
	
	glEnableVertexAttribArray(ATTRIB_POS);

	glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), spawnVtx);
}

void initCircle(GLfloat x, GLfloat y) {
	generatePolygon(0.1, 0, x, y, vertices, nVertices, sizeof(Vtx));
	vertices[0].z = 0;
	for ( size_t i = 1; i < nVertices-1; ++i ) {
		vertices[i].z = (sin((double)i/(nVertices-1.0) * 7 * PIE) + 1.0)/2.0;
	}
	vertices[nVertices-1].z = vertices[1].z;
	
	glEnableVertexAttribArray(ATTRIB_POS);

	glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), vertices);
}

void initCannon(GLfloat x, GLfloat y) {
	generatePolygon(0.05, 0, x, y, cannonVtx, nVertices/2, sizeof(Vtx));
	cannonVtx[0].z = 0;
	for ( size_t i = 1; i < nVertices-1; ++i ) {
		cannonVtx[i].z = (sin((double)i/((nVertices/2)-1.0) * 7 * PIE) + 1.0)/2.0;
	}
	cannonVtx[nVertices/2-1].z = cannonVtx[1].z;
	
	glEnableVertexAttribArray(ATTRIB_POS);

	glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), cannonVtx);
}

void initShot(GLfloat x, GLfloat y, GLfloat offset) {
	generatePolygon(0.03, 0.9*offset , x, y, currentShotVtx, 6, sizeof(Vtx));
	currentShotVtx[0].z = 0;
	for ( size_t i = 1; i < 6-1; ++i ) {
		currentShotVtx[i].z = (sin((double)i/((6)-1.0) * 7 * PIE) + 1.0)/2.0;
	}
	currentShotVtx[6-1].z = currentShotVtx[1].z;
	
	glEnableVertexAttribArray(ATTRIB_POS);

	glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), currentShotVtx);
}

void initBeast(GLfloat x, GLfloat y, GLfloat offset) {
	generatePolygon(0.07, 0.1*offset, x, y, currentBeastVtx, 5, sizeof(Vtx));
	//generatePolygon(0.045, 0.5*offset, 0, 0.5, currentBeastVtx, 5, sizeof(Vtx));
	currentBeastVtx[0].z = 0;
	for ( size_t i = 1; i < 5-1; ++i ) {
		currentBeastVtx[i].z = (sin((double)i/((5)-1.0) * 7 * PIE) + 1.0)/2.0;
	}
	currentBeastVtx[5-1].z = currentBeastVtx[1].z;
	
	glEnableVertexAttribArray(ATTRIB_POS);

	glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), currentBeastVtx);
}

void nextInLine(int pos){
	shots[pos][0] += shots[pos][3];
	shots[pos][1] += shots[pos][4];
	shots[pos][5] += 1;
	//cout << shots[pos][0] << ", " << shots[pos][1] << " are the next coordinates\n";
}

void nextEnemy(int pos){
	beasts[pos][0] += beasts[pos][3];
	beasts[pos][1] += beasts[pos][4];
	beasts[pos][5] += 1;
}

/*void initColor(){
	for (int x = 0 ; x < sizeof(vertices) ; x++){
		vertices[x].color = 0xFFFFFFFF;
	}
	for (int y = 0 ; y < sizeof(cannonVtx) ; y++){
		cannonVtx[y].color = 0xFFFF0000;
	}
}*/

int main() {
	if ( !glfwInit() ) {
		std::cerr << "Unable to initialize OpenGL!\n";
		return -1;
	}
	
	if ( !glfwOpenWindow(640,640, //width and height of the screen
				8,8,8,8, //Red, Green, Blue and Alpha bits
				0,0, //Depth and Stencil bits
				GLFW_WINDOW)) {
		std::cerr << "Unable to create OpenGL window.\n";
		glfwTerminate();
		return -1;
	}

	glfwSetWindowTitle("Zax");

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );

	// Enable vertical sync (on cards that support it)
	glfwSwapInterval( 1 );

	glClearColor(0,0,0,0);

	if ( !initShader() ) {
		return -1;
	}

	glUseProgram(mainProgram);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//initColor();
	initSpawn();

	float circleX = 0;
	float circleY = 0;
	int cannonPnt = 32;
	int timeBeast = 20;
	beasts[0][0] = 0.0; beasts[0][1] = 0.5; beasts[0][5] = 0;

	do {
		srand(time(0));
		int width, height;
		// Get window size (may be different than the requested size)
		//we do this every frame to accommodate window resizing.
		glfwGetWindowSize( &width, &height );
		glViewport( 0, 0, width, height );

		glClear(GL_COLOR_BUFFER_BIT);

		initCircle(circleX, circleY);
		glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices);

		initCannon(vertices[cannonPnt].x, vertices[cannonPnt].y);
		glDrawArrays(GL_TRIANGLE_FAN, 0, nVertices/2);

		int ii = 0;
		if (!fullShots){
			ii = currentShot;
		}
		else{
			ii = 100;
		}
		for (int i = 0 ; i < ii ; i++){
			initShot(shots[i][0], shots[i][1], shots[i][5]);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
			nextInLine(i);
		}


		if (!fullBeasts){
			ii = currentBeast;
		}
		else{
			ii = 100;
		}
		for (int i = 0 ; i < ii ; i++){
			initBeast(beasts[i][0], beasts[i][1], beasts[i][5]);
			//initBeast(beasts[0][0], beasts[0][1], beasts[0][5]);
			//initBeast(0, 0, 0);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 5);
			nextEnemy(i);
			//beasts[0][5] += 2;
		}

		if (glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey('W') == GLFW_PRESS){
			circleY += cannonAgi;
		}
		if (glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey('D') == GLFW_PRESS){
			circleX += cannonAgi;
		}
		if (glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey('S') == GLFW_PRESS){
			circleY -= cannonAgi;
		}
		if (glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey('A') == GLFW_PRESS){
			circleX -= cannonAgi;
		}

		if (glfwGetKey('E') == GLFW_PRESS || glfwGetKey('P') == GLFW_PRESS){
			if (cannonPnt <= 4){
				cannonPnt = nVertices-1;
			}
			else{
				cannonPnt -= 3;
			}
		}
		if (glfwGetKey('Q') == GLFW_PRESS || glfwGetKey('I') == GLFW_PRESS){
			if (cannonPnt >= nVertices-2){
				cannonPnt = 2;
			}
			else{
				cannonPnt += 3;
			}
		}

		shotInterval++;
		timeBeast++;
		if ((glfwGetKey(GLFW_KEY_SPACE) == GLFW_PRESS || glfwGetKey('O') == GLFW_PRESS) && shotInterval > 15){
			shotInterval = 0;
			shots[currentShot][0] = circleX;
			shots[currentShot][1] = circleY;
			shots[currentShot][2] = 0;
			shots[currentShot][3] = vertices[cannonPnt].x - circleX;
			shots[currentShot][4] = vertices[cannonPnt].y - circleY;
			shots[currentShot][5] = 0;

			/*cout << vertices[cannonPnt].x << ", " << vertices[cannonPnt].y << " are the supposed coordinates\n";
			cout << shots[currentShot][0] << ", " << shots[currentShot][1] << " are the actual coordinates\n";*/ 

			if (currentShot == 99){
				currentShot = 0;
				fullShots = 1;
			}
			else{
				currentShot++;
			}
		}
		if (timeBeast > 70){
			timeBeast = 0;
			int pos = (rand() % (nVertices*2));
			
			//int pos = 30;
			beasts[currentBeast][0] = spawnVtx[pos].x;
			beasts[currentBeast][1] = spawnVtx[pos].y;
			//beasts[currentBeast][0] = vertices[pos].x;
			//beasts[currentBeast][1] = vertices[pos].y;
			beasts[currentBeast][2] = 0;
			beasts[currentBeast][3] = (-1*spawnVtx[pos].x)/150;
			beasts[currentBeast][4] = (-1*spawnVtx[pos].y)/150;
			//beasts[currentBeast][3] = (-1*vertices[pos].x)/10;
			//beasts[currentBeast][4] = (-1*vertices[pos].y)/10;
			beasts[currentBeast][5] = 0;

			//cout << vertices[cannonPnt].x << ", " << vertices[cannonPnt].y << " are the supposed coordinates\n";
			//cout << shots[currentShot][0] << ", " << shots[currentShot][1] << " are the actual coordinates\n";

			if (currentBeast == 99){
				currentBeast = 0;
				fullBeasts = 1;
			}
			else{
				currentBeast++;
			}
		}

		//VERY IMPORTANT: displays the buffer to the screen
		glfwSwapBuffers();
	} while ( glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS &&
			glfwGetWindowParam(GLFW_OPENED) );

	glDeleteProgram(mainProgram);
	glfwTerminate();
	return 0;
}
