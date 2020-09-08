// Hrvoje Abramovic 00365606160
// IRG 6. lab vjezba
#include<iostream>
#include<glm/glm.hpp>
#include<GL/freeglut.h>
#include<fstream>
#include<sstream>
#include<string>
#include<algorithm>
#include<vector>
#include<math.h>
#include<windows.h>
#define INF 1000000000

int WIDTH = 700;
int HEIGHT = 700;


int V_N = 0;
int F_N = 0;

typedef struct pf {
	double A;
	double B;
	double C;
	double D;
}PlainCoefs;

typedef struct c {
	float r;
	float g;
	float b;
}Color;

enum states { POLYGON, OBJECT };
states STATE;

glm::vec4* objV;
glm::vec4* objV_PROJ;
int** objF;
PlainCoefs* plains;

double xMin = INF;
double xMax = -INF;
double yMin = INF;
double yMax = -INF;
double zMin = INF;
double zMax = -INF;

double MaxRange;

Color BgColor = { 1, 1, 1 };
Color LineColor = { 0, 0, 0 };
Color PointColor = { 1,0,0 };


glm::vec3 Point;
glm::vec4 Center;
std::string fileName;
std::string polyFile;

glm::vec4 O;
glm::vec4 G(0,0,0,1);
glm::vec4 originalO;

glm::mat4 viewMat;
glm::mat4 projMat;

double tAnim = 0;
double deltaT = 0.00025;
int tDirection = 1;

int nBezier;
double* factorials;

glm::vec4* bezierPoints;
double deltaO = 0.1;
double deltaG = 0.01;

glm::vec4 O_dir(0, 0, 0, 0);
glm::vec4 G_dir(0, 0, 0, 0);

std::vector<glm::vec3>Points;

void loadPoly();
void scanVF();
void loadObj();
void loadDot();
void computeCenter();
void computeMaxRange();
void translateObj(glm::vec3 pos);
void scaleObj();
void getFaceCoefs();
void checkDot();
void computeExtremes();
void idleFunc();
void posAndScaleObj();
void computeViewMat();
void computeProjMat();
void objectProjection();
void projectionFunctions();
double bezierCoef(int i, double t);

void printCommands();
void animateBezier();

void keyboardDownFunc(unsigned char key, int x, int y);
void keyboardUpFunc(unsigned char key, int x, int y);

void arrowsDownFunc(int key, int x, int y);
void arrowsUpFunc(int key, int x, int y);

void reshapeFunc(int width, int height);
void displayFunc();



int main(int argc, char** argv) {
	// SVI RACUNI
	STATE = POLYGON;

	std::cout << "Unesite ime dat. gdje su podaci o poligonu koji odredjuje Bezierovu krivulju: ";
	std::cin >> polyFile;
	loadPoly();
	std::cout << "Unesite koordinate ocista: ";
	std::cin >> originalO.x >> originalO.y >> originalO.z;
	originalO.w = 1;
	O = originalO;

	// transformacije nad putanjom beziera i postavi u ishodiste
	posAndScaleObj();

	// izracun pogleda
	computeViewMat();
	computeProjMat();
	objectProjection();

	printCommands();
	std::cout << "PRITISKOM NA * SPACEBAR *  TIPKU KRECE UCITAVANJE OBJEKTA I RAD SA OBJEKTOM\n";

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Lab 6");
	glutReshapeFunc(reshapeFunc);
	glutDisplayFunc(displayFunc);
	glutKeyboardFunc(keyboardDownFunc);
	glutKeyboardUpFunc(keyboardUpFunc);
	glutSpecialFunc(arrowsDownFunc);
	glutSpecialUpFunc(arrowsUpFunc);
	glutIdleFunc(idleFunc);
	glutMainLoop();

	return 0;

}

