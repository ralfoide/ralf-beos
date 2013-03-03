/************************************************************
	Author	: X
	1997	: XD
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, X <x.com>

************************************************************/

// *****************************************
// Point3D.
// *****************************************


#include "Point3D.h"
#include "SinTable.h"
#include <math.h>
#include <stdio.h>

// Constructeur lambda.
Point3D::Point3D(SinTable *s) {
	x=y=z=0;
	sintab = s;
}

// construction par valeur
Point3D::Point3D(double valx, double valy, double valz,SinTable *s) {
	x=valx;
	y=valy;
	z=valz;
	sintab = s;
}



// construction par copie
Point3D::Point3D(const Point3D &src) {
	x=src.x;
	y=src.y;
	z=src.z;
	sintab = src.sintab;
}

// construction par valeur
void Point3D::SetTo(double valx, double valy, double valz) {
	x=valx;
	y=valy;
	z=valz;
}


// produit vectoriel
Point3D Point3D::Vectoriel(Point3D &p) {
	Point3D tmp((y*p.z-z*p.y),(z*p.x-x*p.z),(x*p.y-y*p.x),sintab);
	return tmp;
}

// produit vectoriel
void Point3D::Vectoriel(Point3D &p1,Point3D &p2) {
	x = p1.y * p2.z - p1.z * p2.y;
	y = p1.z * p2.x - p1.x * p2.z;
	z = p1.x * p2.y - p1.y * p2.x;
}


void Point3D::normalize(void) {
	double taille = sqrt(x*x+y*y+z*z);
	x /= taille;
	y /= taille;
	z /= taille;
}


Point3D Point3D::mil(Point3D &p) {
	Point3D milieu((x+p.x)/2,(y+p.y)/2,(z+p.z)/2,sintab);
	return milieu;
}

void Point3D::RotateX(double rad) {
	// seules Y et Z changent !
	double ytmp;
	
	ytmp = y*cos(rad)-z*sin(rad);
	z = y*sin(rad)+z*cos(rad);
	y = ytmp;
	
}

void Point3D::RotateY(double rad) {
	// seules Z,X changent !
	double ztmp;
	
	ztmp = z*cos(rad)-x*sin(rad);
	x = z*sin(rad)+x*cos(rad);
	z = ztmp;
}

void Point3D::RotateZ(double rad) {
	// rotation selon l'axe Z : seules X,Y changent : equivalent a
	// une rotation 2D classique
	double xtmp;
	
	xtmp = x*cos(rad)-y*sin(rad);
	y = x*sin(rad)+y*cos(rad);
	x = xtmp;
}

void Point3D::FastRotateX(int deg) {
	// seules Y et Z changent !
	double ytmp;
	
	ytmp = y*sintab->Cos(deg)-z*sintab->Sin(deg);
	z = y*sintab->Sin(deg)+z*sintab->Cos(deg);
	y = ytmp;
	
}

void Point3D::FastRotateY(int deg) {
	// seules Z,X changent !
	double ztmp;
	
	ztmp = z*sintab->Cos(deg)-x*sintab->Sin(deg);
	x = z*sintab->Sin(deg)+x*sintab->Cos(deg);
	z = ztmp;
}

void Point3D::FastRotateZ(int deg) {
	// rotation selon l'axe Z : seules X,Y changent : equivalent a
	// une rotation 2D classique
	double xtmp;
	
	xtmp = x*sintab->Cos(deg)-y*sintab->Sin(deg);
	y = x*sintab->Sin(deg)+y*sintab->Cos(deg);
	x = xtmp;
}


Point3D Point3D::Vect(Point3D &p) {
	Point3D vect(p-(*this));
	return vect;
}

double Point3D::dist(Point3D &p) {
	return (sqrt(pow((x-p.x),2)+pow((y-p.y),2)+pow((z-p.z),2)));
}


double Point3D::dist2(Point3D &p) {
	
	return (pow((x-p.x),2) + pow((y-p.y),2) + pow((z-p.z),2));
}


Point3D & Point3D::Inv(void) { // inverse le vecteur.
	x = -x;
	y = -y;
	z = -z;
	return *this;
}


double Point3D::Cos(Point3D &pt) {
	double cosinus;
	cosinus = x*pt.x + y*pt.y + z*pt.z;
	cosinus/= (Taille()*pt.Taille());
	return cosinus;
}

void Point3D::print3D(void){
	printf("(%f,%f,%f)\n",x,y,z);
}


void Point3D::AutoMult(double *m) {
	double xtmp,ytmp,ztmp, w;
	
	xtmp = m[0+0*4]*x + m[1+0*4]*y + m[2+0*4]*z + m[3+0*4];
	ytmp = m[0+1*4]*x + m[1+1*4]*y + m[2+1*4]*z + m[3+1*4];
	ztmp = m[0+2*4]*x + m[1+2*4]*y + m[2+2*4]*z + m[3+2*4];
	w  =   m[0+3*4]*x + m[1+3*4]*y + m[2+3*4]*z + m[3+3*4];
	
	x = xtmp/w;
	y = ytmp/w;
	z = ztmp/w;

}


bool Point3D::null(void) {
	return (x==0 && y==0 && z==0);
}



// transformation depuis un axe sur x vers direct_dest
double *Point3D::Rotate3D(Point3D &direct_dest) {
	double *m = new double[16];
	m[3+0*3] = m[3+1*4] = m[3+2*4] = m[0+3*4] = m[1+3*4] = m[2+3*4] = 0;
	m[3+3*4] = 1;
	
	m[0+0*4] = direct_dest.x;
	m[0+1*4] = direct_dest.y;
	m[0+2*4] = direct_dest.z;

	m[1+0*4] = direct_dest.x;
	m[1+1*4] = 0;
	m[1+2*4] = 0;
	
	m[2+0*4] = direct_dest.x;
	m[2+1*4] = direct_dest.y;
	m[2+2*4] = direct_dest.z;
	
	return m;
}







