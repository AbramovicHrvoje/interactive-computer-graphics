// Hrvoje Abramovic 00365606160
// IRG 7. lab vjezba
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

glm::vec4* objV;
glm::vec4* objV_PROJ;
int** objF;
PlainCoefs* plains;

glm::vec3* faceNormals;
glm::vec3* vertexNormals;

glm::vec3* faceInt;
glm::vec3* vertexInt;

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

glm::vec3 objColor(0.4, 0.5, 1);
GLfloat lightInt = 1;
GLfloat hiddenCoef = 0.05;
glm::vec3 lightColor(1, 1, 1);

enum projStates { MANUAL, OPENGL };
projStates PROJ_TYPE;

enum shadeModes { CONSTANT, GOUARD };
shadeModes SHADE_MODE;

enum programStates {WIREFRAME, LIGHT};
programStates PROG_STATE;

glm::vec4 Center;
std::string fileName;
std::string polyFile;

glm::vec4 O;
glm::vec4 G(0,0,0,1);
glm::vec4 standardG(0, 0, 0, 1);
glm::vec4 originalO;

glm::vec4 pointLight;
glm::vec3 pointLightNorm;

glm::mat4 viewMat;
glm::mat4 projMat;

bool UNI_COLOR = true;


double deltaO = 0.1;
double deltaG = 0.01;

glm::vec4 O_dir(0, 0, 0, 0);
glm::vec4 G_dir(0, 0, 0, 0);


void scanVF();
void loadObj();
void computeCenter();
void computeMaxRange();
void translateObj(glm::vec3 pos);
void scaleObj();
void computeExtremes();
void idleFunc();
void posAndScaleObj();
void computeViewMat();
void computeProjMat();
void objectProjection();
void projectionFunctions();

void pointLightInput();
void computeNormals();

void printCommands();

void keyboardDownFunc(unsigned char key, int x, int y);
void keyboardUpFunc(unsigned char key, int x, int y);

void arrowsDownFunc(int key, int x, int y);
void arrowsUpFunc(int key, int x, int y);

void reshapeFunc(int width, int height);
void displayFunc();

void startLightState();
void flipShadeType();
void flipProjType();

void computeLightInt();


