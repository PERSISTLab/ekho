/********************************************************************

	Vector.cpp	Source File

	Vector Algebra Objects, Methods, and Procedures
		Donald H. House  April 17, 1997
		Visualization Laboratory
		Texas A&M University

	Copyright (C) - Donald H. House. 2005

*********************************************************************/

#include "Vector.h"
using namespace std;

Vector2d::Vector2d(double vx, double vy){
  set(vx, vy);
}

Vector2d::Vector2d(const Vector2d &v){
  set(v);
}

Vector3d::Vector3d(double vx, double vy, double vz){
  set(vx, vy, vz);
}

Vector3d::Vector3d(const Vector3d &v){
  set(v);
}

Vector4d::Vector4d(double vx, double vy, double vz, double vw){
  set(vx, vy, vz, vw);
}

Vector4d::Vector4d(const Vector4d &v){
  set(v);
}

Vector::Vector(int vN, double *vx){
  int i;

  setsize(vN);
  if(vx != NULL)
    for(i = 0; i < N; i++)
      v[i] = vx[i];
}

Vector::Vector(const Vector& V){
  set(V);
}

Vector::Vector(double vx, double vy){
  setsize(2);
  set(vx, vy);
}

Vector::Vector(double vx, double vy, double vz){
  setsize(3);
  set(vx, vy, vz);
}

Vector::Vector(double vx, double vy, double vz, double vw){
  setsize(4);
  set(vx, vy, vz, vw);
}

// Destructor
Vector::~Vector(){
  delete []v;
}

//
// Array index form of a vector
// Routines returning an lvalue: i.e. X[i] returns addr of X[i]
//
double& Vector2d::operator[](int i)
{
  if(i < 0 || i > 1){
    cerr << "2D vector index bounds error" << endl;
    exit(1);
  }

  switch(i){
  case 0:
    return x;
  default:
    return y;
  }
}

double& Vector3d::operator[](int i)
{
  if(i < 0 || i > 2){
    cerr << "3D vector index bounds error" << endl;
    exit(1);
  }

  switch(i){
  case 0:
    return x;
  case 1:
    return y;
  default:
    return z;
  }
}

double& Vector4d::operator[](int i)
{
  if(i < 0 || i > 3){
    cerr << "4D vector index bounds error" << endl;
    exit(1);
  }

  switch(i){
  case 0:
    return x;
  case 1:
    return y;
  case 2:
    return z;
  default:
    return w;
  }
}

double& Vector::operator[](int i)
{
  if(i < 0 || i >= N){
    cerr << N << " vector index bounds error" << endl;
    exit(1);
  }

  return v[i];
}

//
// Array index form of a vector
// Routines returning an rvalue: i.e. X[i] returns contents of X[i]
//
const double& Vector2d::operator[](int i) const
{
  if(i < 0 || i > 1){
    cerr << "2D vector index bounds error" << endl;
    exit(1);
  }

  switch(i){
  case 0:
    return x;
  default:
    return y;
  }
}

const double& Vector3d::operator[](int i) const
{
  if(i < 0 || i > 2){
    cerr << "3D vector index bounds error" << endl;
    exit(1);
  }

  switch(i){
  case 0:
    return x;
  case 1:
    return y;
  default:
    return z;
  }
}

const double& Vector4d::operator[](int i) const
{
  if(i < 0 || i > 3){
    cerr << "4D vector index bounds error" << endl;
    exit(1);
  }

  switch(i){
  case 0:
    return x;
  case 1:
    return y;
  case 2:
    return z;
  default:
    return w;
  }
}

const double& Vector::operator[](int i) const
{
  if(i < 0 || i >= N){
    cerr << N << " vector index bounds error" << endl;
    exit(1);
  }

  return v[i];
}

