/** $id: iwanj@users.sourceforge.net
*/
#ifndef OGLESCAM_H_
#define OGLESCAM_H_

#include <e32base.h>
#include <w32std.h>
#include <ecam.h>
#include <imageconversion.h>
#include <bitmaptransforms.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <aknnotewrappers.h>
#include "callback.h"

#define INFO(msg) \
		{ \
		CAknInformationNote* popup = new(ELeave) CAknInformationNote(&popup); \
        popup->ExecuteLD(msg); \
		}
#define ERR(msg) \
		{ \
		CAknErrorNote* popup = new(ELeave) CAknErrorNote(&popup); \
        popup->ExecuteLD(msg); \
		}

const TInt KNumProg = 8;

class COglesCam: public CActive, MCameraObserver
	{
public:
	static COglesCam* NewL(RWindow& aWin, MOglesCamCallback* aCallback);
	~COglesCam();

	// from MCameraObserver
	virtual void ReserveComplete(TInt aError);
	virtual void PowerOnComplete(TInt aError);
	virtual void ViewFinderFrameReady(CFbsBitmap& aFrame);
	virtual void ImageReady(CFbsBitmap* aBitmap,HBufC8* aData,TInt aError);
	virtual void FrameBufferReady(MFrameBuffer* aFrameBuffer,TInt aError);
	
	void ChangeProgram(TInt aIdx);
	TBool LoadCustomProgram(const TDesC& aFile);
	TInt CustomProgram() const {return iProg[KNumProg-1];}
	void SaveConfig(const TFileName& fn);
	TBool LoadConfig(TFileName& fn);
	
	void Action(TInt aAction);
	void Pause();
	void Resume();
	
protected:
	void RunL();
	void DoCancel();
	
private:
	COglesCam(MOglesCamCallback* aCallback);
	void ConstructL(RWindow& aWin);
	
	void InitEgl(RWindow& aWin);
	void TerminateEgl();
	void InitOgles();
	void TerminateOgles();
	
	TBool CreateProgram(TInt aIdx, const char* aSrc);
	GLuint CreateShader(const char* aSrc, GLenum aType);
	void PostProcessFrame(CFbsBitmap& aFrame);
	void DrawTex(TInt x, TInt y, TInt w, TInt h);
	void CaptureImage();
	void RotatingComplete();
	void ConvertingComplete();
	
private:
	enum
		{
		EIdle,
		ERotating,
		EConverting
		};
	
	CCamera* iCam;
	TSize iPreviewSize;
	
	EGLDisplay iDisp;
	EGLSurface iSurf;
	EGLContext iCtx;
	EGLConfig iCfg;
	TSize iSurfSize;
	
	RFs& iFs;
	TFileName iHomePath;
	
	GLfloat iWu;
	GLfloat iHu;
	GLuint iProg[KNumProg];
	GLuint iVertShader;
	GLuint iFragShader[KNumProg];
	GLuint iTex;
	GLuint iNoise;
	
	TTime iLastTime;
	TInt iFrameCount;
	
	CFbsBitmap* iCapture;
	CImageEncoder* iEncoder;
	TBool iCaptureImage;
	CBitmapRotator* iRotator;
	TFileName iCaptureFile;
	
	TInt iState;
	MOglesCamCallback* iCallback;
	};

#endif /* OGLESCAM_H_ */
