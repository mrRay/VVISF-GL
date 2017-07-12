#ifndef VVGeom_h
#define VVGeom_h

#include <iostream>
#include "VVBase.hpp"


namespace VVGL
{


using namespace std;




//	basic geometry structs
struct Point	{
	double		x = 0.;
	double		y = 0.;
	Point() {};
	Point(const double & inX, const double & inY) { x=inX;y=inY; };
	inline bool operator==(const Point & n) const { return (n.x==this->x && n.y==this->y); }
	inline bool operator!=(const Point & n) const { return !(*this==(n)); }
	friend ostream & operator<<(ostream & os, const Point & n) { os<<"("<<n.x<<", "<<n.y<<")";return os; }
};
struct Size	{
	double		width = 0.;
	double		height = 0.;
	Size() {};
	Size(const double & inWidth, const double & inHeight) { width=inWidth;height=inHeight; };
	inline bool operator==(const Size & n) const { return (n.width==this->width && n.height==this->height); }
	inline bool operator!=(const Size & n) const { return !(*this==(n)); }
	friend ostream & operator<<(ostream & os, const Size & n) { os<<"("<<n.width<<"x"<<n.height<<")";return os; }
};
struct Rect	{
	Point	origin = {0.,0.};
	Size	size = {0., 0.};
	Rect() {};
	Rect(const double & inX, const double & inY, const double & inW, const double & inH) { origin={inX,inY}; size={inW,inH}; };
	inline bool operator==(const Rect & n) const { return (this->origin.operator==(n.origin) && this->size.operator==(n.size)); }
	inline bool operator!=(const Rect & n) const { return !(*this==(n)); }
	friend ostream & operator<<(ostream & os, const Rect & n) { os<<"{"<<n.origin<<","<<n.size<<"}";return os; }
};

//	inline functions for doing basic ops on the basic geometry structs
inline double MinX(const Rect & r) { return ((r.size.width>=0) ? (r.origin.x) : (r.origin.x+r.size.width)); }
inline double MaxX(const Rect & r) { return ((r.size.width>=0) ? (r.origin.x+r.size.width) : (r.origin.x)); }
inline double MinY(const Rect & r) { return ((r.size.height>=0) ? (r.origin.y) : (r.origin.y+r.size.height)); }
inline double MaxY(const Rect & r) { return ((r.size.height>=0) ? (r.origin.y+r.size.height) : (r.origin.y)); }
inline double MidX(const Rect & r) { return (r.origin.x+(r.size.width/2.0)); }
inline double MidY(const Rect & r) { return (r.origin.y+(r.size.height/2.0)); }
inline Point TopLeft(const Rect & r) { return { MinX(r), MaxY(r) }; }
inline Point TopRight(const Rect & r) { return { MaxX(r), MaxY(r) }; }
inline Point BotLeft(const Rect & r) { return { MinX(r), MinY(r) }; }
inline Point BotRight(const Rect & r) { return { MaxX(r), MinY(r) }; }
inline Point Center(const Rect & r) { return { MidX(r), MidY(r) }; }

inline Point Add(const Point & lhs, const Point & rhs) { return { lhs.x+rhs.x, lhs.y+rhs.y }; }
inline Point Subtract(const Point & lhs, const Point & rhs) { return { (lhs.x-rhs.x), (lhs.y-rhs.y) }; }
inline bool IsZero(const Point & n) { return (n.x==0.0 && n.y==0.0); }
inline Size Add(const Size & lhs, const Size & rhs) { return { lhs.width+rhs.width, lhs.height+rhs.height }; }
inline Size Subtract(const Size & lhs, const Size & rhs) { return { lhs.width-rhs.width, lhs.height-rhs.height }; }
inline bool IsZero(const Size & n) { return (n.width==0.0 && n.height==0.0); }
inline bool IsZero(const Rect & n) { return (IsZero(n.origin) && IsZero(n.size)); }




//	sizing modes used for the 'ResizeRect' function
enum SizingMode	{
	SizingMode_Fit,	//!<	the content is made as large as possible, proportionally, without cutting itself off or going outside the bounds of the desired area
	SizingMode_Fill,	//!<	the content is made as large as possible, proportionally, to fill the desired area- some of the content may get cut off
	SizingMode_Stretch,	//!<	the content is scaled to fit perfectly within the desired area- some stretching or squashing may occur, this isn't necessarily proportional
	SizingMode_Copy	//!<	the content is copied directly to the desired area- it is not made any larger or smaller
};
//	this method resizes one rect to fit in another using the provided sizing mode.  useful for calculating sizes while preserving aspect ratios.
Rect ResizeRect(const Rect & fitThisRect, const Rect & inThisRect, const SizingMode & sizingMode);




//	these structs simplify the process of constructing, referring to, and packing/unpacking GL-based data describing a quad
struct GLBufferVertexXY {
	float		geo[2] = { 0., 0. };
	inline bool operator==(const GLBufferVertexXY & n) const { return (this->geo[0]==n.geo[0] && this->geo[1]==n.geo[1]); }
	inline bool operator!=(const GLBufferVertexXY & n) const { return !(*this==(n)); }
};
struct GLBufferVertexXYST {
	float		geo[2] = { 0., 0. };
	float		tex[2] = { 0., 0. };
	inline bool operator==(const GLBufferVertexXYST & n) const { return (this->geo[0]==n.geo[0] && this->geo[1]==n.geo[1] && this->tex[0]==n.tex[0] && this->tex[1]==n.tex[1]); }
	inline bool operator!=(const GLBufferVertexXYST & n) const { return !(*this==(n)); }
};
struct GLBufferVertexXYC {
	float		geo[2] = { 0., 0. };
	GLColor		color;
	inline bool operator==(const GLBufferVertexXYC & n) const { return (this->geo[0]==n.geo[0] && this->geo[1]==n.geo[1] && this->color.r==n.color.r && this->color.g==n.color.g && this->color.b==n.color.b && this->color.a==n.color.a); }
	inline bool operator!=(const GLBufferVertexXYC & n) const { return !(*this==(n)); }
};
struct GLBufferQuadXY {
	GLBufferVertexXY	bl;
	GLBufferVertexXY	br;
	GLBufferVertexXY	tl;
	GLBufferVertexXY	tr;
	inline int32_t stride() const { return sizeof(bl); }
	inline bool operator==(const GLBufferQuadXY & n) const { return (this->bl==n.bl && this->br==n.br && this->tl==n.tl && this->tr==n.tr); }
	inline bool operator!=(const GLBufferQuadXY & n) const { return !(*this==(n)); }
};
struct GLBufferQuadXYST {
	GLBufferVertexXYST	bl;
	GLBufferVertexXYST	br;
	GLBufferVertexXYST	tl;
	GLBufferVertexXYST	tr;
	inline int32_t stride() const { return sizeof(bl); }
	inline bool operator==(const GLBufferQuadXYST & n) const { return (this->bl==n.bl && this->br==n.br && this->tl==n.tl && this->tr==n.tr); }
	inline bool operator!=(const GLBufferQuadXYST & n) const { return !(*this==(n)); }
};
struct GLBufferQuadXYC {
	GLBufferVertexXYC	bl;
	GLBufferVertexXYC	br;
	GLBufferVertexXYC	tl;
	GLBufferVertexXYC	tr;
	inline int32_t stride() const { return sizeof(bl); }
	inline bool operator==(const GLBufferQuadXYC & n) const { return (this->bl==n.bl && this->br==n.br && this->tl==n.tl && this->tr==n.tr); }
	inline bool operator!=(const GLBufferQuadXYC & n) const { return !(*this==(n)); }
};

struct GLBufferVertexXYZ	{
	float		geo[3] = { 0., 0., 0. };
	inline bool operator==(const GLBufferVertexXYZ & n) const { return (this->geo[0]==n.geo[0] && this->geo[1]==n.geo[1] && this->geo[2]==n.geo[2]); }
	inline bool operator!=(const GLBufferVertexXYZ & n) const { return !(*this==(n)); }
};
struct GLBufferVertexXYZST	{
	float		geo[3] = { 0., 0., 0. };
	float		tex[2] = { 0., 0. };
	inline bool operator==(const GLBufferVertexXYZST & n) const { return (this->geo[0]==n.geo[0] && this->geo[1]==n.geo[1] && this->geo[2]==n.geo[2] && this->tex[0]==n.tex[0] && this->tex[1]==n.tex[1]); }
	inline bool operator!=(const GLBufferVertexXYZST & n) const { return !(*this==(n)); }
};
struct GLBufferVertexXYZC	{
	float		geo[3] = { 0., 0., 0. };
	GLColor		color;
	inline bool operator==(const GLBufferVertexXYZC & n) const { return (this->geo[0]==n.geo[0] && this->geo[1]==n.geo[1] && this->geo[2]==n.geo[2] && this->color.r==n.color.r && this->color.g==n.color.g && this->color.b==n.color.b && this->color.a==n.color.a); }
	inline bool operator!=(const GLBufferVertexXYZC & n) const { return !(*this==(n)); }
};
struct GLBufferQuadXYZ	{
	GLBufferVertexXYZ	bl;
	GLBufferVertexXYZ	br;
	GLBufferVertexXYZ	tl;
	GLBufferVertexXYZ	tr;
	inline int32_t stride() const { return sizeof(bl); }
	inline bool operator==(const GLBufferQuadXYZ & n) const { return (this->bl==n.bl && this->br==n.br && this->tl==n.tl && this->tr==n.tr); }
	inline bool operator!=(const GLBufferQuadXYZ & n) const { return !(*this==(n)); }
};
struct GLBufferQuadXYZST {
	GLBufferVertexXYZST	bl;
	GLBufferVertexXYZST	br;
	GLBufferVertexXYZST	tl;
	GLBufferVertexXYZST	tr;
	inline int32_t stride() const { return sizeof(bl); }
	inline bool operator==(const GLBufferQuadXYZST & n) const { return (this->bl==n.bl && this->br==n.br && this->tl==n.tl && this->tr==n.tr); }
	inline bool operator!=(const GLBufferQuadXYZST & n) const { return !(*this==(n)); }
};
struct GLBufferQuadXYZC {
	GLBufferVertexXYZC	bl;
	GLBufferVertexXYZC	br;
	GLBufferVertexXYZC	tl;
	GLBufferVertexXYZC	tr;
	inline int32_t stride() const { return sizeof(bl); }
	inline bool operator==(const GLBufferQuadXYZC & n) const { return (this->bl==n.bl && this->br==n.br && this->tl==n.tl && this->tr==n.tr); }
	inline bool operator!=(const GLBufferQuadXYZC & n) const { return !(*this==(n)); }
};




//	populates the passed VVBufferQuad struct with the passed geometry and texture coords
void GLBufferQuadPopulate(GLBufferQuadXY *b, Rect geoRect);
void GLBufferQuadPopulate(GLBufferQuadXYST *b, Rect geoRect, Rect texRect, bool flippedTex=false);
void GLBufferQuadPopulate(GLBufferQuadXYC *b, Rect geoRect, GLColor geoColor);
void GLBufferQuadPopulate(GLBufferQuadXYZ *b, Rect geoRect);
void GLBufferQuadPopulate(GLBufferQuadXYZST *b, Rect geoRect, Rect texRect, bool flippedTex=false);
void GLBufferQuadPopulate(GLBufferQuadXYZC *b, Rect geoRect, GLColor geoColor);




}


#endif /* VVGeom_h */