// Set size of generic Vector
void Vector::setsize(int vN){
  if(vN < 0){
    cerr << "vector dimension of " << vN << " invalid" << endl;
    exit(1);    
  }

  N = vN;
  if(N == 0)
    v = NULL;
  else{
    v = new double[N];
    if(!v){
      cerr << "not enough memory to allocate vector of size " << N << endl;
      exit(1);    
    }
  }

  int i;
  for(i = 0; i < N; i++)
    v[i] = 0;
}

// Projections of vectors into other dimensions
Vector2d::operator Vector3d(){
  Vector3d v1(x, y, 0);
  return v1;
}

Vector2d::operator Vector4d(){
  Vector4d v1(x, y, 0, 0);
  return v1;
}

Vector2d::operator Vector(){
  Vector v1(x, y);
  return v1;
}

Vector3d::operator Vector4d(){

  Vector4d v1(x, y, z, 0);
  return v1;
}

Vector3d::operator Vector(){
  Vector v1(x, y, z);
  return v1;
}

Vector4d::operator Vector(){
  Vector v1(x, y, z, w);
  return v1;
}

Vector::operator Vector2d(){
  if(N > 2){
    cerr << "cannot convert " << N << " Vector to Vector2d" << endl;
    exit(1);    
  }

  Vector2d rv;
  int i;
  for(i = 0; i < N; i++)
    rv[i] = v[i];
  for(; i < 2; i++)
    rv[i] = 0;
  return rv;
}

Vector::operator Vector3d(){
  if(N > 3){
    cerr << "cannot convert " << N << " Vector to Vector3d" << endl;
    exit(1);    
  }

  Vector3d rv;
  int i;
  for(i = 0; i < N; i++)
    rv[i] = v[i];
  for(; i < 3; i++)
    rv[i] = 0;
  return rv;
}

Vector::operator Vector4d(){
  if(N > 4){
    cerr << "cannot convert " << N << " Vector to Vector4d" << endl;
    exit(1);    
  }

  Vector4d rv;
  int i;
  for(i = 0; i < N; i++)
    rv[i] = v[i];
  for(; i < 4; i++)
    rv[i] = 0;
  return rv;
}

int Vector::getn() const{
  return N;
}

// Compute the norm of a vector.
Vector2d Vector2d::normalize() const
{
  double magnitude;
  Vector2d newv;
  magnitude = norm();
  if (abs(x) > magnitude * HUGENUMBER ||
      abs(y) > magnitude * HUGENUMBER ){
    cerr << "Attempting to take the norm of a zero 2D vector." << endl;
  }
  newv.x = x / magnitude;
  newv.y = y / magnitude;
  return newv;
}
Vector3d Vector3d::normalize() const
{
  double magnitude;
  Vector3d newv;
  magnitude = norm();
  if (abs(x) > magnitude * HUGENUMBER ||
      abs(y) > magnitude * HUGENUMBER ||
      abs(z) > magnitude * HUGENUMBER ){
    cerr << "Attempting to take the norm of a zero 3D vector." << endl;
  }
  newv.x = x / magnitude;
  newv.y = y / magnitude;
  newv.z = z / magnitude;
  return newv;
}
Vector4d Vector4d::normalize() const
{
  double magnitude;
  Vector4d newv;
  magnitude = norm();
  if (abs(x) > magnitude * HUGENUMBER ||
      abs(y) > magnitude * HUGENUMBER ||
      abs(z) > magnitude * HUGENUMBER ){
    cerr << "taking the norm of a zero 4D vector." << endl;
  }
  newv.x = x / magnitude;
  newv.y = y / magnitude;
  newv.z = z / magnitude;
  newv.w = 1.0;
  return newv;
}
Vector Vector::normalize() const
{
  double magnitude;
  Vector newv(N);
  magnitude = norm();
  int i;

  for(i = 0; i < N; i++){
    newv.v[i] = v[i] / magnitude;
    if(abs(v[i]) > magnitude * HUGENUMBER){
      cerr << "taking the norm of a zero" << N << " Vector" << endl;
      break;
    }
  }
  for(; i < N; i++){
    newv.v[i] = v[i] / magnitude;
  }
  return newv;
}