// ucitavanje podataka o krivulji
// ocekuje file strukturiran:
//prvi red n (dakle ucitati ce n+1 tocku)
// slj n+1 redova: koordinate tocke poligona
void loadPoly() {
	std::ifstream cpFile;
	cpFile.open(polyFile);


	V_N = 0;

	cpFile >> nBezier;
	V_N = nBezier + 1;

	objV = new glm::vec4[V_N];
	objV_PROJ = new glm::vec4[V_N];
	bezierPoints = new glm::vec4[V_N];
	factorials = new double[V_N];
	factorials[0] = 1;

	for (int i = 1; i < V_N; i++)
	{
		factorials[i] = factorials[i - 1] * i;
	}

	double xTemp, yTemp, zTemp;

	for (int i = 0; i < V_N; i++) {
		cpFile >> objV[i].x >> objV[i].y >> objV[i].z;
		objV[i].w = 1;
		bezierPoints[i] = objV[i];
	}

	return;
};

double bezierCoef(int i, double t) {
	double b = factorials[nBezier] / (factorials[i] * factorials[nBezier - i]);
	b = b * glm::pow(t, i) * glm::pow((1 - t), (nBezier - i));
	return b;
}

void posAndScaleObj() {
	computeExtremes();
	computeCenter();
	computeMaxRange();
	translateObj({ 0,0,0 });
	scaleObj();
}


void startObjectState() {
	delete[] objV;
	delete[] objV_PROJ;

	std::cout << "Unesite ime objekta: ";
	std::cin >> fileName;

	G = glm::vec4(0, 0, 0, 1);

	scanVF();
	loadObj();
	posAndScaleObj();
	objectProjection();

	STATE = OBJECT;
	glutPostRedisplay();
	//animateBezier();
}

