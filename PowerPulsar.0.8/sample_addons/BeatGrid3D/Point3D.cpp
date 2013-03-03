/************************************************************
	Author	: Laurent Pontier
	1997	: LP
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, Laurent Pontier

************************************************************/

// *****************************************
// Point3D.
// *****************************************


#include "Point3D.h"
#include <math.h>
#include <stdio.h>

// Constructeur lambda.
Point3D::Point3D() {
	x=y=z=0;
}

// construction par valeur
Point3D::Point3D(double valx, double valy, double valz) {
	x=valx;
	y=valy;
	z=valz;
}



// construction par copie
Point3D::Point3D(const Point3D &src) {
	x=src.x;
	y=src.y;
	z=src.z;
}

// construction par valeur
void Point3D::SetTo(double valx, double valy, double valz) {
	x=valx;
	y=valy;
	z=valz;
}


// produit vectoriel
Point3D Point3D::Vectoriel(Point3D &p) {
	Point3D tmp((y*p.z-z*p.y),(z*p.x-x*p.z),(x*p.y-y*p.x));
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
	Point3D milieu((x+p.x)/2,(y+p.y)/2,(z+p.z)/2);
	return milieu;
}

void Point3D::RotateX(double rad) {
	// seules Y et Z changent !
	double ytmp;
	static	double LastAngleX=0;
	static double	CosLastAngleX=1;
	static double SinLastAngleX=0;

	if (LastAngleX!=rad)
	{
		LastAngleX=rad;
		CosLastAngleX=cos(rad);
		SinLastAngleX=sin(rad);
	}
	ytmp = y*CosLastAngleX-z*SinLastAngleX;
	z = y*SinLastAngleX+z*CosLastAngleX;
	y = ytmp;
}

void Point3D::RotateY(double rad) {
	// seules Z,X changent !
	double ztmp;
	
	static double LastAngleY=0;
	static double	CosLastAngleY=1;
	static double SinLastAngleY=0;

	if (	LastAngleY!=rad)
	{
		CosLastAngleY=cos(rad);
		SinLastAngleY=sin(rad);
		LastAngleY=rad;
	}
	//printf("y\n");
	ztmp = z*CosLastAngleY-x*SinLastAngleY;
	x = z*SinLastAngleY+x*CosLastAngleY;
	z = ztmp;
}

void Point3D::RotateZ(double rad) {
	// rotation selon l'axe Z : seules X,Y changent : equivalent a
	// une rotation 2D classique
	double xtmp;

	static double LastAngleZ=0;
	static double	CosLastAngleZ=1;
	static double SinLastAngleZ=0;

	if (	LastAngleZ!=rad)
	{
		CosLastAngleZ=cos(rad);
		SinLastAngleZ=sin(rad);
		LastAngleZ=rad;
	}
	//printf("z\n");
	xtmp = x*CosLastAngleZ-y*SinLastAngleZ;
	y = x*SinLastAngleZ+y*CosLastAngleZ;
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