//  Normalize a 4D Vector's x, y, z components by the w component, and
//  set the w component to 1.0
Vector4d Vector4d::wnorm() const
{
  Vector4d newv;
  if (w == 1.0)
    return *this;
  else if (fabs(w) < SMALLNUMBER)
    cerr << "w-Normalizing vector [" <<
      x << " " << y << " " << z<< " " << w << "] w ~= 0!" << endl;

  newv.x = x / w;
  newv.y = y / w;
  newv.z = z / w;
  newv.w = 1.0;
  return newv;
}

//  Set the components of a Vector according to the arguments
void Vector2d::set(double vx, double vy)
{
  x = vx;
  y = vy;
}
void Vector2d::set(const Vector2d &v)
{
  x = v.x;
  y = v.y;
}
void Vector3d::set(double vx, double vy, double vz)
{
  x = vx;
  y = vy;
  z = vz;
}
void Vector3d::set(const Vector3d &v)
{
  x = v.x;
  y = v.y;
  z = v.z;
}
void Vector4d::set(double vx, double vy, double vz, double vw)
{
  x = vx;
  y = vy;
  z = vz;
  w = vw;
}
void Vector4d::set(const Vector4d &v)
{
  x = v.x;
  y = v.y;
  z = v.z;
  w = v.w;
}
void Vector::set(double *vx){
  int i;
  for(i = 0; i < N; i++)
    v[i] = vx[i];
}
void Vector::set(const Vector &V){
  int i;

  setsize(V.N);
  for(i = 0; i < N; i++)
    v[i] = V.v[i];
}
void Vector::set(double vx, double vy){
  if(N < 2){
    cerr << "too many arguments for " << N << " Vector set" << endl;
    exit(1);
  }

  v[0] = vx;
  v[1] = vy;
  int i;
  for(i = 2; i < N; i++)
    v[i] = 0;
}
void Vector::set(double vx, double vy, double vz){
  if(N < 3){
    cerr << "too many arguments for " << N << " Vector set" << endl;
    exit(1);
  }

  v[0] = vx;
  v[1] = vy;
  v[2] = vz;
  int i;
  for(i = 3; i < N; i++)
    v[i] = 0;
}
void Vector::set(double vx, double vy, double vz, double vw){
  if(N < 4){
    cerr << "too many arguments for " << N << " Vector set" << endl;
    exit(1);
  }

  v[0] = vx;
  v[1] = vy;
  v[2] = vz;
  v[3] = vw;
  int i;
  for(i = 4; i < N; i++)
    v[i] = 0;
} 

// Print a Vector to the standard output device.
void Vector2d::print() const
{
  cout << "[" << x << " " << y << "]";
}
void Vector2d::print(int w, int p) const
{
  cout << "[" << setw(w) << setprecision(p) << Round(x, p) << " ";
  cout << setw(w) << setprecision(p) << Round(y, p) << "]";
}
void Vector3d::print() const
{
  cout << "[" << x << " " << y << " " << z << "]";
}
void Vector3d::print(int w, int p) const
{
  cout << "[" << setw(w) << setprecision(p) << Round(x, p) << " ";
  cout << setw(w) << setprecision(p) << Round(y, p) << " ";
  cout << setw(w) << setprecision(p) << Round(z, p) << "]";
}
void Vector4d::print() const
{
  cout << "[" << x << " " << y << " " << z << " " << w << "]";
}
void Vector4d::print(int w, int p) const
{
  cout << "[" << setw(w) << setprecision(p) << Round(x, p) << " ";
  cout << setw(w) << setprecision(p) << Round(y, p) << " ";
  cout << setw(w) << setprecision(p) << Round(z, p) << " ";
  cout << setw(w) << setprecision(p) << Round(w, p) << "]";
}
void Vector::print() const
{
  if(N == 0)
    cout << "[]";
  else{
    int i;
    cout << "[" << v[0];
    for(i = 1; i < N; i++)
      cout << " " << v[i];
    cout << "]";
  }
}
void Vector::print(int w, int p) const
{
  if(N == 0)
    cout << "[]";
  else{
    int i;
    cout << "[" << setw(w) << setprecision(p) << Round(v[0], p);
    for(i = 1; i < N; i++)
      cout << " " << setw(w) << setprecision(p) << Round(v[i], p);
    cout << "]";
  }
}