void computeViewMat() {

	glm::mat4 T1(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-O.x, -O.y, -O.z, 1);
	T1 = glm::transpose(T1);

	glm::vec4 G1 = G * T1;

	double sinA = G1.y / glm::sqrt(glm::pow(G1.x, 2) + glm::pow(G1.y, 2));
	double cosA = G1.x / glm::sqrt(glm::pow(G1.x, 2) + glm::pow(G1.y, 2));

	glm::mat4 T2(cosA, -sinA, 0, 0,
		sinA, cosA, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	T2 = glm::transpose(T2);

	glm::vec4 G2 = G1 * T2;

	double sinB = G2.x / glm::sqrt(glm::pow(G2.x, 2) + glm::pow(G2.z, 2));
	double cosB = G2.z / glm::sqrt(glm::pow(G2.x, 2) + glm::pow(G2.z, 2));

	glm::mat4 T3(cosB, 0, sinB, 0,
		0, 1, 0, 0,
		-sinB, 0, cosB, 0,
		0, 0, 0, 1);
	T3 = glm::transpose(T3);

	glm::vec4 G3 = G2 * T3;

	glm::mat4 T4(
		0, -1, 0, 0,
		1, 0, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);

	T4 = glm::transpose(T4);

	glm::mat4 T5(
		-1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);

	T5 = glm::transpose(T5);

	viewMat = T1 * T2 * T3 * T4 * T5;

	return;
}

void computeProjMat() {
	double H = glm::sqrt(
		glm::pow((O.x - G.x), 2) +
		glm::pow((O.y - G.y), 2) +
		glm::pow((O.z - G.z), 2));

	projMat = glm::mat4(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 0, 1 / H,
		0, 0, 0, 0);

	projMat = glm::transpose(projMat);
	return;
}

void objectProjection() {

	for (int i = 0; i < V_N; i++) {
		glm::vec4 tempVec = objV[i];
		tempVec = tempVec * viewMat * projMat;
		tempVec = tempVec / tempVec.w;
		objV_PROJ[i] = tempVec;
	}

}

void projectionFunctions() {
	computeViewMat();
	computeProjMat();
	objectProjection();
}


// prvi prolaz kroz file i dobivanje broja Vertexa i Faceova, alocira prostor
void scanVF() {
	std::ifstream objFile;
	objFile.open(fileName);

	std::string line;

	V_N = 0;
	F_N = 0;

	while (std::getline(objFile, line)) {
		if (line[0] == 'v')V_N++;
		else if (line[0] == 'f')F_N++;
	}

	objFile.close();

	objV = new glm::vec4[V_N];
	objV_PROJ = new glm::vec4[V_N];
	objF = new int* [F_N];
	plains = new PlainCoefs[F_N];

	for (int i = 0; i < F_N; i++)
		objF[i] = new int[3];

	return;
}

// ucitavanje cijelog objekta
void loadObj() {
	// dobiti i xmin, xmax, ymin, ymax, zmin, zmax
	std::ifstream objFile;
	objFile.open(fileName);

	std::string fileLine;
	int vi = 0;
	int fi = 0;

	while (std::getline(objFile, fileLine)) {
		if (fileLine[0] != 'v' && fileLine[0] != 'f')continue;

		char tempType;
		if (fileLine[0] == 'v') {
			double xTemp, yTemp, zTemp;
			std::istringstream stream(fileLine);
			stream >> tempType >> xTemp >> yTemp >> zTemp;
			objV[vi] = glm::vec4(xTemp, yTemp, zTemp, 1);
			vi++;
		}
		else if (fileLine[0] == 'f') {
			int v1, v2, v3;
			std::istringstream stream(fileLine);
			stream >> tempType >> v1 >> v2 >> v3;
			objF[fi][0] = v1 - 1;
			objF[fi][1] = v2 - 1;
			objF[fi][2] = v3 - 1;
			fi++;
		}
	}
	objFile.close();
	return;
}

// izracun maksimalnih i minimalnih koordinata
void computeExtremes() {

	xMin = INF;
	xMax = -INF;
	yMin = INF;
	yMax = -INF;
	zMin = INF;
	zMax = -INF;

	for (int i = 0; i < V_N; i++)
	{

		if (objV[i].x > xMax) xMax = objV[i].x;
		if (objV[i].x < xMin) xMin = objV[i].x;
		if (objV[i].y > yMax) yMax = objV[i].y;
		if (objV[i].y < yMin) yMin = objV[i].y;
		if (objV[i].z > zMax) zMax = objV[i].z;
		if (objV[i].z < zMin) zMin = objV[i].z;
	}
	return;
}

// izracun sredista tijela
void computeCenter() {

	Center.x = (xMin + xMax) / 2.0;
	Center.y = (yMin + yMax) / 2.0;
	Center.z = (zMin + zMax) / 2.0;
	Center.w = 1;
	return;
}

// izracun najveceg raspona
void computeMaxRange() {
	MaxRange = std::max(xMax - xMin, std::max(yMax - yMin, zMax - zMin));
	return;
}

// translacija na poziciju
void translateObj(glm::vec3 pos) {
	double dx, dy, dz;
	dx = pos.x - Center.x;
	dy = pos.y - Center.y;
	dz = pos.z - Center.z;

	glm::mat4 translateMatrix(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		dx, dy, dz, 1);
	translateMatrix = glm::transpose(translateMatrix); //jer je column-wise

	for (int i = 0; i < V_N; i++) {
		objV[i] = objV[i] * translateMatrix;
	}

	return;
}

// skaliranje na [-1, 1]
void scaleObj() {

	double sF = 2.0 / MaxRange; // scaleFactor


	glm::mat4 scaleMatrix(sF, 0, 0, 0,
		0, sF, 0, 0,
		0, 0, sF, 0,
		0, 0, 0, 1);

	scaleMatrix = glm::transpose(scaleMatrix);

	for (int i = 0; i < V_N; i++) {
		objV[i] = objV[i] * scaleMatrix;
	}

	return;
}

// izracun koeficijenata ravnina poligona
void getFaceCoefs() {
	//podzadatak 2
	for (int i = 0; i < F_N; i++) {
		double x1, x2, x3;
		double y1, y2, y3;
		double z1, z2, z3;

		x1 = objV[objF[i][0]].x;
		x2 = objV[objF[i][1]].x;
		x3 = objV[objF[i][2]].x;

		y1 = objV[objF[i][0]].y;
		y2 = objV[objF[i][1]].y;
		y3 = objV[objF[i][2]].y;

		z1 = objV[objF[i][0]].z;
		z2 = objV[objF[i][1]].z;
		z3 = objV[objF[i][2]].z;

		plains[i].A = (y2 - y1) * (z3 - z1) - (z2 - z1) * (y3 - y1);
		plains[i].B = -(x2 - x1) * (z3 - z1) + (z2 - z1) * (x3 - x1);
		plains[i].C = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1);
		plains[i].D = -x1 * plains[i].A - y1 * plains[i].B - z1 * plains[i].C;
	}
	return;
}

