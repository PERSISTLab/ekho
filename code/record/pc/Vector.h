/********************************************************************

	Vector.h	Header File

	Vector Algebra Objects, Methods, and Procedures
		Donald H. House  April 17, 1997
		Visualization Laboratory
		Texas A&M University

	Copyright (C) - Donald H. House. 2005

*********************************************************************/

#ifndef _H_Vector
#define _H_Vector

#include <stdio.h>
#include <cstdlib>
#include <stdbool.h>
#include <cmath>

#include <iostream>
#include <iomanip>

/* range of real numbers */
#define SMALLNUMBER	1.0e-5
#define HUGENUMBER	1.0e10

/* Miscellaneous Scalar Math */
#define Abs(x)		(((x) < 0) ? (-(x)) : (x))
#define Sqr(x)		((x) * (x))
#define Min(x1,x2)	((x1)<(x2)?(x1):(x2))
#define Max(x1,x2)	((x1)>(x2)?(x1):(x2))
#define Round(x, p)	(((int)((x)*pow(10.0,p)+((x)<0?-0.5:0.5)))/pow(10.0,p))
#define Sign(x)		((x)>=0? 1: -1)
#define Swap(x1, x2)	{int tmp=x1; x1=x2; x2=tmp}
#define ApplySign(x, y)	((y) >= 0? Abs(x): -Abs(x))

/* Angle Conversions & Constants */

#ifndef PI
#define PI 3.1415926535897
#endif

#define RAD2DEG (180/PI)
#define DEG2RAD (PI/180)

#define DegToRad(x) ((x)*DEG2RAD)
#define RadToDeg(x) ((x)*RAD2DEG)

using namespace std;

/* Vector Descriptions and Operations */

class Vector2d;
class Vector3d;
class Vector4d;
class Vector;

class Vector2d {
public:
  double x, y;

  Vector2d(double vx = 0, double vy = 0);
  Vector2d(const Vector2d &v);

  double& operator[](int i);
  const double& operator[](int i) const;

  operator Vector3d();
  operator Vector4d();
  operator Vector();

  void print() const;
  void print(int w, int p) const;	// print with width and precision

  double norm() const;			// magnitude of vector
  double normsqr() const;		// magnitude squared
  Vector2d normalize() const;		// normalize

  void set(double vx = 0, double vy = 0);	// set assuming y = 0
  void set(const Vector2d &v);

  /* Vector2d operator prototypes */
  friend Vector2d operator-(const Vector2d& v1); // unary negation of vector
  friend Vector2d operator+(const Vector2d& v1, const Vector2d& v2);//addition 
  friend Vector2d operator-(const Vector2d& v1, const Vector2d& v2);//subtract
  friend Vector2d operator*(const Vector2d& v, double s); // scalar mult
  friend Vector2d operator*(double s, const Vector2d& v);
  friend double   operator*(const Vector2d& v1, const Vector2d& v2); // dot
  friend Vector2d operator^(const Vector2d& v1, const Vector2d& v2); //compt *
  friend Vector3d operator%(const Vector2d& v1, const Vector2d& v2); // cross
  friend Vector2d operator/(const Vector2d& v, double s); // division by scalar
  friend short    operator==(const Vector2d& one, const Vector2d& two); // eq
  friend ostream& operator<< (ostream& os, const Vector2d& v);
};

class Vector3d {
public:
  double x, y, z;

  Vector3d(double vx = 0, double vy = 0, double vz = 0);
  Vector3d(const Vector3d &v);

  double& operator[](int i);
  const double& operator[](int i) const;

  operator Vector4d();
  operator Vector();

  void print() const;
  void print(int w, int p) const;	// print with width and precision

  double norm() const;			// magnitude of vector
  double normsqr() const;		// magnitude squared
  Vector3d normalize() const;		// normalize

  void set(double vx = 0, double vy = 0, double vz = 0); // set
  void set(const Vector3d &v);

