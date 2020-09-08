// Hrvoje Abramovic 00365606160
// IRG 8. lab vjezba
#include<iostream>
#include<glm/glm.hpp>
#include<GL/freeglut.h>
#include<string>
#include<algorithm>
#include<vector>
#include<stack>
#include<math.h>

typedef struct c {
	float r;
	float g;
	float b;
}Color;
Color bgColor = { 1,1,1 };


int WIDTH = 550;
int HEIGHT = 550;

double U_MIN = -2;
double U_MAX = 2;
double V_MIN = -2;
double V_MAX = 2;


double U_MIN_original = -2;
double U_MAX_original = 2;
double V_MIN_original = -2;
double V_MAX_original = 2;

enum drawStates {MANDELBROT, JULIA};
drawStates  DRAW_STATE;

double EPSILON = 100; 
int MAX_ITER = 35;
int delta_MAX_ITER = 10;
int delta_EPS = 20;

double zoomInFactor = 0.7;
double zoomOutFactor = 1 / zoomInFactor;

glm::vec2 juliaC(0.5, 0.2);

double calc_u(int x);
double calc_v(int y);
void loadUV();
void loadEpsIter();
void printCommands();

void reshapeFunc(int width, int height);
void displayFunc();
void keyboardDownFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

int checkConv(glm::vec2 c, glm::vec2 z0);
void drawMandelbrot();
void drawJulia(glm::vec2 c);

void setJulia();

Color calcColor(int k);

void zoomIn(int x, int y);
void zoomOut();
glm::vec2 getCenter();

std::stack<glm::vec2> oldCenters;
glm::vec2 originalCenter;



int main(int argc, char** argv) {
	std::string option;
	std::cout << "Trenutne postavke:\n(umin, umax) = (" << U_MIN << " , "<< U_MAX<<")\n";
	std::cout<<"(vmin, vmax) = (" << V_MIN << ", "<< V_MAX<<")\n";
	std::cout << "eps: " << EPSILON << " m: " << MAX_ITER;
	std::cout << "\nZelite li promijeniti postavke? y/bilo sto ";
	std::cin >> option;
	if (option == "y") {
		loadUV();
		loadEpsIter();
	}
	U_MIN_original = U_MIN;
	U_MAX_original = U_MAX;
	V_MIN_original = V_MIN;
	V_MAX_original = V_MAX;


	DRAW_STATE = MANDELBROT;
	printCommands();
	originalCenter = getCenter();
	oldCenters.push(originalCenter);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Lab 8");
	glutReshapeFunc(reshapeFunc);
	glutDisplayFunc(displayFunc);
	glutKeyboardFunc(keyboardDownFunc);
	glutMouseFunc(mouseFunc);
	//glutKeyboardUpFunc(keyboardUpFunc);
	//glutSpecialFunc(arrowsDownFunc);
	//glutSpecialUpFunc(arrowsUpFunc);
	//glutIdleFunc(idleFunc);
	glutMainLoop();
}

double calc_u(int x) {
	return U_MIN + (U_MAX - U_MIN)*(double)x / (double)(WIDTH);
}
double calc_v(int y) {
	return V_MIN + (V_MAX - V_MIN)*(double)y / (double)(HEIGHT);
}


void loadUV() {
	std::cout << "Unesite raspon za prikazati:\nu min: ";
	std::cin >> U_MIN;
	std::cout << "u max: ";
	std::cin >> U_MAX;
	std::cout << "v min: ";
	std::cin >> V_MIN;
	std::cout << "v max: ";
	std::cin >> V_MAX;
}

void loadEpsIter() {
	std::cout << "Unesite epsilon: ";
	std::cin >> EPSILON;
	std::cout << "Unesite najveci broj iteracija: ";
	std::cin >> MAX_ITER;
}

void printCommands() {
	std::cout << "UPUTE\n";

	std::cout << "! Kada se prikazuje Mandelbrotov skup klikom misa na neku lokaciju crta se Julijev skup s tom tockom !\n";
	std::cout << "U Julijevom skupu pritiskom na ESC ili m vraca se na Mandelbrotov skup\n";
	std::cout << "U Mandelbrotovom skupu pritiskom na ESC izlazi se iz programa\n";
	std::cout << "j   >> Julijev skup\n";
	std::cout << "+/- >> zoom in / zoom out\n";
	std::cout << "r   >> reset zooma\n";
	std::cout << "1   >> smanji broj iteracija za " << delta_MAX_ITER << "\n";
	std::cout << "2   >> povecaj broj iteracija za "<<delta_MAX_ITER<<"\n";
	std::cout << "3   >> smanji epsilon za " << delta_EPS << "\n";
	std::cout << "4   >> povecaj epsilon za " << delta_EPS <<"\n";
	std::cout << "c   >> rucno postavi granice, epislon i m\n"; 

	return;
}

