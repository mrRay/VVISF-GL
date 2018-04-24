#ifndef VVGL_Geom_h
#define VVGL_Geom_h

#include "VVGL_Defines.hpp"

#include <iostream>
#include "VVGL_Base.hpp"

#define BUFFER_OFFSET(i) ((uint8_t*)NULL + (i))




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




//	these structs define basic complex data types for vertices.  VT is short for "vertex type".
struct VT_XY	{
	float		x = 0.;
	float		y = 0.;
	
	VT_XY() {}
	VT_XY(const float & inX, const float & inY) : x(inX), y(inY) {}
	
	inline float & operator[] (const int index) { switch(index) { case 0: return x; case 1: default: return y; } }
	inline bool operator== (const VT_XY & n) const { return (this->x==n.x && this->y==n.y); }
	inline bool operator!= (const VT_XY & n) const { return !(*this==n); }
	
	inline void zero() { x=0.; y=0; }
};
struct VT_XYZ	{
	float		x = 0.;
	float		y = 0.;
	float		z = 0.;
	
	VT_XYZ() {}
	VT_XYZ(const float & inX, const float & inY, const float & inZ) : x(inX), y(inY), z(inZ) {}
	
	inline float & operator[] (const int index) { switch(index) { case 0: return x; case 1: return y; case 2: default: return z; } }
	inline bool operator== (const VT_XYZ & n) const { return (this->x==n.x && this->y==n.y && this->z==n.z); }
	inline bool operator!= (const VT_XYZ & n) const { return !(*this==n); }
	
	inline void zero() { x=0.; y=0; z=0; }
};
//	this struct defines a basic two-dimensional texture data type (ST)
struct VT_ST	{
	float		s = 0.;
	float		t = 0.;
	
	VT_ST() {}
	VT_ST(const float & inS, const float & inT) : s(inS), t(inT) {}
	
	inline float & operator[] (const int index) { switch(index) { case 0: return s; case 1: default: return t; } }
	inline bool operator== (const VT_ST & n) const { return (this->s==n.s && this->t==n.t); }
	inline bool operator!= (const VT_ST & n) const { return !(*this==n); }
	
	inline void zero() { s=0.; t=0; }
};
//	this struct defines an RGBA color data type
struct VT_RGBA	{
	float		r = 0.0;
	float		g = 0.0;
	float		b = 0.0;
	float		a = 0.0;
	
	VT_RGBA() {}
	VT_RGBA(const float & inR, const float & inG, const float & inB, const float & inA) : r(inR), g(inG), b(inB), a(inA) {}
	
	inline float & operator[] (const int index) { switch(index) { case 0: return r; case 1: return g; case 2: return b; case 3: default: return a; } }
	inline bool operator== (const VT_RGBA & n) const { return (this->r==n.r && this->g==n.g && this->b==n.b && this->a==n.a); };
	inline bool operator!= (const VT_RGBA & n) const { return !(*this==n); };
	
	inline void zero() { r=0.; g=0; b=0; a=0; }
};


//	these structs define different commonly-used kinds of vertices using the above data types
struct VertXY	{
	VT_XY		geo;
	
	inline bool operator== (const VertXY & n) const { return (this->geo==n.geo); }
	inline bool operator!= (const VertXY & n) const { return !(*this==n); }
	
	inline int geoOffset() const { return 0; }
	inline void zero() { geo.zero(); }
};
struct VertXYST	{
	VT_XY		geo;
	VT_ST		tex;
	
	inline bool operator== (const VertXYST & n) const { return (this->geo==n.geo && this->tex==n.tex); }
	inline bool operator!= (const VertXYST & n) const { return !(*this==n); }
	
	inline int geoOffset() const { return 0; }
	inline int texOffset() const { return sizeof(geo); }
	inline void zero() { geo.zero(); tex.zero(); }
};
struct VertXYRGBA	{
	VT_XY		geo;
	VT_RGBA		color;
	
	inline bool operator== (const VertXYRGBA & n) const { return (this->geo==n.geo && this->color==n.color); }
	inline bool operator!= (const VertXYRGBA & n) const { return !(*this==n); }
	
	inline int geoOffset() const { return 0; }
	inline int colorOffset() const { return sizeof(geo); }
	inline void zero() { geo.zero(); color.zero(); }
};
struct VertXYSTRGBA	{
	VT_XY		geo;
	VT_ST		tex;
	VT_RGBA		color;
	
	inline bool operator== (const VertXYSTRGBA & n) const { return (this->geo==n.geo && this->tex==n.tex && this->color==n.color); }
	inline bool operator!= (const VertXYSTRGBA & n) const { return !(*this==n); }
	
	inline int geoOffset() const { return 0; }
	inline int texOffset() const { return sizeof(geo); }
	inline int colorOffset() const { return (sizeof(geo) + sizeof(tex)); }
	inline void zero() { geo.zero(); tex.zero(); color.zero(); }
};


struct VertXYZ	{
	VT_XYZ		geo;
	
	inline bool operator== (const VertXYZ & n) const { return (this->geo==n.geo); }
	inline bool operator!= (const VertXYZ & n) const { return !(*this==n); }
	
