/*****************************************************************************
//
//	File:			3dVector.h
//
//	Description:	Generic 3d vector.
//
//	Copyright 1997, Be Incorporated
//
// The 3dKit is a fully object-oriented library, going from low-level 3d
// engine to high-level 3d API. The whole system is decomposed in functional
// blocks, that can include both high-level and low-level API. To allow all
// those classes to communicate smoothly and efficiently, most of their datas
// and methods are public. That can introduce some confusion in the sense that
// reading the headers will not allow developer to differenciate API levels
// by their protection only. That's why some comments were added in the key
// classes of the 3dKit to help you recognize which API you should use
// depending of the level of usage you want.
// ***************************************************************************/

#ifndef _3D_VECTOR_H
#define _3D_VECTOR_H

#include <stdio.h>

/**************************************
// B3dVector.
/ *************************************/

class B3dVector {

/*************************************
// HIGH-LEVEL API                   */

 public:
	float   x;
	float   y;
	float   z;
	inline B3dVector& Set(float new_x, float new_y, float new_z);
	/* Get and set the three coordinates of the vector.
	   */

/*************************************
// LOW-LEVEL API                    */

	inline float operator* (const B3dVector& v2) const;
	/* return the scalar product of both vectors.
	   */
	inline B3dVector& operator* (const float k) const;
	inline void operator*= (const float k);
	/* multiply all component of a vector by the same coefficient.
	   */
	inline B3dVector& operator- (void) const;
	inline B3dVector& operator- (const B3dVector& v2) const;
	inline void operator-= (const B3dVector v2);
	inline B3dVector& operator+ (const B3dVector& v2) const;
	inline void operator+= (const B3dVector v2);
	/* Add and substract vectors one with another.
	   */
	inline B3dVector& operator^ (const B3dVector& v2) const;
	inline void operator^= (const B3dVector v2);
	/* return the vectorial product of two vectors.
	   */
	inline bool operator== (const B3dVector v2);
	/* test the equality of two vectors.
	   */
	inline bool operator!= (const B3dVector v2);
	/* test the inequality of two vectors.
	   */
	inline void operator<<= (const B3dVector v2);
	/* Get the minimal component of the 2 vectors, component per
	   component.
	   */
	inline void operator>>= (const B3dVector v2);
	/* Get the maximal component of the 2 vectors, component per
	   component.
	   */
	inline B3dVector& VectorielX() const;
	/* return the vectorial product of the vector with the X axis.
	   */
	inline B3dVector& VectorielY() const;
	/* return the vectorial product of the vector with the Y axis.
	   */
	inline B3dVector& VectorielZ() const;
	/* return the vectorial product of the vector with the Z axis.
	   */
	B3dVector& Rotate(B3dVector *axis, float alpha);
	float Length() const;  
	/* return the length of the vector, without modifying it.
	   */
	float Norm(bool return_prev = false);  
	/* change the length of the vector to make it equal to 1,
	   but don't change its direction and orientation. This
	   function can also return the previous length is needed.
	   */  
	inline float Square();
	/* returns Length()*Length().
	   */
	   
/*************************************
// PRIVATE STUFF                    */

	void Debug();
};

/**************************************
// HIGH-LEVEL API
/ *************************************/

extern B3dVector B_VECTOR_NULL;

/**************************************
// INLINE FUNCTION DEFINITIONS
/ *************************************/

float B3dVector::operator* (const B3dVector& v2) const {
	return x*v2.x + y*v2.y + z*v2.z;
}

B3dVector& B3dVector::operator* (const float k) const {
	B3dVector v;

	v.x = x*k;
	v.y = y*k;
	v.z = z*k;
	return v;
}

void B3dVector::operator*= (const float k) {
	x *= k;
	y *= k;
	z *= k;
}

B3dVector& B3dVector::operator- (void) const {
	B3dVector v;

	v.x = -x;
	v.y = -y;
	v.z = -z;
	return v;	
};

B3dVector& B3dVector::operator- (const B3dVector& v2) const {
	B3dVector v;

	v.x = x-v2.x;
	v.y = y-v2.y;
	v.z = z-v2.z;
	return v;
}

void B3dVector::operator-= (const B3dVector v2) {
	x -= v2.x;
	y -= v2.y;
	z -= v2.z;
}

B3dVector& B3dVector::operator+ (const B3dVector& v2) const {
	B3dVector v;

	v.x = x+v2.x;
	v.y = y+v2.y;
	v.z = z+v2.z;
	return v;
}

void B3dVector::operator+= (const B3dVector v2) {
	x += v2.x;
	y += v2.y;
	z += v2.z;
}

B3dVector& B3dVector::operator^ (const B3dVector& v2) const {
	B3dVector v;

	v.x = y*v2.z - z*v2.y;
	v.y = z*v2.x - x*v2.z;
	v.z = x*v2.y - y*v2.x;
	return v;
}

void B3dVector::operator^= (const B3dVector v2) {
	B3dVector v;

	v.x = y*v2.z - z*v2.y;
	v.y = z*v2.x - x*v2.z;
	v.z = x*v2.y - y*v2.x;
	x = v.x;
	y = v.y;
	z = v.z;
}

B3dVector& B3dVector::VectorielX() const {
	B3dVector v;

	v.x = 0.0;
	v.y = z;
	v.z = -y;
	return v;
}

B3dVector& B3dVector::VectorielY() const {
	B3dVector v;

	v.x = -z;
	v.y = 0.0;
	v.z = x;
	return v;
}

B3dVector& B3dVector::VectorielZ() const {
	B3dVector v;

	v.x = y;
	v.y = -x;
	v.z = 0.0;
	return v;
}

bool B3dVector::operator== (const B3dVector v2) {
	return (x == v2.x) && (y == v2.y) && (z == v2.z);
}

bool B3dVector::operator!= (const B3dVector v2) {
	return (x != v2.x) || (y != v2.y) || (z != v2.z);
}

void B3dVector::operator<<= (const B3dVector v2) {
	if (v2.x < x) x = v2.x;
	if (v2.y < y) y = v2.y;
	if (v2.z < z) z = v2.z;
}

void B3dVector::operator>>= (const B3dVector v2) {
	if (v2.x > x) x = v2.x;
	if (v2.y > y) y = v2.y;
	if (v2.z > z) z = v2.z;
}

B3dVector& B3dVector::Set(float new_x, float new_y, float new_z) {
	x = new_x;
	y = new_y;
	z = new_z;
	return *this;
}

float B3dVector::Square() {
	return (x*x + y*y + z*z);
}

#endif
