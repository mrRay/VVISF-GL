#ifndef VVGL_GLGeom_hpp
#define VVGL_GLGeom_hpp

#include "VVGL_Defines.hpp"

#include <iostream>
#include "VVGL_Base.hpp"

#define BUFFER_OFFSET(i) ((uint8_t*)NULL + (i))




namespace VVGL
{


using namespace std;



/*!
\brief Basic struct for 2D point
\ingroup VVGL_GEOM
*/
//	basic geometry structs
struct Point	{
	double		x = 0.;
	double		y = 0.;
	
	Point() {};
	Point(const double & inX, const double & inY) { x=inX;y=inY; };
	
	inline bool isZero() const { return (this->x==0.0 && this->y==0.0); }
	
	inline bool operator==(const Point & n) const { return (n.x==this->x && n.y==this->y); }
	inline bool operator!=(const Point & n) const { return !(*this==(n)); }
	inline Point operator+(const Point & n) const { return { this->x + n.x, this->y + n.y }; }
	inline Point operator-(const Point & n) const { return { this->x - n.x, this->y - n.y }; }
	friend ostream & operator<<(ostream & os, const Point & n) { os<<"("<<n.x<<", "<<n.y<<")";return os; }
};
/*!
\brief Basic struct for 2D size
\ingroup VVGL_GEOM
*/
struct Size	{
	double		width = 0.;
	double		height = 0.;
	
	Size() {};
	Size(const double & inWidth, const double & inHeight) { width=inWidth;height=inHeight; };
	
	inline bool isZero() const { return (this->width==0.0 && this->height==0.0); }
	
	inline bool operator==(const Size & n) const { return (n.width==this->width && n.height==this->height); }
	inline bool operator!=(const Size & n) const { return !(*this==(n)); }
	inline Size operator+(const Size & n) const { return { this->width + n.width, this->height + n.height }; }
	inline Size operator-(const Size & n) const { return { this->width - n.width, this->height - n.height }; }
	friend ostream & operator<<(ostream & os, const Size & n) { os<<"("<<n.width<<"x"<<n.height<<")";return os; }
};
/*!
\brief Basic struct for a rectangle using VVGL::Point and VVGL::Size
\ingroup VVGL_GEOM
*/
struct Rect	{
	Point	origin = {0.,0.};
	Size	size = {0., 0.};
	
	Rect() {};
	Rect(const double & inX, const double & inY, const double & inW, const double & inH) { origin={inX,inY}; size={inW,inH}; };
	
	//!	Returns the min X coordinate of the rect.
	inline double minX() const { return ((size.width>=0) ? (origin.x) : (origin.x+size.width)); }
	//!	Returns the max X coordinate of the rect.
	inline double maxX() const { return ((size.width>=0) ? (origin.x+size.width) : (origin.x)); }
	//!	Returns the min Y coordinate of the rect.
	inline double minY() const { return ((size.height>=0) ? (origin.y) : (origin.y+size.height)); }
	//!	Returns the max Y coordinate of the rect.
	inline double maxY() const { return ((size.height>=0) ? (origin.y+size.height) : (origin.y)); }
	//!	Returns the middle X coordinate of the rect.
	inline double midX() const { return (origin.x+(size.width/2.0)); }
	//!	Returns the middle Y coordinate of the rect.
	inline double midY() const { return (origin.y+(size.height/2.0)); }
	//!	Returns a Point describing the top-left corner of the rect.
	inline Point topLeft() const { return { this->minX(), this->maxY() }; }
	//!	Returns a Point describing the top-right corner of the rect.
	inline Point topRight() const { return { this->maxX(), this->maxY() }; }
	//!	Returns a Point describing the bottom-left corner of the rect.
	inline Point botLeft() const { return { this->minX(), this->minY() }; }
	//!	Returns a Point describing the bottom-right corner of the rect.
	inline Point botRight() const { return { this->maxX(), this->minY() }; }
	//!	Returns a Point describing the center of the rect.
	inline Point center() const { return { this->midX(), this->midY() }; }
	//!	Sets all the values of the rect to 0.
	inline bool isZero() const { return (this->origin.isZero() && this->size.isZero()); }
	
