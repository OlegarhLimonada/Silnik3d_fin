//Standartowe biblioteki c++
#include <Windows.h>
#include <iostream>
//Biblioteki glew i freeglut
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <gl/GL.h>
#include <gl/GLU.h>
//Biblioteki glm
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//Dol¹czony plik .h stb_image.h który pomaga ladowaæ pliki .png, .jpg, .jpeg i tak dalej
#define STB_IMAGE_IMPLEMENTATION
#include <GL/stb_image.h>

#include <vector>

using namespace std;

// Globalna zmienna dla kata obrotu
float angle = 0;

//Definiowanie wierzholków dla planszy do której bedzie przywiazana tekstura 
const float ground_ver[] = {
    -1.0f, -1.0f,  1.0f,    
     1.0f, -1.0f,  1.0f,    
    -1.0f,  1.0f,  1.0f,   
     1.0f,  1.0f,  1.0f
};
//Definiowanie punktów przywiazania tekstury, za pomoca których tekstura bedzie wiazana z plansza
const float ground_text[] = {
    1.0f, 0.0f,		0.0f, 0.0f,		1.0f, 1.0f,		0.0f, 1.0f
};
//Porzadek punktów przywiazania
const unsigned char ground_ind[] = {
    0, 1, 2,   
    2, 1, 3
};


//Definicja wlasciwosci swiatla
GLfloat lightAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat lightDif[] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat lightSpc[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPos[] = { 0.0f, 4.0f, 0.0f, 1.0f };


// Tablica sciezek(absolutnych) do plików tekstur
const char* textureFiles[] = {
    "C://Users//Oleg//Desktop//Politechnika//GK1//engine3d_fin//Silnik3d_test1//Silnik3d_test1//frame_00_delay-0.12s.jpg"
    //I tak dalej
};

// Liczba tekstur
const int numTextures = sizeof(textureFiles) / sizeof(textureFiles[0]);

// Wektor do przechowywania identyfikatorów tekstur
std::vector<GLuint> textureIds(numTextures);

// Indeks bie¿¹cej tekstury
int currentTextureIndex = 0;

// Czas ostatniej zmiany tekstury
time_t lastTextureChangeTime = 0;

// Interwa³ zmiany tekstury
const float textureChangeInterval = 0.1f;

/**
 * @brief LoadMipmapFromFile wczytuje plik obrazu i tworzy mipmapy dla tekstury OpenGL.
 * \~polish param FileName Œcie¿ka do pliku obrazu.
 * \~polish @return True jeœli sukces, false w przeciwnym razie.
 */
bool LoadMipmapFromFile(const char* FileName) {
    int width, height, nchan;
    unsigned char* dt;
    dt = stbi_load(FileName, &width, &height, &nchan, 0);
    
    if (!dt)	return false;
 
    if (nchan != 3) {
        stbi_image_free(dt);
        return false;
    }

    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, dt);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    stbi_image_free(dt);
    
    return true;
}


/**
 * @class Scene
 * \~polish @brief Reprezentuje scenê z obiektami do narysowania.
 */
class Scene {
public:
    /**
     * \~polish @brief Flaga kontroluj¹ca, czy œwiat³o jest w³¹czone czy wy³¹czone.
     */
    boolean isLightOn = true;

    /**
     * \~polish @brief Rysuje przewodnikowy szeœcian, o czerwonym kolorze, w scenie.
     */
    void drawWireCube() {
        glPushMatrix();
        glTranslatef(-10.0f, 1.0f, -50.0f);
        glRotatef(angle, 1.0f, 0.0f, 1.0f);
        glColor3f(1, 0, 0);
        glutSolidCube(2.0f);
        glPopMatrix();
    }
    /**
     * \~polish @brief Rysuje p³aski sto¿ek, o zielonym kolorze, w scenie.
     */
    void drawFlatCone() {
        glPushMatrix();
        glTranslatef(-5.0f, 0.0f, -50.0f);
        glRotatef(-90, 1.0f, 0.0f, 0.0f);
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        glColor3f(0, 1, 0);
        glutSolidCone(2.0f, 2.0f, 15, 15);
        glPopMatrix();
    }
    /**
     * \~polish @brief Rysuje p³aszczyznê z tekstur¹.
     * \~polish @param currentTextureId Identyfikator bie¿¹cej tekstury do zastosowania.
     */
    void drawGround(GLuint currentTextureId) {
        glPushMatrix();
        glBindTexture(GL_TEXTURE_2D, currentTextureId);
        glEnable(GL_TEXTURE_2D);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, ground_ver);
        glTexCoordPointer(2, GL_FLOAT, 0, ground_text);