	inline int geoOffset() const { return 0; }
	inline void zero() { geo.zero(); }
};
struct VertXYZST	{
	VT_XYZ		geo;
	VT_ST		tex;
	
	inline bool operator== (const VertXYZST & n) const { return (this->geo==n.geo && this->tex==n.tex); }
	inline bool operator!= (const VertXYZST & n) const { return !(*this==n); }
	
	inline int geoOffset() const { return 0; }
	inline int texOffset() const { return sizeof(geo); }
	inline void zero() { geo.zero(); tex.zero(); }
};
struct VertXYZRGBA	{
	VT_XYZ		geo;
	VT_RGBA		color;
	
	inline bool operator== (const VertXYZRGBA & n) const { return (this->geo==n.geo && this->color==n.color); }
	inline bool operator!= (const VertXYZRGBA & n) const { return !(*this==n); }
	
	inline int geoOffset() const { return 0; }
	inline int colorOffset() const { return sizeof(geo); }
	inline void zero() { geo.zero(); color.zero(); }
};
struct VertXYZSTRGBA	{
	VT_XYZ		geo;
	VT_ST		tex;
	VT_RGBA		color;
	
	inline bool operator== (const VertXYZSTRGBA & n) const { return (this->geo==n.geo && this->tex==n.tex && this->color==n.color); }
	inline bool operator!= (const VertXYZSTRGBA & n) const { return !(*this==n); }
	
	inline int geoOffset() const { return 0; }
	inline int texOffset() const { return sizeof(geo); }
	inline int colorOffset() const { return (sizeof(geo) + sizeof(tex)); }
	inline void zero() { geo.zero(); tex.zero(); color.zero(); }
};
struct VertRGBA	{
	VT_RGBA		color;
	
	inline bool operator== (const VertRGBA & n) const { return (this->color==n.color); }
	inline bool operator!= (const VertRGBA & n) const { return !(*this==n); }
	
	inline int colorOffset() const { return 0; }
	inline void zero() { color.zero(); }
};


//	this struct defines a quad- it's a template which is expected to be passed one of the above Vert_* struct types
template <typename QuadType>
struct Quad	{
	QuadType		bl;
	QuadType		br;
	QuadType		tl;
	QuadType		tr;
	
	Quad() {}
	Quad(const QuadType & inBL, const QuadType & inBR, const QuadType & inTL, const QuadType & inTR) : bl(inBL), br(inBR), tl(inTL), tr(inTR) {}
	
	inline QuadType & operator[] (const int index) { switch(index) { case 0: return br; case 1: return br; case 2: return tl; case 3: default: return tr; } }
	inline bool operator==(const Quad & n) const { return (this->bl==n.bl && this->br==n.br && this->tl==n.tl && this->tr==n.tr); }
	inline bool operator!=(const Quad & n) const { return !(*this==n); }
	
	inline int stride() const { return sizeof(bl); }
	inline int geoOffset() const { return bl.geoOffset(); }
	inline int texOffset() const { return bl.texOffset(); }
	inline int colorOffset() const { return bl.colorOffset(); }
	
	void populateGeo(Rect & inRect)	{
		const Rect		tmpRect = inRect;
		populateGeo(tmpRect);
	}
	void populateGeo(const Rect & inRect)	{
		bl.geo.x = MinX(inRect);
		bl.geo.y = MinY(inRect);
	
		br.geo.x = MaxX(inRect);
		br.geo.y = MinY(inRect);
	
		tl.geo.x = MinX(inRect);
		tl.geo.y = MaxY(inRect);
	
		tr.geo.x = MaxX(inRect);
		tr.geo.y = MaxY(inRect);
	}
	void populateTex(Rect & inRect, const bool & inFlip=false)	{
		const Rect		tmpRect = inRect;
		populateTex(tmpRect, inFlip);
	}
	void populateTex(const Rect & inRect, const bool & inFlip=false)	{
		if (!inFlip)	{
			bl.tex.s = MinX(inRect);
			bl.tex.t = MinY(inRect);
	
			br.tex.s = MaxX(inRect);
			br.tex.t = MinY(inRect);
	
			tl.tex.s = MinX(inRect);
			tl.tex.t = MaxY(inRect);
	
			tr.tex.s = MaxX(inRect);
			tr.tex.t = MaxY(inRect);
		}
		else	{
			bl.tex.s = MinX(inRect);
			bl.tex.t = MaxY(inRect);
	
			br.tex.s = MaxX(inRect);
			br.tex.t = MaxY(inRect);
	
			tl.tex.s = MinX(inRect);
			tl.tex.t = MinY(inRect);
	
			tr.tex.s = MaxX(inRect);
			tr.tex.t = MinY(inRect);
		}
	}
	void populateColor(VT_RGBA & inColor)	{
		const VT_RGBA		tmpColor = inColor;
		populateColor(tmpColor);
	}
	void populateColor(const VT_RGBA & inColor)	{
		VT_RGBA		tmpColor = inColor;
		for (int i=0; i<4; ++i)	{
			bl.color[i] = tmpColor[i];
			br.color[i] = tmpColor[i];
			tl.color[i] = tmpColor[i];
			tr.color[i] = tmpColor[i];
		}
	}
};




}


#endif /* VVGL_Geom_h */