	inline bool operator==(const Rect & n) const { return (this->origin.operator==(n.origin) && this->size.operator==(n.size)); }
	inline bool operator!=(const Rect & n) const { return !(*this==(n)); }
	friend ostream & operator<<(ostream & os, const Rect & n) { os<<"{"<<n.origin<<","<<n.size<<"}";return os; }
};




/*!
sizing modes used for the 'ResizeRect' function
\ingroup VVGL_GEOM
*/
enum SizingMode	{
	SizingMode_Fit,	//!<	the content is made as large as possible, proportionally, without cutting itself off or going outside the bounds of the desired area
	SizingMode_Fill,	//!<	the content is made as large as possible, proportionally, to fill the desired area- some of the content may get cut off
	SizingMode_Stretch,	//!<	the content is scaled to fit perfectly within the desired area- some stretching or squashing may occur, this isn't necessarily proportional
	SizingMode_Copy	//!<	the content is copied directly to the desired area- it is not made any larger or smaller
};
/*!
\brief Proportional resizing of Rects
This method resizes one rect to fit in another using the provided sizing mode.  Useful for calculating sizes while preserving aspect ratios.
\ingroup VVGL_GEOM
*/
VVGL_EXPORT Rect ResizeRect(const Rect & fitThisRect, const Rect & inThisRect, const SizingMode & sizingMode);




/*!
\ingroup VVGL_GEOM
\brief Abstract base struct for vertex types (VT == "vertex type")
*/
struct VT	{
	//virtual void zero() = 0;
};
/*!
\ingroup VVGL_GEOM
\brief This vertex type describes a two-dimensional point.  Derives from VT.  Concrete subclasses of Vertex use one or more of these to store their data.
*/
struct VT_XY : public VT	{
	float		x = 0.;
	float		y = 0.;
	
	VT_XY() {}
	VT_XY(const float & inX, const float & inY) : x(inX), y(inY) {}
	
	inline float & operator[] (const int index) { switch(index) { case 0: return x; case 1: default: return y; } }
	inline bool operator== (const VT_XY & n) const { return (this->x==n.x && this->y==n.y); }
	inline bool operator!= (const VT_XY & n) const { return !(*this==n); }
	friend ostream & operator<<(ostream & os, const VT_XY & n) { os<<"{"<<n.x<<","<<n.y<<"}";return os; }
	
	inline void zero() { x=0.; y=0; }
	inline int numComponents() const { return 2; }
};
/*!
\ingroup VVGL_GEOM
\brief This vertex type describes a three-dimensional point.  Derives from VT.  Concrete subclasses of Vertex use one or more of these to store their data.
*/
struct VT_XYZ : public VT	{
	float		x = 0.;
	float		y = 0.;
	float		z = 0.;
	
	VT_XYZ() {}
	VT_XYZ(const float & inX, const float & inY, const float & inZ) : x(inX), y(inY), z(inZ) {}
	
	inline float & operator[] (const int index) { switch(index) { case 0: return x; case 1: return y; case 2: default: return z; } }
	inline bool operator== (const VT_XYZ & n) const { return (this->x==n.x && this->y==n.y && this->z==n.z); }
	inline bool operator!= (const VT_XYZ & n) const { return !(*this==n); }
	friend ostream & operator<<(ostream & os, const VT_XYZ & n) { os<<"{"<<n.x<<","<<n.y<<","<<n.z<<"}";return os; }
	
	inline void zero() { x=0.; y=0; z=0; }
	inline int numComponents() const { return 3; }
};
/*!
\ingroup VVGL_GEOM
\brief This vertex type describes a two-dimensional texture location.  Derives from VT.  Concrete subclasses of Vertex use one or more of these to store their data.
*/
struct VT_ST : public VT	{
	float		s = 0.;
	float		t = 0.;
	
	VT_ST() {}
	VT_ST(const float & inS, const float & inT) : s(inS), t(inT) {}
	
	inline float & operator[] (const int index) { switch(index) { case 0: return s; case 1: default: return t; } }
	inline bool operator== (const VT_ST & n) const { return (this->s==n.s && this->t==n.t); }
	inline bool operator!= (const VT_ST & n) const { return !(*this==n); }
	friend ostream & operator<<(ostream & os, const VT_ST & n) { os<<"{"<<n.s<<","<<n.t<<"}";return os; }
	
	inline void zero() { s=0.; t=0; }
	inline int numComponents() const { return 2; }
};
/*!
\ingroup VVGL_GEOM
\brief This vertex type describes RGBA color data.  Derives from VT.  Concrete subclasses of Vertex use one or more of these to store their data.
*/
struct VT_RGBA : public VT	{
	float		r = 0.0;
	float		g = 0.0;
	float		b = 0.0;
	float		a = 0.0;
	
	VT_RGBA() {}
	VT_RGBA(const float & inR, const float & inG, const float & inB, const float & inA) : r(inR), g(inG), b(inB), a(inA) {}
	