  /* Vector3d operator prototypes */
  friend Vector3d operator-(const Vector3d& v1);	// unary negation
  Vector3d operator+(const Vector3d& v2) const; // vector addition 
  friend Vector3d operator-(const Vector3d& v1, const Vector3d& v2); // subtract
  friend Vector3d operator*(const Vector3d& v, double s);      // multiply
  friend Vector3d operator*(double s, const Vector3d& v);
  friend double   operator*(const Vector3d& v1, const Vector3d& v2); // dot
  friend Vector3d operator^(const Vector3d& v1, const Vector3d& v2); // compt *
  friend Vector3d operator%(const Vector3d& v1, const Vector3d& v2); // cross
  friend Vector3d operator/(const Vector3d& v, double s); // division by scalar
  friend short    operator==(const Vector3d& one, const Vector3d& two); // equ
  friend ostream& operator<< (ostream& os, const Vector3d& v);
};

class Vector4d {
public:
  double x, y, z, w;

  Vector4d(double vx = 0, double vy = 0, double vz = 0, double vw = 0);
  Vector4d(const Vector4d &v);

  double& operator[](int i);
  const double& operator[](int i) const;
  
  operator Vector();

  void print() const;
  void print(int w, int p) const;	// print with width and precision

  double norm() const;			// magnitude of vector
  double normsqr() const;		// magnitude squared
  Vector4d normalize() const;		// normalize
  Vector4d wnorm() const;		// normalize to w coord.

  void set(double vx = 0, double vy = 0, double vz = 0, double vw = 0); // set
  void set(const Vector4d &v);

  /* Vector4d operator prototypes */
  friend Vector4d operator-(const Vector4d& v1);  // unary negation
  Vector4d operator+(const Vector4d& v2) const; // vector addition 
  friend Vector4d operator-(const Vector4d& v1, const Vector4d& v2); //subtract
  friend Vector4d operator*(const Vector4d& v, double s);	// multiply
  friend Vector4d operator*(double s, const Vector4d& v);
  friend double   operator*(const Vector4d& v1, const Vector4d& v2); // dot
  friend Vector4d operator^(const Vector4d& v1, const Vector4d& v2); // compt *
  friend Vector4d operator%(const Vector4d& v1, const Vector4d& v2); // cross
  friend Vector4d operator/(const Vector4d& v, double s); // divide by scalar
  friend short    operator==(const Vector4d& one, const Vector4d& two); // equ
  friend ostream& operator<< (ostream& os, const Vector4d& v);
};

class Vector {
protected:
  int N;
  double *v;

public:
  Vector(int vN = 0, double *vx = NULL);
  Vector(const Vector& V);
  Vector(double vx, double vy);
  Vector(double vx, double vy, double vz);
  Vector(double vx, double vy, double vz, double vw);

  ~Vector();

  void setsize(int vN);

  double& operator[](int i);
  const double& operator[](int i) const;  

  operator Vector2d();
  operator Vector3d();
  operator Vector4d();

  int getn() const;

  void print() const;
  void print(int w, int p) const;	// print with width and precision

  double norm() const;			// magnitude of vector
  double normsqr() const;		// magnitude squared
  Vector normalize() const;		// normalize

  void set(double *vx); 		// set
  void set(const Vector &v);
  void set(double vx, double vy);
  void set(double vx, double vy, double vz);
  void set(double vx, double vy, double vz, double vw);  

  /* Vector operator prototypes */
  const Vector& operator=(const Vector& v2);		// assignment
  friend Vector operator-(const Vector& v1);		// unary negation
  friend Vector operator+(const Vector& v1, const Vector& v2); // vector add
  friend Vector operator-(const Vector& v1, const Vector& v2); // vector sub
  friend Vector operator*(const Vector& v, double s);       // scalar multiply
  friend Vector operator*(double s, const Vector& v);
  friend Vector operator^(const Vector& v1, const Vector& v2); // component *
  friend double operator*(const Vector& v1, const Vector& v2); // dot product 
  friend Vector operator%(const Vector& v1, const Vector& v2); // cross product
  friend Vector operator/(const Vector& v, double s);	  // division by scalar
  friend short  operator==(const Vector& one, const Vector& two); // equality
  friend ostream& operator<< (ostream& os, const Vector& v);
};

#endif
