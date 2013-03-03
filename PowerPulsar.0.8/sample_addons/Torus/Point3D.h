/************************************************************
	Author	: X
	1997	: XD
	5/2000	: RM updated for  PP 0.8.0

	IMPORTANT: this source is not done by RM. I claim no warranty
	and no right on it. You are not allowed to reuse this source
	or associated source/headers without the prior approbation of
	its author, X <x.com>

************************************************************/



#ifndef _POINT3D_H_
#define _POINT3D_H_

class SinTable;

class Point3D {
	public:
		double x,y,z;
		SinTable *sintab;

		Point3D(SinTable *);
		Point3D(double,double,double, SinTable *);
		Point3D(const Point3D &src);
		
		void SetTo(double,double,double);

		// Surcharge de l'operateur '='
		Point3D &operator = ( Point3D &p) {
			x=p.x;
			y=p.y;
			z=p.z;
			return *this;
		}
	
		// Surcharge de l'operateur '+' =>translation
		Point3D operator + ( Point3D &p) const {
			Point3D tmp(x+p.x,y+p.y,z+p.z,sintab);
			return tmp;
		}
	
		Point3D operator - ( Point3D &p) const {
			Point3D tmp(x-p.x,y-p.y,z-p.z,sintab);
			return tmp;
		}


		double operator * (Point3D &p) const { // produit scalaire
			return x*p.x + y*p.y + z*p.z;
		}


		Point3D Vectoriel(Point3D &);
		void Vectoriel(Point3D &, Point3D &);

		Point3D operator * (double i) const {
			return Point3D(x*i,y*i,z*i,sintab);
		}

		Point3D operator / (double i) const {
			Point3D tmp(x/i,y/i,z/i,sintab);
			return tmp;
		}
		
		// milieu de 2 point
		Point3D mil(Point3D &);

		// creation du vecteur this->p
		Point3D Vect(Point3D &);

		// distance entre 2 points
		double dist(Point3D &); // distance
		double dist2(Point3D &);  // distance au carre ...
		
		void normalize(void);
		
		// norm du vecteur
  		inline double Taille(void) { return sqrt(x*x+y*y+z*z); }
  		inline double Taille2(void) { return x*x+y*y+z*z; }

		// retourne la valeur du cos entre les deux vecteurs
		// en calculant le produit scalaire/normes
  		double Cos(Point3D &);	
  		
		void RotateX(double rad);
		void RotateY(double rad);
		void RotateZ(double rad);

		void FastRotateX(int deg);
		void FastRotateY(int deg);
		void FastRotateZ(int deg);
	
		Point3D &Inv(void);
		void print3D(void);
		bool null(void);
		
		void AutoMult(double *m);

		double *Rotate3D(Point3D &direct_dest);
};


#endif