	inline float & operator[] (const int index) { switch(index) { case 0: return r; case 1: return g; case 2: return b; case 3: default: return a; } }
	inline void operator= (const GLColor & n) { r=n.r; g=n.g; b=n.b; a=n.a; };
	inline bool operator== (const VT_RGBA & n) const { return (this->r==n.r && this->g==n.g && this->b==n.b && this->a==n.a); };
	inline bool operator!= (const VT_RGBA & n) const { return !(*this==n); };
	friend ostream & operator<<(ostream & os, const VT_RGBA & n) { os<<"{"<<n.r<<","<<n.g<<","<<n.b<<","<<n.a<<"}";return os; }
	
	inline void zero() { r=0.; g=0; b=0; a=0; }
	inline int numComponents() const { return 4; }
};




//! Abstract base struct for a vertex that has one or kinds of data stored in one or more vertex types
/*!
\ingroup VVGL_GEOM
Structs derived from VVGL::Vertex describe a single vertex and the various kinds of data associated with it (geometry, color, texture coords, and various combinations thereof).  Typically, these derived structs are passed to the Quad template (Quad consists of four vertices and the template is used to determine what kind of data is contained in each vertex)
*/
struct Vertex	{
	//!	Zeros out any data contained by the vertex.
	//virtual void zero() = 0;
};
//	these structs define different commonly-used kinds of vertices using the above data types.  the ordering and alignment of the members in these structs should correspond directly to how the memory is used in GL (likewise, it is expected that the members of all these structs are compatible with GL).
/*!
\ingroup VVGL_GEOM
\brief Describes a vertex that only contains x/y geometry data.  Derived from Vertex.
*/
struct VertXY : public Vertex	{
	//!	The geometry data for this vertex.
	VT_XY		geo;
	
	inline bool operator== (const VertXY & n) const { return (this->geo==n.geo); }
	inline bool operator!= (const VertXY & n) const { return !(*this==n); }
	
	//!	Returns the offset of the geometry data within the vertex.
	inline int geoOffset() const { return 0; }
	
	inline void zero() { geo.zero(); }
};
/*!
\ingroup VVGL_GEOM
\brief Describes a vertex that contains x/y geometry data and s/t texture coordinate data.  Derived from Vertex.
*/
struct VertXYST : public Vertex	{
	//!	The geometry data for this vertex.
	VT_XY		geo;
	//!	The texture coordinate data for this vertex.
	VT_ST		tex;
	
	inline bool operator== (const VertXYST & n) const { return (this->geo==n.geo && this->tex==n.tex); }
	inline bool operator!= (const VertXYST & n) const { return !(*this==n); }
	
	//!	Returns the offset of the geometry data within the vertex.
	inline int geoOffset() const { return 0; }
	//!	Returns the offset of the texture data within the vertex.
	inline int texOffset() const { return sizeof(geo); }
	
	inline void zero() { geo.zero(); tex.zero(); }
};
/*!
\ingroup VVGL_GEOM
\brief Describes a vertex that contains x/y geometry data and RGBA color data.  Derived from Vertex.
*/
struct VertXYRGBA : public Vertex	{
	//!	The geometry data for this vertex.
	VT_XY		geo;
	//!	The RGBA color data for this vertex.
	VT_RGBA		color;
	
	inline bool operator== (const VertXYRGBA & n) const { return (this->geo==n.geo && this->color==n.color); }
	inline bool operator!= (const VertXYRGBA & n) const { return !(*this==n); }
	
	//!	Returns the offset of the geometry data within the vertex.
	inline int geoOffset() const { return 0; }
	//!	Returns the offset of the color data within the vertex.
	inline int colorOffset() const { return sizeof(geo); }
	
	inline void zero() { geo.zero(); color.zero(); }
};
/*!
\ingroup VVGL_GEOM
\brief Describes a vertex that contains x/y geometry data and s/t texture coordinate data and RGBA color data.  Derived from Vertex.
*/
struct VertXYSTRGBA : public Vertex	{
	//!	The geometry data for this vertex.
	VT_XY		geo;
	//!	The texture coordinate data for this vertex.
	VT_ST		tex;
	//!	The RGBA color data for this vertex.
	VT_RGBA		color;
	
	inline bool operator== (const VertXYSTRGBA & n) const { return (this->geo==n.geo && this->tex==n.tex && this->color==n.color); }
	inline bool operator!= (const VertXYSTRGBA & n) const { return !(*this==n); }
	