// Compute the magnitude of a vector.
double Vector2d::norm() const
{
  return sqrt(normsqr());
}
double Vector3d::norm() const
{
  return sqrt(normsqr());
}
double Vector4d::norm() const
{
  return sqrt(normsqr());
}
double Vector::norm() const
{
  return sqrt(normsqr());
}

// Compute the squared magnitude of a vector.
double Vector2d::normsqr() const
{
  return Sqr(x) + Sqr(y);
}
double Vector3d::normsqr() const
{
  return Sqr(x) + Sqr(y) + Sqr(z);
}
double Vector4d::normsqr() const
{
  return Sqr(x) + Sqr(y) + Sqr(z) + Sqr(w);
}
double Vector::normsqr() const
{
  int i;
  double sumsqr = 0;
  for(i = 0; i < N; i++)
    sumsqr += Sqr(v[i]);

  return sumsqr;
}

// Unary negation of a vector
Vector2d operator-(const Vector2d& v1){
  Vector2d r(-v1.x, -v1.y);
  return r;
}
Vector3d operator-(const Vector3d& v1){
  Vector3d r(-v1.x, -v1.y, -v1.z);
  return r;
}
Vector4d operator-(const Vector4d& v1){
  Vector4d r(-v1.x, -v1.y, -v1.z, -v1.w);
  return r;
}
Vector operator-(const Vector& v1){
  Vector r(v1.N);
  int i;
  
  for(i = 0; i < v1.N; i++)
    r[i] = -v1.v[i];
  return r;
}

//  Addition of 2 Vectors.
Vector2d operator+(const Vector2d& v1, const Vector2d& v2)
{
  Vector2d r;
  r.x = v1.x + v2.x;
  r.y = v1.y + v2.y;
  return r;
}

Vector3d Vector3d::operator+(const Vector3d& v2) const
{
  Vector3d r;
  r.x = x + v2.x;
  r.y = y + v2.y;
  r.z = z + v2.z;
  return r;
}

Vector4d Vector4d::operator+(const Vector4d& v2) const
{
  Vector4d r;
  r.x = x + v2.x;
  r.y = y + v2.y;
  r.z = z + v2.z;
  r.w = w + v2.w;
  return r;
}

Vector operator+(const Vector& v1, const Vector& v2)
{
  if(v1.N != v2.N){
    cerr << "cannot add " << v1.N << " Vector to " << v2.N << " Vector" << 
      endl;
    exit(1);
  }
  Vector r(v1.N);
  int i;

  for(i = 0; i < v1.N; i++)
    r[i] = v1.v[i] + v2.v[i];
  return r;
}

//  Subtract two Vectors.
Vector2d operator-(const Vector2d& v1, const Vector2d& v2)
{
  Vector2d r;
  r.x = v1.x - v2.x;
  r.y = v1.y - v2.y;
  return r;
}
Vector3d operator-(const Vector3d& v1, const Vector3d& v2)
{
  Vector3d r;
  r.x = v1.x - v2.x;
  r.y = v1.y - v2.y;
  r.z = v1.z - v2.z;
  return r;
}
Vector4d operator-(const Vector4d& v1, const Vector4d& v2)
{
  Vector4d r;
  r.x = v1.x - v2.x;
  r.y = v1.y - v2.y;
  r.z = v1.z - v2.z;
  r.w = v1.w - v2.w;
  return r;
}
Vector operator-(const Vector& v1, const Vector& v2)
{
  if(v1.N != v2.N){
    cerr << "cannot subtract " << v2.N << " Vector from " << v1.N <<
      " Vector" << endl;
    exit(1);
  }

  Vector r(v1.N);
  int i;

  for(i = 0; i < v1.N; i++)
    r[i] = v1.v[i] - v2.v[i];
  return(r);
}