        glm::vec3 normal;
        glm::vec3 edge1, edge2;
        for (int i = 0; i < 2; ++i) {
            edge1 = glm::vec3(ground_ver[i * 3 + 3] - ground_ver[i * 3],
                ground_ver[i * 3 + 4] - ground_ver[i * 3 + 1],
                ground_ver[i * 3 + 5] - ground_ver[i * 3 + 2]);

            edge2 = glm::vec3(ground_ver[i * 3 + 6] - ground_ver[i * 3 + 3],
                ground_ver[i * 3 + 7] - ground_ver[i * 3 + 4],
                ground_ver[i * 3 + 8] - ground_ver[i * 3 + 5]);

            normal = glm::cross(edge1, edge2);
            normal = glm::normalize(normal);
            glNormal3f(normal.x, normal.y, normal.z);
        }
        glTranslatef(-7.5f, 4.0f, -50.0f);
        glRotatef(90, 1.0f, 0.0f, 0.0f);
        //glRotatef(angle, 1.0f, 1.0f, 1.0f);
        glScalef(8.0f, 6.0f, 6.0f);
        glColor3f(1, 1, 1);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }

};

/**
 * @class Camera
 * \~polish @brief Reprezentuje kamerê w scenie, z odpowiednimi parametrami(pozycja, rotacja ora ostatnia pozycja myszy), które zmieniaj¹ siê w trakcie dzia³ania programu.
 */
class Camera {
private:
    glm::vec3 position;
    glm::vec2 rotation;
    glm::vec2 lastMousePos;

public:
    /**
     * \~polish @brief Konstruktor dla klasy Camera, ustawiaj¹cy camera na pozycje pocz¹tkow¹
     */
    Camera() : position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f), lastMousePos(0.0f) {}
    /**
     * \~polish @brief Zastosuj transformacjê kamery do stosu macierzy OpenGL, Funkcja stosuje siê ró¿nych metod z biblioteki glm..
     */
    void applyTransform() {
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::rotate(view, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        view = glm::rotate(view, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        view = glm::translate(view, -position);
        glLoadMatrixf(glm::value_ptr(view));
        angle += 1.0f;
        if (angle >= 360) angle = 0;
    }

    /**
     * \~polish @brief Poruszaj kamer¹ do przodu.
     * \~polish @param speed Szybkoœæ ruchu.
     */
    void moveForward(float speed) {
        position += speed * glm::vec3(glm::sin(glm::radians(rotation.y)), -glm::sin(glm::radians(rotation.x)), -glm::cos(glm::radians(rotation.y)));
    }

    /**
    * \~polish @brief Poruszaj kamer¹ do ty³u.
    * \~polish @param speed Szybkoœæ ruchu.
    */
    void moveBackward(float speed) {
        position -= speed * glm::vec3(glm::sin(glm::radians(rotation.y)), -glm::sin(glm::radians(rotation.x)), -glm::cos(glm::radians(rotation.y)));
    }

    /**
     * @brief Strafe kamery w prawo.
     * \~polish @param speed Szybkoœæ ruchu.
     */
    void strafeRight(float speed) {
        position += speed * glm::vec3(glm::cos(glm::radians(rotation.y)), 0.0f, glm::sin(glm::radians(rotation.y)));
    }

    /**
    * @brief Strafe kamery w lewo.
    * \~polish @param speed Szybkoœæ ruchu.
    */
    void strafeLeft(float speed) {
        position -= speed * glm::vec3(glm::cos(glm::radians(rotation.y)), 0.0f, glm::sin(glm::radians(rotation.y)));
    }

    /**
     * \~polish @brief Aktualizuj rotacjê kamery na podstawie ruchu myszy.
     * @param deltaX Zmiana w kierunku X.
     * @param deltaY Zmiana w kierunku Y.
     */
    void updateRotation(float deltaX, float deltaY) {
        rotation.x += deltaY;
        rotation.y += deltaX;
    }
    //Getery i Setery
    /**
     * \~polish @brief Pobierz ostatni¹ pozycjê myszy w osi X.
     * @return Ostatnia pozycja myszy w osi X.
     */
    float getLastMouseX() const { return lastMousePos.x; }
    /**
     * \~polish @brief Pobierz ostatni¹ pozycjê myszy w osi Y.
     * @return Ostatnia pozycja myszy w osi Y.
     */
    float getLastMouseY() const { return lastMousePos.y; }
    /**
     * \~polish @brief Ustaw ostatni¹ pozycjê myszy w osi X.
     * @param newX Nowa pozycja myszy w osi X.
     */
    void setLastMouseX(float newX) { this->lastMousePos.x = newX; }
    /**
    * \~polish @brief Ustaw ostatni¹ pozycjê myszy w osi Y.
    * @param newY Nowa pozycja myszy w osi Y.
    */
    void setLastMouseY(float newY) { this->lastMousePos.y = newY; }
};


//Definiowanie objektów scene klasy "Scene" oraz camera klasy "Camera"
Scene scene;
Camera camera;


/**
 * \~polish @brief Metoda ustawiaj¹ca ró¿ne aspekty i ustawienia OpenGl oraz laduje tekstury.
 */
void enable(void) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glShadeModel(GL_SMOOTH);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightAmb);

    glEnable(GL_LIGHT0);

    glMaterialfv(GL_FRONT, GL_AMBIENT, lightAmb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, lightDif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, lightSpc);
    glMateriali(GL_FRONT, GL_SHININESS, 64);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(numTextures, textureIds.data());

    for (int i = 0; i < numTextures; ++i) {
        glBindTexture(GL_TEXTURE_2D, textureIds[i]);
        if (LoadMipmapFromFile(textureFiles[i])) {
            cout << "Texture loaded: " << textureFiles[i] << endl;
        }
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, ground_ver);
    glTexCoordPointer(2, GL_FLOAT, 0, ground_text);
}