int main(int argc, char** argv) {
	// SVI RACUNI
	PROJ_TYPE = MANUAL;
	PROG_STATE = WIREFRAME;
	std::cout << "Unesite ime objekta: ";
	std::cin >> fileName;

	scanVF();
	loadObj();
	posAndScaleObj();
	computeNormals();
	
	std::cout << "Unesite koordinate ocista: ";
	std::cin >> originalO.x >> originalO.y >> originalO.z;
	originalO.w = 1;
	O = originalO;
	G = glm::vec4(0, 0, 0, 1);

	projectionFunctions();


	printCommands();
	std::cout << "PRITISKOM NA * SPACEBAR *  TIPKU KRECE UCITAVANJE IZVORA SVJETLA\n";

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Lab 7");
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


void posAndScaleObj() {
	computeExtremes();
	computeCenter();
	computeMaxRange();
	translateObj({ 0,0,0 });
	scaleObj();
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
	vertexNormals = new glm::vec3[V_N];
	vertexInt = new glm::vec3[V_N];

	objF = new int* [F_N];
	plains = new PlainCoefs[F_N];
	faceNormals = new glm::vec3[F_N];
	faceInt = new glm::vec3[F_N];

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

// izracun svih normala  -- i poligona i vrhova
void computeNormals() {

	glm::vec3* normalSums = new glm::vec3[V_N];
	int* vN = new int[V_N];

	for (int i = 0; i < V_N; i++) {
		normalSums[i] = glm::vec3(0, 0, 0);
		vN[i] = 0;
	}


	for (int i = 0; i < F_N; i++) {

		glm::vec3 n1 = objV[objF[i][1]] - objV[objF[i][0]];
		glm::vec3 n2 = objV[objF[i][2]] - objV[objF[i][0]];
		glm::vec3 tempNormal = glm::cross(n1, n2);
		tempNormal = tempNormal / glm::length(tempNormal);
		faceNormals[i] = tempNormal;

		normalSums[objF[i][0]] += tempNormal;
		vN[objF[i][0]] += 1;

		normalSums[objF[i][1]] += tempNormal;
		vN[objF[i][1]] += 1;

		normalSums[objF[i][2]] += tempNormal;
		vN[objF[i][2]] += 1;
		
	}

	for (int i = 0; i < V_N; i++) {
		vertexNormals[i] = normalSums[i] / (GLfloat)vN[i];
		vertexNormals[i] = vertexNormals[i] / glm::length(vertexNormals[i]);
	}

	return;
}

// ucitavanje izvora svjetla
void pointLightInput() {
	std::cout << "Unesite koordinate izvora svjetla: ";
	std::cin >> pointLight.x >> pointLight.y >> pointLight.z;
	pointLight.w = 1;
	pointLightNorm = pointLight / glm::length(pointLight);

	return;
}


void startLightState() {
	PROG_STATE = LIGHT;
	SHADE_MODE = CONSTANT;
	
	O = originalO;
	G = standardG;
	projectionFunctions();

	pointLightInput();
	computeLightInt();
	std::cout << "PRITISKOM NA * SPACEBAR *  TIPKU MIJENJA SE NACIN SJENCANJA\n";
	std::cout << "> Sjencanje: KONSTANTNO\n";
	glutPostRedisplay();
	return;
}

glm::vec3 calcInt(float LN) {
	return objColor * hiddenCoef + lightInt * objColor * LN;
}

void computeLightInt() {

	// izracunati intenzitete za svaki poligon i tocku

	// faceInt, vertexInt;

	for (int i = 0; i < F_N; i++) {
		float LN = std::fmax(0, glm::dot(pointLightNorm, faceNormals[i]));
		faceInt[i] = calcInt(LN);
		faceInt[i].y = std::fmin(1, faceInt[i].y);
		if (!UNI_COLOR) {
			faceInt[i].x = std::fmin(1, faceInt[i].x);
			faceInt[i].z = std::fmin(1, faceInt[i].z);
		}
		else {
			faceInt[i].x = 0;
			faceInt[i].z = 0;
		}
	}

	for (int i = 0; i < V_N; i++) {
		float LN = std::fmax(0, glm::dot(pointLightNorm, vertexNormals[i]));
		vertexInt[i] = calcInt(LN);
		vertexInt[i].y = std::fmin(1, vertexInt[i].y);
		if (!UNI_COLOR) {
			vertexInt[i].x = std::fmin(1, vertexInt[i].x);
			vertexInt[i].z = std::fmin(1, vertexInt[i].z);
		}
		else {
			vertexInt[i].x = 0;
			vertexInt[i].z = 0;
		}
	}


}

void flipShadeType() {
	if (SHADE_MODE == GOUARD) {
		SHADE_MODE = CONSTANT;
		std::cout << "> Sjencanje: KONSTANTNO\n";
	}
	else {
		SHADE_MODE = GOUARD;
		std::cout << "> Sjencanje: GOUARD\n";
	}
	glutPostRedisplay();
}

void flipProjType() {
	O = originalO;
	if (PROJ_TYPE == MANUAL) {
		PROJ_TYPE = OPENGL;
		std::cout << ">>OpenGL projekcija\n";
	}
	else {
		PROJ_TYPE = MANUAL;
		std::cout << ">>Projekcija iz 5.lab\n";
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-5, 5, -5, 5);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	glutPostRedisplay();
}


// funkcija koja se poziva pri promjeni velicine prozora
void reshapeFunc(int width, int height) {
	WIDTH = width;
	HEIGHT = height;

	glViewport(0, 0, (GLsizei)WIDTH, (GLsizei)HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-5, 5, -5, 5);

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

	for (int i = 0; i < F_N; i++) {
		// uklanjanje straznjih poligona (tj. ignoriranje)
		glm::vec3 tempO = O;
		if (glm::dot(faceNormals[i], tempO) <= 0)continue;

		glm::vec3 vertexA, vertexB, vertexC;

		if (PROJ_TYPE == OPENGL) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(45.0, (float)WIDTH / (float)HEIGHT, 0, 10);
			gluLookAt(O.x, O.y, O.z, G.x, G.y, G.z, 0, 1, 0);
			glMatrixMode(GL_MODELVIEW);
		}
		if (PROJ_TYPE == MANUAL) {
			vertexA = objV_PROJ[objF[i][0]];
			vertexB = objV_PROJ[objF[i][1]];
			vertexC = objV_PROJ[objF[i][2]];
		}
		else {
			vertexA = objV[objF[i][0]];
			vertexB = objV[objF[i][1]];
			vertexC = objV[objF[i][2]];
		}

		if (PROG_STATE == WIREFRAME) {
			//glColor3f(0, 0, 0);
			glBegin(GL_LINE_LOOP);
			glVertex3f(vertexA.x, vertexA.y, vertexA.z);
			glVertex3f(vertexB.x, vertexB.y, vertexB.z);
			glVertex3f(vertexC.x, vertexC.y, vertexC.z);
			glEnd();
		}
		else {
			if (SHADE_MODE == CONSTANT) {
				glColor3f(faceInt[i].x, faceInt[i].y, faceInt[i].z);
				glBegin(GL_TRIANGLES);
				glVertex3f(vertexA.x, vertexA.y, vertexA.z);
				glVertex3f(vertexB.x, vertexB.y, vertexB.z);
				glVertex3f(vertexC.x, vertexC.y, vertexC.z);
				glEnd();
			}
			else {
				int indexA = objF[i][0];
				int indexB = objF[i][1];
				int indexC = objF[i][2];
				glBegin(GL_TRIANGLES);
				glColor3f(vertexInt[indexA].x, vertexInt[indexA].y, vertexInt[indexA].z);
				glVertex3f(vertexA.x, vertexA.y, vertexA.z);
				glColor3f(vertexInt[indexB].x, vertexInt[indexB].y, vertexInt[indexB].z);
				glVertex3f(vertexB.x, vertexB.y, vertexB.z);
				glColor3f(vertexInt[indexC].x, vertexInt[indexC].y, vertexInt[indexC].z);
				glVertex3f(vertexC.x, vertexC.y, vertexC.z);
				glEnd();
			}

		}
	}

	glutSwapBuffers();
	return;

	for (int i = 0; i < V_N; i++) {
		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		glm::vec4 vertex = objV[i];

		glVertex3f(objV_PROJ[i].x, objV_PROJ[i].y, objV_PROJ[i].z);
		glm::vec4 normV = vertex + glm::vec4(vertexNormals[i], 0);
		normV = normV * viewMat * projMat;
		normV = normV / normV.w;
		glVertex3f(normV.x, normV.y, normV.z);
		glEnd();
	}


	glutSwapBuffers();
	return;
}


void keyboardDownFunc(unsigned char key, int x, int y) {

	if (key == 32) {
		if (PROG_STATE == WIREFRAME) {
			startLightState();
		}
		else {
			 flipShadeType();
		}
		return;
	}

	if (key == 27) {
		exit(0);
		return;
	}

	switch (key) {
	case 'p':
		flipProjType();
		break;
	case 'r':
		O = originalO;
		G = standardG;
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

	case 'i':
		pointLight.x += 1;
		computeLightInt();
		glutPostRedisplay();
	case 'o':
		pointLight.x -= 1;
		computeLightInt();
		glutPostRedisplay();

	default:
		break;
	}
}

void keyboardUpFunc(unsigned char key, int x, int y) {
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

	if (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN)
		O_dir.z = 0;
	if (key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT)
		O_dir.x = 0;

	return;
}

//  funckija koja se konstantno vrti u loopu
void idleFunc() {
	O = O + O_dir * glm::vec4(deltaO, deltaO, deltaO, 0);
	G = G + G_dir * glm::vec4(deltaG, deltaG, deltaG, 0);
	projectionFunctions();
	glutPostRedisplay();
}


void printCommands() {

	std::cout << "Upute za pomicanje:\nOciste:\n  - x os: LEFT ARROW / RIGHT ARROW\n  - y os: a/s\n  - z os: UP ARROW/DOWN ARROW\nr je reset ocista\np je promjena nacina projekcije (5. lab ili OpenGL)\n";


}