// unos tocke
void loadDot() {

	std::cout << "Unesite koordinate tocke" << "\n";

	std::cout << "x: ";
	std::cin >> Point.x;
	std::cout << "y: ";
	std::cin >> Point.y;
	std::cout << "z: ";
	std::cin >> Point.z;

	return;
}

// provjera je li tocka  u konveksnom objektu
void checkDot() {
	bool inside = true;
	bool onObject = false;

	for (int i = 0; i < F_N; i++) {
		double eq = plains[i].A * Point.x +
			plains[i].B * Point.y +
			plains[i].C * Point.z +
			plains[i].D;

		if (eq > 0) {
			inside = false;
		}
		else if (eq == 0) {
			onObject = true;
		}
	}

	if (onObject && inside)
		std::cout << "Tocka je na objektu\n";
	else if (inside)
		std::cout << "Tocka je unutar objekta\n";
	else
		std::cout << "Tocka je izvan objekta\n";

	return;
}

// funkcija koja se poziva pri promjeni velicine prozora
void reshapeFunc(int width, int height) {
	WIDTH = width;
	HEIGHT = height;

	glViewport(0, 0, (GLsizei)WIDTH, (GLsizei)HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-3, 3, -3, 3);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	glClearColor(BgColor.r, BgColor.g, BgColor.b, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glPointSize(3);

	return;
}

// funkcija za prikaz
void displayFunc() {

	glClearColor(BgColor.r, BgColor.g, BgColor.b, 1);
	glClear(GL_COLOR_BUFFER_BIT);



	glColor3f(LineColor.r, LineColor.g, LineColor.b);

	switch (STATE) {
	case POLYGON:
		// crtanje "poligona"
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < V_N; i++) {
			glVertex3f(objV_PROJ[i].x, objV_PROJ[i].y, objV_PROJ[i].z);
			//glVertex2f(objV_PROJ[i].x, objV_PROJ[i].y);
		}
		glEnd();

		// crtanje krivulje
		// ides t od 0 do 1 - za svaki t dobis tocku, kada odredis tu tocku na temelju objV pomnozis tu tocku s onim matricama za projekciju i to onda nacrtas
		glColor3f(PointColor.r, PointColor.g, PointColor.b);
		glBegin(GL_POINTS);
		for (double t = 0; t < 1; t += 0.01) {
			glm::vec4 pt(0, 0, 0, 1);
			for (int i = 0; i <= nBezier; i++) {
				double b = bezierCoef(i, t);
				pt = pt + glm::vec4(b,b,b,0) * objV[i];
			}
			pt = pt * viewMat * projMat;
			pt = pt / pt.w;
			glVertex2f(pt.x, pt.y);
		}
		glEnd();


		break;
	case OBJECT:
		for (int i = 0; i < F_N; i++) {
			glm::vec3 n1 = objV[objF[i][1]] - objV[objF[i][0]];
			glm::vec3 n2 = objV[objF[i][2]] - objV[objF[i][0]];
			glm::vec3 N = glm::cross(n1, n2);
			glm::vec3 tempO = O;
			if (glm::dot(N, tempO) <= 0)continue;
			glBegin(GL_LINE_LOOP);
			glVertex2f(objV_PROJ[objF[i][0]].x, objV_PROJ[objF[i][0]].y);
			glVertex2f(objV_PROJ[objF[i][1]].x, objV_PROJ[objF[i][1]].y);
			glVertex2f(objV_PROJ[objF[i][2]].x, objV_PROJ[objF[i][2]].y);
			glEnd();
		}
		//glFrontFace(GL_CCW);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		break;

	}


	glutSwapBuffers();
	return;
}