	//!	Returns the offset of the geometry data within the vertex.
	inline int geoOffset() const { return 0; }
	//!	Returns the offset of the texture data within the vertex.
	inline int texOffset() const { return sizeof(geo); }
	//!	Returns the offset of the color data within the vertex.
	inline int colorOffset() const { return (sizeof(geo) + sizeof(tex)); }
	
	inline void zero() { geo.zero(); tex.zero(); color.zero(); }
};


/*!
\ingroup VVGL_GEOM
\brief Describes a vertex that contains x/y/z geometry data.  Derived from Vertex.
*/
struct VertXYZ : public Vertex	{
	//!	The geometry data for this vertex.
	VT_XYZ		geo;
	
	inline bool operator== (const VertXYZ & n) const { return (this->geo==n.geo); }
	inline bool operator!= (const VertXYZ & n) const { return !(*this==n); }
	
	//!	Returns the offset of the geometry data within the vertex.
	inline int geoOffset() const { return 0; }
	
	inline void zero() { geo.zero(); }
};
/*!
\ingroup VVGL_GEOM
\brief Describes a vertex that contains x/y/z geometry data and s/t texture coordinate data.  Derived from Vertex.
*/
struct VertXYZST : public Vertex	{
	//!	The geometry data for this vertex.
	VT_XYZ		geo;
	//!	The texture coordinate data for this vertex.
	VT_ST		tex;
	
	inline bool operator== (const VertXYZST & n) const { return (this->geo==n.geo && this->tex==n.tex); }
	inline bool operator!= (const VertXYZST & n) const { return !(*this==n); }
	
	//!	Returns the offset of the geometry data within the vertex.
	inline int geoOffset() const { return 0; }
	//!	Returns the offset of the texture data within the vertex.
	inline int texOffset() const { return sizeof(geo); }
	
	inline void zero() { geo.zero(); tex.zero(); }
};
/*!
\ingroup VVGL_GEOM
\brief Describes a vertex that contains x/y/z geometry data and RGBA color data.  Derived from Vertex.
*/
struct VertXYZRGBA : public Vertex	{
	//!	The geometry data for this vertex.
	VT_XYZ		geo;
	//!	The RGBA color data for this vertex.
	VT_RGBA		color;
	
	inline bool operator== (const VertXYZRGBA & n) const { return (this->geo==n.geo && this->color==n.color); }
	inline bool operator!= (const VertXYZRGBA & n) const { return !(*this==n); }
	
	//!	Returns the offset of the geometry data within the vertex.
	inline int geoOffset() const { return 0; }
	//!	Returns the offset of the color data within the vertex.
	inline int colorOffset() const { return sizeof(geo); }
	
	inline void zero() { geo.zero(); color.zero(); }
};
/*!
\ingroup VVGL_GEOM
\brief Describes a vertex that contains x/y/z geometry data and s/t texture coordinate data and RGBA color data.  Derived from Vertex.
*/
struct VertXYZSTRGBA : public Vertex	{
	//!	The geometry data for this vertex.
	VT_XYZ		geo;
	//!	The texture coordinate data for this vertex.
	VT_ST		tex;
	//!	The RGBA color data for this vertex.
	VT_RGBA		color;
	
	inline bool operator== (const VertXYZSTRGBA & n) const { return (this->geo==n.geo && this->tex==n.tex && this->color==n.color); }
	inline bool operator!= (const VertXYZSTRGBA & n) const { return !(*this==n); }
	
	//!	Returns the offset of the geometry data within the vertex.
	inline int geoOffset() const { return 0; }
	//!	Returns the offset of the texture data within the vertex.
	inline int texOffset() const { return sizeof(geo); }
	//!	Returns the offset of the color data within the vertex.
	inline int colorOffset() const { return (sizeof(geo) + sizeof(tex)); }
	
	inline void zero() { geo.zero(); tex.zero(); color.zero(); }
};
/*!
\ingroup VVGL_GEOM
\brief Describes a vertex that contains RGBA color data.  Derived from Vertex.
*/
struct VertRGBA : public Vertex	{
	//!	The RGBA color data for this vertex.
	VT_RGBA		color;
	
	inline bool operator== (const VertRGBA & n) const { return (this->color==n.color); }
	inline bool operator!= (const VertRGBA & n) const { return !(*this==n); }
	
	//!	Returns the offset of the color data within the vertex.
	inline int colorOffset() const { return 0; }
	