int checkConv(glm::vec2 c, glm::vec2 z0) {

	int k = -1;
	glm::vec2 z = z0;
	double r = 0;
	bool cond = true;

	while(k < MAX_ITER && cond) {
		k += 1;
		glm::vec2 zNew;
		zNew.x = (z.x * z.x) - (z.y * z.y);
		zNew.y = 2 * z.x * z.y;
		z = zNew + c;

		//if (z.x >= EPSILON || z.y < -EPSILON || z.x * z.y >= EPSILON || std::sin((z.x*z.y)/EPSILON) > 0)cond = false;
		//if (z.x * z.y >= EPSILON)cond = false;
		r = glm::length(z);
		if (r >= EPSILON)cond = false;
	}
	return k;
}

void drawMandelbrot() {
	glBegin(GL_POINTS);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			double u = calc_u(x);
			double v = calc_v(y);
			glm::vec2 c(u, v);
			int k = checkConv(c, glm::vec2(0,0));
			Color col = calcColor(k);
			glColor3f(col.r, col.g, col.b);
			glVertex2i(x, y);
		}
	}
	glEnd();

}

void setJulia() {
	std::cout << "Unesite tocku c za Julijev skup: ";
	std::cin >> juliaC.x >> juliaC.y;
	return;
}

void drawJulia(glm::vec2 c) {
	glBegin(GL_POINTS);
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			double u = calc_u(x);
			double v = calc_v(y);
			glm::vec2 z0(u, v);
			int k = checkConv(c, z0);
			Color col = calcColor(k);
			glColor3f(col.r, col.g, col.b);
			glVertex2i(x, y);
		}
	}
	glEnd();
	return;
}


Color calcColor(int k) {
	std::string mode = "OLD";
	Color col;

	if (mode == "NEW") {
		if (k == MAX_ITER)return { 0,0,0 };

		//col.r = 0.5 * (float)k / (float)MAX_ITER;
		//col.g = 1 - (float)k / ((float)MAX_ITER*1.5);
		//col.b = 1 - (float)k / (float)MAX_ITER;
		//return col;
		col.r = std::pow(std::tan((float)k / ((float)MAX_ITER * 1.5)), 2) + 0.2;
		col.g = std::pow(std::cos(1 - 2 * (float)k / ((float)MAX_ITER)), 2) + 0.1;
		col.b = std::pow(std::cos(std::asin(std::tan((float)k / (float)MAX_ITER))), 2) + 0.1;
		col.r = std::fmin(col.r, 0.95);
		col.g = std::fmin(col.g, 0.95);
		col.b = std::fmin(col.b, 0.9);
		col.b = std::fmax(col.b, 0.1);
		col.g = std::fmax(col.g, 0.1);
	}

	if (mode == "NEW") {
		if (k == MAX_ITER)return { 0,0,0 };
		
		//col.r = 0.5 * (float)k / (float)MAX_ITER;
		//col.g = 1 - (float)k / ((float)MAX_ITER*1.5);
		//col.b = 1 - (float)k / (float)MAX_ITER;
		//return col;
		col.r = std::pow(std::tan((float)k / ((float)MAX_ITER * 1.5)), 2) + 0.2;
		col.g = std::pow(std::cos(1 - 2 * (float)k / ((float)MAX_ITER)), 2) + 0.1;
		col.b = std::pow(std::cos(std::asin(std::tan((float)k / (float)MAX_ITER))), 2) + 0.1;
		col.r = std::fmin(col.r, 0.95);
		col.g = std::fmin(col.g, 0.95);
		col.b = std::fmin(col.b, 0.9);
		col.b = std::fmax(col.b, 0.1);
		col.g = std::fmax(col.g, 0.1);
	}
	else {
		if (k == MAX_ITER)return { 0,0,0 };
		col.r = (float)k / ((float)MAX_ITER * 1.5) + 0.2;
		col.g = 1 - 2 * (float)k / ((float)MAX_ITER);
		col.b = 1 - (float)k / (float)MAX_ITER;
		col.r = std::fmin(col.r, 0.95);
		col.g = std::fmin(col.g, 0.95);
		col.b = std::fmin(col.b, 1);
		col.b = std::fmax(col.b, 0.1);
		col.g = std::fmax(col.g, 0.3);
	}
	return col;
}