/**
 * @brief Funkcja, która jest stosowana przez OpenGL jako funkcja, rysuj¹ca objekty w oknie programu.
 */
void display(void) {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.applyTransform();
    scene.drawWireCube();
    scene.drawFlatCone();
    scene.drawGround(textureIds[currentTextureIndex]);
    glutSwapBuffers();
}


/**
 * \~polish @brief Funkcja wywo³ywana przy zmianie rozmiaru okna.
 * @param w Nowa szerokoœæ okna.
 * @param h Nowa wysokoœæ okna.
 */
void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat)w / (GLfloat)h, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

/**
 * \~polish @brief Funkcja obs³uguj¹ca klawisze na klawiaturze.
 * \~polish @param key Wciœniêty klawisz.
 * \~polish @param x Wspó³rzêdna X myszy w momencie naciœniêcia klawisza.
 * \~polish @param y Wspó³rzêdna Y myszy w momencie naciœniêcia klawisza.
 */
void keyboard(unsigned char key, int x, int y) {
    const float speed = 0.5f;// Szybkoœæ przymieszciania camery
    if (key == 'w') camera.moveForward(speed);// Do przodu
    if (key == 's') camera.moveBackward(speed);// 
    if (key == 'd') camera.strafeRight(speed);// Lewo
    if (key == 'a') camera.strafeLeft(speed);// Prawo
    if (key == 'q') {//Wl¹czenie/wyl¹czenia œwiatla
        scene.isLightOn = !scene.isLightOn;
        if (scene.isLightOn) glEnable(GL_LIGHT0);
        else glDisable(GL_LIGHT0);
    }
    if (key == 27) exit(0);//Wyjœcie z programu
}


/**
 * \~polish @brief Funkcja wywo³ywana w trakcie braku interakcji u¿ytkownika, obs³uguj¹ca zmiany tekstur.
 */
const int FPS_COUNT = 60; //Licznik klatek przez sekunde
void OnIdle() {
    static int last_time = 0;
    int now_time = glutGet(GLUT_ELAPSED_TIME);
    static float addedtime = 0;
    if (last_time > 0) {
        float elapsed_time = (now_time - last_time) / 1000.0f;
        float target_frame_time = 1.0f / static_cast<float>(FPS_COUNT);
        if (elapsed_time >= target_frame_time) {
            glutPostRedisplay();
            last_time = now_time;
            addedtime = addedtime + elapsed_time;
            if (addedtime >= textureChangeInterval) {
                currentTextureIndex = (currentTextureIndex + 1) % numTextures;
                glBindTexture(GL_TEXTURE_2D, textureIds[currentTextureIndex]);
                addedtime = 0;
            }

        }
    }
    else last_time = now_time;
}

/**
 * \~polish @brief Funkcja obs³uguj¹ca ruch myszy.
 * \~polish @param x Aktualna wspó³rzêdna X myszy.
 * \~polish @param y Aktualna wspó³rzêdna Y myszy.
 */
void motion(int x, int y) {
    camera.updateRotation((x - camera.getLastMouseX()) * 0.1f, (y - camera.getLastMouseY()) * 0.1f);
    camera.setLastMouseX(x);
    camera.setLastMouseY(y);
}

/**
 * \~polish @brief Funkcja obs³uguj¹ca zdarzenie przycisku myszy.
 * @param button Numer przycisku myszy.
 * @param state Stan przycisku myszy (naciœniêty lub puszczony).
 * \~polish @param x Wspó³rzêdna X myszy w momencie zdarzenia.
 * \~polish @param y Wspó³rzêdna Y myszy w momencie zdarzenia.
 */
void mouse(int button, int state, int x, int y) {
    camera.setLastMouseX(x);
    camera.setLastMouseY(y);
}

/**
 * \~polish @brief G³ówna funkcja programu.
 * \~polish @param argc Liczba argumentów wiersza polecenia.
 * \~polish @param argv Tablica argumentów wiersza polecenia.
 * \~polish @return Kod wyjœcia z programu.
 */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
    glutInitWindowSize(1920, 1080);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Baza");
    //glutEnterGameMode(); //Funkcja, która wl¹cza tryb FullScreen dla okan programu
    glutDisplayFunc(display);
    glutIdleFunc(OnIdle);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    enable();
    glutMainLoop();
    return 0;
}