// Product of vector and scalar
Vector2d operator*(const Vector2d& v, double s)
{
  Vector2d r;

  r.x = v.x * s;
  r.y = v.y * s;
  return r;
}
Vector2d operator*(double s, const Vector2d& v)
{
  Vector2d r;

  r.x = v.x * s;
  r.y = v.y * s;
  return r;
}
Vector3d operator*(const Vector3d& v, double s)
{
  Vector3d r;

  r.x = v.x * s;
  r.y = v.y * s;
  r.z = v.z * s;
  return r;
}
Vector3d operator*(double s, const Vector3d& v)
{
  Vector3d r;

  r.x = v.x * s;
  r.y = v.y * s;
  r.z = v.z * s;
  return r;
}
Vector4d operator*(const Vector4d& v, double s)
{
  Vector4d r;

  r.x = v.x * s;
  r.y = v.y * s;
  r.z = v.z * s;
  r.w = v.w * s;
  return r;
}
Vector4d operator*(double s, const Vector4d& v)
{
  Vector4d r;

  r.x = v.x * s;
  r.y = v.y * s;
  r.z = v.z * s;
  r.w = v.w * s;
  return r;
}
Vector operator*(double s, const Vector& v1)
{
  Vector r(v1.N);
  int i;

  for(i = 0; i < v1.N; i++)
    r[i] = s * v1.v[i];
  return r;
}
Vector operator*(const Vector& v1, double s)
{
  Vector r(v1.N);
  int i;

  for(i = 0; i < v1.N; i++)
    r[i] = s * v1.v[i];
  return r;
}

// Inner product of two Vectors
double operator*(const Vector2d& v1, const Vector2d& v2)
{
  return(v1.x * v2.x +
	 v1.y * v2.y);
}
double operator*(const Vector3d& v1, const Vector3d& v2)
{
  return(v1.x * v2.x +
	 v1.y * v2.y +
	 v1.z * v2.z);
}
double operator*(const Vector4d& v1, const Vector4d& v2)
{
  return(v1.x * v2.x +
	 v1.y * v2.y +
	 v1.z * v2.z +
	 v1.w * v2.w);
}
double operator*(const Vector& v1, const Vector& v2)
{
  if(v1.N != v2.N){
    cerr << "cannot take dot product of " << v1.N << " Vector with " << 
      v2.N << " Vector" << endl;
    exit(1);
  }

  int i;
  double p = 0;

  for(i = 0; i < v1.N; i++)
    p += v1.v[i] * v2.v[i];
  return p;
}
//  Component-wise multiplication of two Vectors
Vector2d operator^(const Vector2d& v1, const Vector2d& v2)
{
  Vector2d r;
  r.x = v1.x * v2.x;
  r.y = v1.y * v2.y;
  return r;
}
Vector3d operator^(const Vector3d& v1, const Vector3d& v2)
{
  Vector3d r;
  r.x = v1.x * v2.x;
  r.y = v1.y * v2.y;
  r.z = v1.z * v2.z;
  return r;
}
Vector4d operator^(const Vector4d& v1, const Vector4d& v2)
{
  Vector4d r;
  r.x = v1.x * v2.x;
  r.y = v1.y * v2.y;
  r.z = v1.z * v2.z;
  r.w = v1.w * v2.w;
  return r;
}
Vector operator^(const Vector& v1, const Vector& v2)
{
  Vector r(v1.N);
  int i;

  for(i = 0; i < v1.N; i++)
    r[i] = v1.v[i] * v2.v[i];
  return r;
}