glm::vec2 getCenter() {
	glm::vec2 center;
	center.x = (U_MAX + U_MIN) / 2;
	center.y = (V_MAX + V_MIN) / 2;
	return center;
}


void zoomIn(int x, int y) {
	glm::vec2 centerNow = getCenter();
	oldCenters.push(centerNow);
	glm::vec2 newCenter;
	newCenter.x = calc_u(x);
	newCenter.y = calc_v(HEIGHT - y);
	//std::cout << "y: " << newCenter.y;
	glm::vec2 delta = newCenter - centerNow;
	U_MAX *= zoomInFactor;
	U_MIN *= zoomInFactor;
	V_MAX *= zoomInFactor;
	V_MIN *= zoomInFactor;
	U_MAX += delta.x;
	U_MIN += delta.x;
	V_MAX += delta.y;
	V_MIN += delta.y;
	return;
}

void zoomOut() {

	glm::vec2 centerNow = getCenter();
	glm::vec2 newCenter = oldCenters.top();
	oldCenters.pop();
	if (oldCenters.empty()) {
		oldCenters.push(originalCenter);
	}
	glm::vec2 delta = newCenter - centerNow;
	U_MAX *= zoomOutFactor;
	U_MIN *= zoomOutFactor;
	V_MAX *= zoomOutFactor;
	V_MIN *= zoomOutFactor;
	U_MAX += delta.x;
	U_MIN += delta.x;
	V_MAX += delta.y;
	V_MIN += delta.y;
	return;

}


void reshapeFunc(int width, int height) {
	WIDTH = width;
	HEIGHT = height;
	
	glViewport(0, 0, WIDTH, HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);

	glClearColor(bgColor.r, bgColor.g, bgColor.b,1 );
	glClear(GL_COLOR_BUFFER_BIT);
	
}

void displayFunc() {

	glClearColor(bgColor.r, bgColor.g, bgColor.b, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(1);

	if (DRAW_STATE == MANDELBROT) {
		drawMandelbrot();
	}
	else {
		drawJulia(juliaC);
	}

	glutSwapBuffers();

}


void keyboardDownFunc(unsigned char key, int x, int y) {

	if (key == 27) {
		if (DRAW_STATE == MANDELBROT) {
			exit(0);
		}
		else {
			DRAW_STATE = MANDELBROT;
			glutPostRedisplay();
		}
		return;
	}

	if (key == 'j') {
		if (DRAW_STATE == MANDELBROT) {
			setJulia();
			DRAW_STATE = JULIA;
			std::cout << "Crtanje Julijevog skupa za: " << juliaC.x << " + " << juliaC.y << "i\n";
			glutPostRedisplay();
		}
		return;
	}
	
	if (key == '+') {
		zoomIn(x, y);
		glutPostRedisplay();
	}
	else if (key == '-') {
		zoomOut();
		glutPostRedisplay();
		return;
	}
	else if (key == 'r') {
		// postaviti center i velicinu na stare vrijednosti
		U_MIN = U_MIN_original;
		U_MAX= U_MAX_original;
		V_MIN = V_MIN_original;
		V_MAX= V_MAX_original;
		while (!oldCenters.empty())
			oldCenters.pop();
		oldCenters.push(originalCenter);
		glutPostRedisplay();
	}
	else if (key == '2') {
		MAX_ITER += delta_MAX_ITER;
		glutPostRedisplay();
	}
	else if (key == '1') {
		MAX_ITER -= delta_MAX_ITER;
		if (MAX_ITER <= 0) MAX_ITER = 1;
		glutPostRedisplay();
	}

	else if (key == '4') {
		EPSILON += delta_EPS;
		glutPostRedisplay();
	}
	else if (key == '3') {
		EPSILON -= delta_EPS;
		if (EPSILON <= 0) EPSILON = 5;
		glutPostRedisplay();
	}

	else if (key == 'c') {
		loadUV();
		loadEpsIter();
		U_MIN_original = U_MIN;
		U_MAX_original = U_MAX;
		V_MIN_original = V_MIN;
		V_MAX_original = V_MAX;
		glutPostRedisplay();
	}



}


void mouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && DRAW_STATE  == MANDELBROT) {
		double tempU = calc_u(x);
		double tempV = calc_v(HEIGHT - y);
		juliaC = glm::vec2(tempU, tempV);
		DRAW_STATE = JULIA;
		std::cout << "Crtanje Julijevog skupa za: " << juliaC.x << " , " << juliaC.y << "i\n";
		glutPostRedisplay();
	}
}