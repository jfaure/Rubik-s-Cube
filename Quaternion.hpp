#pragma once

/*
 * Quaternion: generalization of complex numbers in 4 dimensions
 * Versor (unit quaternion): represents a rotation in 3 dimensional space.
 * They are much more convenient to work with than Euler angles.
 */
struct Quaternion
{
  GLfloat w, x, y, z;

  Quaternion(): Quaternion(1, 0, 0, 0) {}
  Quaternion(GLfloat qw, GLfloat qx, GLfloat qy, GLfloat qz):
    w(qw), x(qx), y(qy), z(qz) {}

  friend std::ostream &operator<<(std::ostream &out, Quaternion const &q)
  { return out << "[" << q.w << ", (" << q.x << ", " << q.y << ", " << q.z << ")]\n"; }

  void loadAxisAngle(GLfloat angle, GLfloat ax, GLfloat ay, GLfloat az) {
    GLfloat s = sin(angle/2);
    *this = Quaternion(cos(angle/2), ax*s, ay*s, az*s);
  }

  // Construct from Euler angles (yaw, pitch, roll)
  Quaternion(GLfloat yaw, GLfloat pitch, GLfloat roll)
  {
    float cy = cos(yaw/2),   sy = sin(yaw/2),
          cr = cos(roll/2),  sr = sin(roll/2),
          cp = cos(pitch/2), sp = sin(pitch/2);

    *this = Quaternion(
        cy*cr*cp + sy*sr*sp,
        cy*sr*cp - sy*cr*sp,
        cy*cr*sp + sy*sr*cp,
        sy*cr*cp - cy*sr*sp);

    unitize();
  }

  GLfloat norm() const { return std::sqrt(w*w + x*x + y*y + z*z); }
  void unitize() { GLfloat n(norm()); w /= n; x /= n; y /= n; z /= n; }
  Quaternion conjugate() const { return Quaternion(w, -x, -y, -z); }
  Quaternion inverse() const { Quaternion q(conjugate()); q.unitize(); return q; }
  Quaternion operator-() const { return Quaternion(-w, -x, -y, -z); }

  Quaternion operator-=(Quaternion const &q) 
  { w -= q.w; x -= q.x; y -= q.y; z -= q.z; return *this; }
  Quaternion operator+=(Quaternion const &q) 
  { w += q.w; x += q.x; y += q.y; z += q.z; return *this; }
  Quaternion operator*=(GLfloat const f) { w *= f; x *= f; y *= f; z *= f; return *this; }
  Quaternion operator*=(Quaternion const &q) { 
    return *this = Quaternion(
        w*q.w - x*q.x - y*q.y - z*q.z,
        w*q.x + x*q.w - z*q.y + y*q.z,
        w*q.y - x*q.z + y*q.w + z*q.x,
        w*q.z + x*q.y - y*q.x + z*q.w); }

  Quaternion operator*(GLfloat const f)     const { return Quaternion(*this) *= f; }
  Quaternion operator*(Quaternion const &q) const { return Quaternion(*this) *= q; }
  Quaternion operator+(Quaternion const &q) const { return Quaternion(*this) += q; }
  Quaternion operator-(Quaternion const &q) const { return Quaternion(*this) -= q; }

  friend bool operator==(Quaternion const &a, Quaternion const &b) 
  { return a.w == b.w && a.x == b.x && a.y == b.y && a.z == b.z; }
  friend bool operator!=(Quaternion const &a, Quaternion const &b) { return not (a == b); }

  void ToAxisAngle(GLfloat &angle, GLfloat &ax, GLfloat &ay, GLfloat &az) const { 
    GLfloat s = std::sqrt(x*x + y*y + z*z);
    angle = (180 / M_PI) * 2 * atan2(s, w); //acos(w);
    if (s < 0.001) { ax = ay = az = 0; return; }
    ax = x / s; ay = y / s; az = z / s;
  }
  
  void glRotate() const
  {
    GLfloat angle, x, y, z;
    ToAxisAngle(angle, x, y, z);
    glRotatef(angle, x, y, z);
  }

  void rotateVector(GLfloat &vx, GLfloat &vy, GLfloat &vz) const
  {
    Quaternion qm = *this * Quaternion(0, vx, vy, vz) * (*this).inverse();
    vx = qm.x; vy = qm.y; vz = qm.z;  
  }
  
  Quaternion slerp(Quaternion const &v0, Quaternion const &v1, double t)
  {
    GLfloat dot = v0.w*v1.w + v0.x*v1.x + v0.y*v1.y + v0.z*v1.z;

    if (dot > 0.9995) {
        Quaternion result = v0 + (v1 - v0)*t;
        result.unitize();
        return result;
    }
    if (dot < -1) dot = -1; else if (dot > 1) dot = 1; // stay in domain of acos
    GLfloat theta_0 = acos(dot);  // theta_0 = angle between input vectors
    GLfloat theta = theta_0*t;    // theta = angle between v0 and result
    Quaternion v2 = v1 - v0*dot;
    v2.unitize();              // { v0, v2 } is now an orthonormal basis
    return v0*cos(theta) + v2*sin(theta);
  }

};