// Cross product of two Vectors
Vector3d operator%(const Vector2d& v1, const Vector2d& v2)
{
  Vector3d cp;
  cp.x = 0;
  cp.y = 0;
  cp.z = v1.x * v2.y - v1.y * v2.x;
  return (cp);
}
Vector3d operator%(const Vector3d& v1, const Vector3d& v2)
{
  Vector3d cp;
  cp.x = v1.y * v2.z - v1.z * v2.y;
  cp.y = v1.z * v2.x - v1.x * v2.z;
  cp.z = v1.x * v2.y - v1.y * v2.x;
  return (cp);
}
Vector4d operator%(const Vector4d& v1, const Vector4d& v2)
{
  cerr << "sorry, cross product of Vector4d's not yet implemented" << endl;
  exit(1);

  Vector4d cp = v1;	// here only to avoid warning message from compiler
  cp = v2;
  return (cp);
}
Vector operator%(const Vector& v1, const Vector& v2)
{
  if(v1.N != v2.N || v1.N < 3){
    cerr << "cannot take cross product of " << v1.N << " Vector with " <<
      v2.N << " Vector" << endl;
    exit(1);
  }

  cerr << "sorry, (Vector % Vector) not implemented yet" << endl;
  exit(1);
  return v1;
}

// Divide a vector by a scalar.
Vector2d operator/(const Vector2d& v, double s)
{
  Vector2d r;
  r.x = v.x / s;
  r.y = v.y / s;
  return(r);
}
Vector3d operator/(const Vector3d& v, double s)
{
  Vector3d r;
  r.x = v.x / s;
  r.y = v.y / s;
  r.z = v.z / s;
  return(r);
}
Vector4d operator/(const Vector4d& v, double s)
{
  Vector4d r;
  r.x = v.x / s;
  r.y = v.y / s;
  r.z = v.z / s;
  r.w = 1.0;
  return(r);
}
Vector operator/(const Vector& v1, double s)
{
  Vector r(v1.N);
  int i;

  for(i = 0; i < v1.N; i++)
    r[i] = v1.v[i] / s;
  return r;
}

// Determine if two Vectors are identical.
short operator==(const Vector2d& one, const Vector2d& two)
{
  return((one.x == two.x) && (one.y == two.y));
}
short operator==(const Vector3d& one, const Vector3d& two)
{
  return((one.x == two.x) && (one.y == two.y) && (one.z == two.z));
}
short operator==(const Vector4d& one, const Vector4d& two)
{
  return((one.x == two.x) && (one.y == two.y) && (one.z == two.z));
}
short operator==(const Vector& one, const Vector& two)
{
  int n, N;
  double *v, *V;

  if(one.N >= two.N){
    N = one.N;
    n = two.N;
    V = one.v;
    v = two.v;
  }
  else{
    N = two.N;
    n = one.N;
    V = two.v;
    v = one.v;
  }

  int i;
  for(i = 0; i < n; i++)
    if(V[i] != v[i])
      return 0;
  for(;i < N; i++)
    if(V[i] != 0)
      return 0;
    
  return 1;
}

// assign one vector to another
const Vector& Vector::operator=(const Vector& v2){
  int i;

  if(N != v2.N){
    delete []v;
    setsize(v2.N);
  }
  for(i = 0; i < N; i++)
    v[i] = v2.v[i];

  return *this;
}

ostream& operator<< (ostream& os, const Vector2d& v){
  os << "[" << v.x << " " << v.y << "]";
  return os;
}

ostream& operator<< (ostream& os, const Vector3d& v){
  os << "[" << v.x << " " << v.y << " " << v.z << "]";
  return os;
}

ostream& operator<< (ostream& os, const Vector4d& v){
  os << "[" << v.x << " " << v.y << " " << v.z << " " << v.w << "]";
  return os;
}

ostream& operator<< (ostream& os, const Vector& v){
  if(v.N == 0)
    os << "[]";
  else{
    int i;
    os << "[" << v[0];
    for(i = 1; i < v.N; i++)
      os << " " << v[i];
    os << "]";
  }
  return os;
}

