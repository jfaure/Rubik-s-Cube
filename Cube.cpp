#include <SFML/Window.hpp>
#include <GL/glu.h>
#include <cmath>
#include <iostream>
#include <cctype>
#include <map>
#include <algorithm>

#include "Quaternion.hpp"

// static texture data
static GLuint texName;
static GLubyte checkImage[64][64][4];

// Cube Vertex data for OpenGL
struct glCube
{
  GLfloat size;
  std::array<GLfloat, 6 * 12> cube;
  std::array<GLfloat,  6 * 4> cl = {
      1,.5, 0, 1,  // left  // orange y
      //0.5, 0, 0.26, 0, // Pink
      1, 0, 0, 1,  // right // red    y
      1, 1, 0, 1,  // down  // yellow
      //0.5, 0.9, 1, 0, //light blue
      1, 1, 1, 1,  // up    // white
      0, 0, 1, 1,  // back  // blue
      0, 1, 0, 1}; // front // green 

  glCube(): glCube(.47) {}
  glCube(GLfloat s): size(s), cube { 
    -s, -s, -s, -s, -s,  s, -s,  s,  s, -s,  s, -s, // left
     s, -s, -s,  s, -s,  s,  s,  s,  s,  s,  s, -s, // right
    -s, -s, -s,  s, -s, -s,  s, -s,  s, -s, -s,  s, // down
    -s,  s, -s,  s,  s, -s,  s,  s,  s, -s,  s,  s, // up
    -s, -s, -s,  s, -s, -s,  s,  s, -s, -s,  s, -s, // back
    -s, -s,  s,  s, -s,  s,  s,  s,  s, -s,  s,  s} // front  
    {
      if (not texName) {
      int i, j, c;
      for (i = 0; i < 64; ++i)
        for (j = 0; j < 64; ++j) {
          c = 255 * !((i ^ j) & 0x8);
          checkImage[i][j][0] =  checkImage[i][j][1] =  checkImage[i][j][2] = c;
          checkImage[i][j][3] = 255;
        }
      glGenTextures(1, &texName);
      glBindTexture(GL_TEXTURE_2D, texName);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA,
          GL_UNSIGNED_BYTE, checkImage);
      }
    }

  void draw_with_texture() const
  {
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    for (int i = 0; i < 6; ++i) { 
      glColor4f(cl[i * 4], cl[i * 4 + 1], cl[i * 4 + 2], cl[i * 4 + 3]);
      glTexCoord2f(0, 0); glVertex3f(cube[i * 12 + 0], cube[i * 12 + 1], cube[i * 12 + 2]);
      glTexCoord2f(0, 1); glVertex3f(cube[i * 12 + 3], cube[i * 12 + 4], cube[i * 12 + 5]);
      glTexCoord2f(1, 1); glVertex3f(cube[i * 12 + 6], cube[i * 12 + 7], cube[i * 12 + 8]);
      glTexCoord2f(1, 0); glVertex3f(cube[i * 12 + 9], cube[i * 12 +10], cube[i * 12 +11]);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
  }

  void draw() const
  {
    glBegin(GL_QUADS);
    for (int i = 0; i < 6; ++i) {
      glColor4f(cl[i * 4], cl[i * 4 + 1], cl[i * 4 + 2], cl[i * 4 + 3]);
      for (int j: { 0, 3, 6, 9 })
        glVertex3f(cube[i * 12 + j], cube[i * 12 + j + 1], cube[i * 12 + j + 2]);
    }
    glEnd();
  }
};

struct Cubelet
{
  GLfloat x, y, z;
  Quaternion Orientation;
  glCube glcube;

  Cubelet(GLfloat x, GLfloat y, GLfloat z, Quaternion q = Quaternion(1, 0, 0, 0)):
      x(x), y(y), z(z), Orientation(q), glcube(glCube())
  { // Black out interior faces
    int t = x < 0 ? 4 : 0;
    glcube.cl[t] = glcube.cl[t + 1] = glcube.cl[t + 2] = 0;
    t = y < 0 ? 12 : 8;
    glcube.cl[t] = glcube.cl[t + 1] = glcube.cl[t + 2] = 0;
    t = z < 0 ? 20 : 16;
    glcube.cl[t] = glcube.cl[t + 1] = glcube.cl[t + 2] = 0;
    if (!x) glcube.cl[4] = glcube.cl[5] = glcube.cl[6] = 0;
    if (!y) glcube.cl[12] = glcube.cl[13] = glcube.cl[14] = 0;
    if (!z) glcube.cl[20] = glcube.cl[21] = glcube.cl[22] = 0;
  }

  friend std::ostream &operator<<(std::ostream &out, Cubelet const c)
  { return out << "(" << c.x << ", " << c.y << ", " << c.z << ")\n"; }

  void rotate(Quaternion const &t)
  {
    Orientation = t * Orientation;
    t.rotateVector(x, y, z);
  }

  void draw() const
  {
    glPushMatrix();
    glTranslatef(x, y, z);
    Orientation.glRotate();
//  if (x == 0 && y == 1 && z == 0)
//    glcube.draw_with_texture(); // draw image on centre white cubelet
//  else 
      glcube.draw();
    glPopMatrix();
  }

  void align() { x = round(x), y = round(y), z = round(z); }
};

struct Rubik
{
  Quaternion Orientation;
  std::vector<Cubelet> cubelets, animationStart;
  std::map<char, Cubelet*> centres; // subset of cubelets

  // Animation
  GLfloat movetime = 0.9;
  Cubelet *pendingFace = NULL;
  GLfloat pendingAngle;
  std::vector<Cubelet> animationTarget = cubelets;
 