	inline void zero() { color.zero(); }
};




/*!
\ingroup VVGL_GEOM
\brief This struct defines a quad- it's a simple template which is expected to be passed one of the structs derived from Vertex as the template type (this template type defines what kind of data the quad will contain).  The instance methods make populating the quad with geometry/texture/color data easy, and the stride and offset methods simplify uploading the struct's memory directly to GL.
*/
template <typename QuadType>
struct Quad	{
public:
	//!	The bottom-left vertex
	QuadType		bl;
	//!	The bottom-right vertex
	QuadType		br;
	//!	The top-left vertex
	QuadType		tl;
	//!	The top-right vertex
	QuadType		tr;
	
	//	make sure that the template type derives from VVGL::Vertex
	static_assert(std::is_base_of<VVGL::Vertex,QuadType>::value, "QuadType must inherit from VVGL::Vertex");
	
	Quad() {}
	Quad(const QuadType & inBL, const QuadType & inBR, const QuadType & inTL, const QuadType & inTR) : bl(inBL), br(inBR), tl(inTL), tr(inTR) {}
	
	inline QuadType & operator[] (const int index) { switch(index) { case 0: return br; case 1: return br; case 2: return tl; case 3: default: return tr; } }
	inline bool operator==(const Quad & n) const { return (this->bl==n.bl && this->br==n.br && this->tl==n.tl && this->tr==n.tr); }
	inline bool operator!=(const Quad & n) const { return !(*this==n); }
	
	//!	Returns the size of a single vertex in bytes/the stride between vertices.  If you know the stride of a vertex you can upload vertex data directly to GL.
	inline int stride() const { return sizeof(bl); }
	//!	Returns the offset of the geometry data within the quad.
	inline int geoOffset() const { return bl.geoOffset(); }
	//!	Returns the offset of the texture coordinate data within the quad.
	inline int texOffset() const { return bl.texOffset(); }
	//!	Returns the offset of the RGBA color data within the quad.
	inline int colorOffset() const { return bl.colorOffset(); }
	
	//!	Populates the quad's geometry with the passed rect's geometry
	inline void populateGeo(Rect & inRect)	{
		const Rect		tmpRect = inRect;
		populateGeo(tmpRect);
	}
	//!	Populates the quad's geometry with the passed rect's geometry
	inline void populateGeo(const Rect & inRect)	{
		bl.geo.x = static_cast<float>(inRect.minX());
		bl.geo.y = static_cast<float>(inRect.minY());

		br.geo.x = static_cast<float>(inRect.maxX());
		br.geo.y = static_cast<float>(inRect.minY());

		tl.geo.x = static_cast<float>(inRect.minX());
		tl.geo.y = static_cast<float>(inRect.maxY());

		tr.geo.x = static_cast<float>(inRect.maxX());
		tr.geo.y = static_cast<float>(inRect.maxY());
	}
	//!	Populates the quad's texture coordinate data with the passed rect's geometry.
	inline void populateTex(struct Rect & inRect, const bool & inFlip)	{
		const Rect		tmpRect = inRect;
		populateTex(tmpRect, inFlip);
	}
	//!	Populates the quad's texture coordinate data with the passed rect's geometry.
	inline void populateTex(const Rect & inRect, const bool & inFlip)	{
		if (!inFlip)	{
			bl.tex.s = static_cast<float>(inRect.minX());
			bl.tex.t = static_cast<float>(inRect.minY());

			br.tex.s = static_cast<float>(inRect.maxX());
			br.tex.t = static_cast<float>(inRect.minY());

			tl.tex.s = static_cast<float>(inRect.minX());
			tl.tex.t = static_cast<float>(inRect.maxY());

			tr.tex.s = static_cast<float>(inRect.maxX());
			tr.tex.t = static_cast<float>(inRect.maxY());
		}
		else	{
			bl.tex.s = static_cast<float>(inRect.minX());
			bl.tex.t = static_cast<float>(inRect.maxY());

			br.tex.s = static_cast<float>(inRect.maxX());
			br.tex.t = static_cast<float>(inRect.maxY());

			tl.tex.s = static_cast<float>(inRect.minX());
			tl.tex.t = static_cast<float>(inRect.minY());

			tr.tex.s = static_cast<float>(inRect.maxX());
			tr.tex.t = static_cast<float>(inRect.minY());
		}
	}
	//!	Populates the quad's RGBA color data with the passed color.
	inline void populateColor(VT_RGBA & inColor)	{
		const VT_RGBA		tmpColor = inColor;
		populateColor(tmpColor);
	}
	//!	Populates the quad's RGBA color data with the passed color.
	inline void populateColor(const VT_RGBA & inColor)	{
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


#endif /* VVGL_GLGeom_hpp */
