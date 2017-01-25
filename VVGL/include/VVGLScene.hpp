#ifndef VVGLScene_hpp
#define VVGLScene_hpp

#include <functional>
#include <mutex>
#include "VVGLBufferPool.hpp"

#if ISF_TARGET_MAC
#import <TargetConditionals.h>
#endif




namespace VVGL	{


using namespace std;


class VVGLScene;




class VVGLScene	{
	public:
		//	this defines the interface for declaring a lambda as a member variable for encapsulating drawing code
		using RenderCallback = std::function<void(const VVGLScene &)>;
		
		//	RenderTarget has all the attachments for the GL framebuffer, which need to be tracked so we can bind/unbind appropriately
		struct RenderTarget	{
				VVGLBufferRef	fbo = nullptr;
				VVGLBufferRef	color = nullptr;
				VVGLBufferRef	depth = nullptr;
			public:
				RenderTarget(){};
				RenderTarget(const VVGLBufferRef &f, const VVGLBufferRef &c, const VVGLBufferRef &d) { fbo=f;color=c;depth=d; };
		
				inline uint32_t fboName() const { return (fbo==nullptr) ? 0 : fbo->name; };
				inline uint32_t colorName() const { return (color==nullptr) ? 0 : color->name; };
				inline uint32_t colorTarget() const { return (color==nullptr) ? GL_TEXTURE_2D : color->desc.target; };
				inline uint32_t depthName() const { return (depth==nullptr) ? 0 : depth->name; };
				inline uint32_t depthTarget() const { return (depth==nullptr) ? GL_TEXTURE_2D : depth->desc.target; };
		};
	
	
	
	
	//	variables
	protected:
		recursive_mutex		renderLock;
		VVGLContext			*context = nullptr;
		
		bool				deleted = false;
		bool				initialized = false;
		bool				needsReshape = true;
		bool				alwaysNeedsReshape = false;
		
		RenderCallback		renderCallback = nullptr;	//	every time the scene renders, this lambda is executed.  drawing code goes here.
		RenderTarget		renderTarget;	//	the render target contains the GL framebuffer and relevant attachments (render to texture/buffer/depth buffer/etc)
		
		Size				size = { 1., 1. };
		bool				flipped = false;
		
		mutex				projMatrixLock;
		float				projMatrix[16];	//	modern GL requires the modelview/projection matrices to be expressed as a single matrix that gets passed to the vertex shader.  this is that matrix- it configures orthogonal projection.  stored here in column-major format.
#if ISF_TARGET_IOS
		void				*projMatrixEffect = nil;	//	really a GLKBaseEffect*
#endif
		
		bool				performClear = true;
		GLColor				clearColor = GLColor(0., 0., 0., 0.);
		bool				clearColorUpdated = false;
		
	
	//	methods
	public:
		//	creates a new GL context that shares the global buffer pool's context
		VVGLScene();
		//	uses the passed context- doesn't create any new GL contexts (just retains what was passed to it)
		VVGLScene(const VVGLContext * inCtx);
		
		//VVGLScene(const VVGLContext * inSharedCtx, const Size & inSize={640.,480.});
		virtual ~VVGLScene();
		
		virtual void prepareToBeDeleted();
		
		virtual VVGLBufferRef createAndRenderABuffer(const Size & inSize=Size(640.,480.), const VVGLBufferPoolRef & inPool=GetGlobalBufferPool());
		virtual void renderToBuffer(const VVGLBufferRef & inBuffer);
		virtual void render(const RenderTarget & inRenderTarget=RenderTarget());
		virtual void renderBlackFrame(const RenderTarget & inRenderTarget=RenderTarget());
		virtual void renderOpaqueBlackFrame(const RenderTarget & inRenderTarget=RenderTarget());
		virtual void renderRedFrame(const RenderTarget & inRenderTarget=RenderTarget());
		
		void setContext(const VVGLContext & inCtx);
		inline VVGLContext * getContext() const { return context; }
		
		void setRenderCallback(const RenderCallback & n);
		
		void setAlwaysNeedsReshape(const bool & n);
		virtual void setSize(const Size & n);
		Size getSize() const;
		void setFlipped(const bool & n);
		bool getFlipped() const;
		
		void setClearColor(const GLColor & n);
		void setClearColor(const float & r, const float & g, const float & b, const float & a);
		void setClearColor(float * n);
		void setPerformClear(const bool & n);
		
	protected:
		virtual void _renderPrep();
		virtual void _initialize();
		virtual void _reshape();
		virtual void _renderCleanup();
		
	private:
#if ISF_TARGET_IOS
		void _configProjMatrixEffect();	//	acquire 'projMatrixLock' before calling
#endif
};




}


#endif /* VVGLScene_hpp */