  Rubik(): Orientation(0, M_PI/4, M_PI/4)
  {
    cubelets.reserve(27);
      for (int x: {-1, 0, 1})
        for (int y: {-1, 0, 1})
          for (int z: {-1, 0, 1})
            cubelets.push_back(Cubelet(x, y, z));
 // assign Centre cubelets
    centres['r'] = &cubelets[04]; centres['d'] = &cubelets[10];
    centres['b'] = &cubelets[12]; centres['f'] = &cubelets[14];
    centres['u'] = &cubelets[16]; centres['l'] = &cubelets[22];
  }

  void finishAnimation()
  {
    if (pendingFace) {
      cubelets = animationStart;
      rotateFace(pendingFace, pendingAngle < 0 ? -M_PI/2 : M_PI/2);
      pendingFace = NULL;
    }
  }

  void draw(GLfloat secs)
  { 
    if (pendingFace) {
      GLfloat rtheta = M_PI / 2 * secs / movetime; // quarterturn
      if (pendingAngle < 0) rtheta = -rtheta;
      if (std::abs(rtheta) > std::abs(pendingAngle))
        finishAnimation();
      else {
        rotateFace(pendingFace, rtheta);
        pendingAngle -= rtheta;
      }
    }
 
    for (Cubelet c: cubelets)
      c.draw();
  }

  void doMove(char move)
  {
    GLfloat angle = -M_PI / 2;
    if (isupper(move)) { angle = -angle; move = tolower(move); }
    if (centres[move]) {
      if (movetime > 0) { // setup animation
        finishAnimation(); // can't have two faces rotating at once
        pendingFace = centres[move];
        pendingAngle = angle;
        animationStart = cubelets;
      }
      else 
        rotateFace(centres[move], angle);
    }
  }

// Rotate face by given angle
  void rotateFace(Cubelet *face, GLfloat angle)
  {
    Quaternion t; t.loadAxisAngle(angle, face->x, face->y, face->z);
    rotateFace(face, t);
  }

  void rotateFace(Cubelet *face, Quaternion t)
  {
    for (Cubelet &c: cubelets) // find the cubelets of this face
      if (  face->x && face->x == c.x
         || face->y && face->y == c.y
         || face->z && face->z == c.z)
        c.rotate(t);
   }

  // Cube orientation represented with Quaternion
  void rotate(GLfloat dx, GLfloat dy)
  {
    Orientation = Quaternion(0, dx * (M_PI/180), dy * (M_PI/180)) * Orientation;
    Orientation.glRotate();
  }

  // read moves from string and execute them in a loop
  // ! if first char of s is not a valid move, will reverse moves at string end
  void story(std::string const &moves)
  {
    static sf::Clock clock;
    static int pointer = -1;

    if ((int) clock.getElapsedTime().asSeconds() > pointer) {
      int p = ++pointer % (2 * moves.size()) - moves.size();
      doMove(p < 0 ? toupper(moves[-p]) : moves[p]);
    }
  }
};

void sendtoffmpeg()
{
#include <stdio.h>
  int w(700), h(450);
  int size(w * h * sizeof(char) * 3);
  char data[size];
  static FILE *ffmpeg = popen("/usr/bin/ffmpeg -f rawvideo -pix_fmt rgb24 -s 700x450 -i pipe:0 out.avi", "w");

  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);
  fwrite(data, 1, size, ffmpeg);
}

void init(int w, int h)
{
  glClearDepth(1);
  glClearColor(0, 0, 0, 0);
  glShadeModel(GL_FLAT);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

// Projection matrix
  glMatrixMode(GL_PROJECTION);
  glFrustum((GLfloat)-w / h, (GLfloat)w / h, -1, 1, 3, 500); 
  //also gluPerspective(50, (GLfloat)w/h, 1, 20);
  glMatrixMode(GL_MODELVIEW);
}

int main()
{ 
  // Options
  int w(1400), h(900);
  GLfloat defaultzoom = 12;
  sf::Window window(sf::VideoMode(w, h), "sfml cube", sf::Style::Default,
      sf::ContextSettings(24)); // request 24bit depth buffer
  window.setActive();

  Rubik rubik;
  sf::Clock clock;
  sf::Vector2i mouse, diff(0, 0);
  GLfloat r = defaultzoom;
  bool redraw(true);

  init(w, h);

  while (window.isOpen()) {
    sf::Event event;
    float secs = clock.restart().asSeconds();

// Events
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed
           || event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
        window.close();
      else if (event.type == sf::Event::Resized)
        glViewport(0, 0, window.getSize().x, window.getSize().y), redraw = true;
      else if (event.type == sf::Event::TextEntered) {
        rubik.doMove(static_cast<char>(event.text.unicode));
        if (event.text.unicode == ' ') 
          r = defaultzoom;
      }
      else if (event.type == sf::Event::MouseWheelScrolled)
        r *= 1 + event.mouseWheelScroll.delta / 20.f, redraw = true;
    }  
    if (sf::Mouse::isButtonPressed(sf::Mouse::Middle))
      diff = sf::Mouse::getPosition(window) - mouse, redraw = true;
    else
      mouse = sf::Mouse::getPosition(window);

    if (redraw || diff.x || diff.y || rubik.pendingFace) { // only redraw if necessary
    // View transformations
      glLoadIdentity();
      glTranslatef(0, 0, -r);
      rubik.rotate(diff.x * secs, diff.y * secs);
      
     // rubik.story(" urdtfl");

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      rubik.draw(secs);
      window.display(); //   sendtoffmpeg();
      redraw = false;
    }
  }
}