void animateBezier() {
	O = bezierPoints[0];
	G = glm::vec4(0, 0, 0, 1);

	for (double t = 0; t < 1; t+=0.01)
	{
		glm::vec4 pt(0, 0, 0, 1);
		for (int i = 0; i <= nBezier; i++) {
			double b = bezierCoef(i, t);
			pt = pt + glm::vec4(b, b, b, 0) * bezierPoints[i];
		}
		O = pt;
		projectionFunctions();
		glutPostRedisplay();
	}
	std::cout << "GOTOV SAN";
	return;
}

void keyboardDownFunc(unsigned char key, int x, int y) {
	if (STATE == OBJECT)
		return;
	if (key == 32 && STATE == POLYGON) {
		startObjectState();
		return;
	}

	switch (key) {
	case 'r':
		G = glm::vec4(0, 0, 0, 1);
		break;
	case 'a':
		O_dir.y = 1;
		break;
	case 's':
		O_dir.y = -1;
		break;
	case '1':
		G_dir.x = 1;
		break;
	case '2':
		G_dir.x = -1;
		break;
	case '3':
		G_dir.y = -1;
		break;
	case '4':
		G_dir.y = 1;
		break;
	case '5':
		G_dir.z = -1;
		break;
	case '6':
		G_dir.z = 1;
		break;
	default:
		break;
	}
}

void keyboardUpFunc(unsigned char key, int x, int y) {
	if (STATE == OBJECT)
		return;
	switch (key) {

	case 'a':
	case 's':
		O_dir.y = 0;
		break;
	case '1':
	case '2':
		G_dir.x = 0;
		break;
	case '3':
	case '4':
		G_dir.y = 0;
		break;
	case '5':
	case '6':
		G_dir.z = 0;
		break;
	default:
		break;
	}

}


// funkcija koja se poziva pri pritisku strelica
void arrowsDownFunc(int key, int x, int y) {
	if (STATE == OBJECT)
		return;
	if (key == GLUT_KEY_UP)
		O_dir.z = 1;
	if (key == GLUT_KEY_DOWN)
		O_dir.z = -1;
	if (key == GLUT_KEY_LEFT)
		O_dir.x = -1;
	if (key == GLUT_KEY_RIGHT)
		O_dir.x = 1;

	return;
}

// funkcija koja se poziva pri otpustanju strelica
void arrowsUpFunc(int key, int x, int y) {
	if (STATE == OBJECT)
		return;
	if (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN)
		O_dir.z = 0;
	if (key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT)
		O_dir.x = 0;

	return;
}

//  funckija koja se konstantno vrti u loopu
void idleFunc() {
	//if (O_dir.x == 0 && O_dir.y == 0 && O_dir.z == 0 && G_dir.x == 0 && G_dir.y == 0 && G_dir.z == 0)return;
	if (STATE == OBJECT) {
		if (tAnim >= 1) { tDirection = -1; }
		if (tAnim <= 0) { tDirection = 1;  }
		tAnim += deltaT*tDirection;

		glm::vec4 pt(0, 0, 0, 1);
		for (int i = 0; i <= nBezier; i++) {
			double b = bezierCoef(i, tAnim);
			pt = pt + glm::vec4(b, b, b, 0) * bezierPoints[i];
		}
		O = pt;
		projectionFunctions();
		glutPostRedisplay();
		
		return;
	}
	O = O + O_dir * glm::vec4(deltaO, deltaO, deltaO, 0);
	G = G + G_dir * glm::vec4(deltaG, deltaG, deltaG, 0);
	projectionFunctions();

	glutPostRedisplay();
}


void printCommands() {

	std::cout << "Upute za pomicanje:\nOciste:\n  - x os: LEFT ARROW / RIGHT ARROW\n  - y os: a/s\n  - z os: UP ARROW/DOWN ARROW\nGlediste:\n  - x os: 1/2\n  - y os: 3/4\n  - z os: 5/6\nr je reset gledista (0,0,0)\n";


}
