/** $id: iwanj@users.sourceforge.net
*/

#include <eikapp.h>
#include <e32debug.h>
#include "oglescam.h"
#include "uipanel.h"

#include "shader.h"

const TSize KPreviewSize(480, 360);
_LIT(KSavePath, "E:\\DCIM\\");

#ifdef _DEBUG
#define TRACE(s)        RDebug::Printf("kameraku: %s", s);
#define TRACEd(s,d)     RDebug::Printf("kameraku: " s, d);
#else
#define TRACE(s)        
#define TRACEd(s,d)     
#endif

const char* KVertShader = " \
	attribute vec4 vPos; \
	attribute vec4 vUV; \
	uniform mat4 vMat; \
	varying vec2 vCoord; \
	void main(void) \
	{ \
		gl_Position = vMat * vPos; \
		vCoord = vUV.st; \
	}";

COglesCam* COglesCam::NewL(RWindow& aWin, MOglesCamCallback* aCallback)
	{
	COglesCam* self = new(ELeave) COglesCam(aCallback);
	CleanupStack::PushL(self);
	self->ConstructL(aWin);
	CleanupStack::Pop(self);
	return self;
	}

COglesCam::COglesCam(MOglesCamCallback* aCallback):
	CActive(EPriorityStandard),
	iPreviewSize(KPreviewSize),
	iFs(CCoeEnv::Static()->FsSession()),
	iCallback(aCallback)
	{
	CActiveScheduler::Add(this);
	}

COglesCam::~COglesCam()
	{
	delete iCapture;
	delete iRotator;

	Pause();
	if (iCam)
		{
		iCam->PowerOff();
		iCam->Release();
		delete iCam;
		}

	TerminateOgles();
	TerminateEgl();
	}

void COglesCam::ConstructL(RWindow& aWin)
	{
	iLastTime.HomeTime();

	iFs.MkDir(KSavePath);
	iFs.PrivatePath(iHomePath);
	TParsePtrC installDir(CEikonEnv::Static()->EikAppUi()->Application()->AppFullName());
	iHomePath.Insert(0, installDir.Drive());
	
	iCapture = new(ELeave) CFbsBitmap;
	TInt err = iCapture->Create(KPreviewSize, EColor16MU);
	User::LeaveIfError(err);

	iRotator = CBitmapRotator::NewL();
	
	InitEgl(aWin);
	InitOgles();
	
	iCam = CCamera::NewL(*this, 0);
	iCam->Reserve();
	}

void COglesCam::InitEgl(RWindow& aWin)
	{
    TRACE("eglGetDisplay")
	iDisp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (iDisp == EGL_NO_DISPLAY)
		{
		User::Leave(KErrNotSupported);
		}
	
	TRACE("eglInitialize")
	EGLBoolean rc = eglInitialize(iDisp, NULL, NULL);
	if (!rc)
		{
		User::Leave(KErrGeneral);
		}

	EGLint attr[] = {
		EGL_RED_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	EGLint nCfg;
	
	TRACE("eglChooseConfig")
	rc = eglChooseConfig(iDisp, attr, &iCfg, 1, &nCfg);
	if (!rc)
		{
		User::Leave(KErrNotSupported);
		}
	
	TRACE("eglCreateWindowSurface")
	iSurf = eglCreateWindowSurface(iDisp, iCfg, &aWin, NULL);
	if (iSurf == EGL_NO_SURFACE)
		{
		User::Leave(KErrNoMemory);
		}
	
	eglQuerySurface(iDisp, iSurf, EGL_WIDTH, &iSurfSize.iWidth);
	eglQuerySurface(iDisp, iSurf, EGL_HEIGHT, &iSurfSize.iHeight);
	
	EGLint es2[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	
	TRACE("eglCreateContext")
	iCtx = eglCreateContext(iDisp, iCfg, EGL_NO_CONTEXT, es2);
	if (iCtx == EGL_NO_CONTEXT)
		{
		User::Leave(KErrNoMemory);
		}
	
	TRACE("eglMakeCurrent")
	eglMakeCurrent(iDisp, iSurf, iSurf, iCtx);
	}

void COglesCam::TerminateEgl()
	{
	if (iDisp != EGL_NO_DISPLAY)
		{
		eglMakeCurrent(iDisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);		
		if (iCtx != EGL_NO_CONTEXT)
			{
			eglDestroyContext(iDisp, iCtx);
			}
		if (iSurf != EGL_NO_SURFACE)
			{
			eglDestroySurface(iDisp, iDisp);
			}
		eglTerminate(iDisp);
		}
	eglReleaseThread();
	}

void COglesCam::InitOgles()
	{
	const char* KFragShader[KNumProg-1] = {
			KFragShader_natural,
			KFragShader_thermal,
			KFragShader_toon,
			KFragShader_mono,
			KFragShader_night,
			KFragShader_edge,			
			KFragShader_negative
	};
	for (TInt idx=0; idx<KNumProg-1; ++idx)
		{
		CreateProgram(idx, KFragShader[idx]);
		}

	TFileName fn;
	
#ifdef ENABLE_PRO	
	if (LoadConfig(fn))
		{
		LoadCustomProgram(fn);
		}
#endif
	
	ChangeProgram(0);

	fn.SetLength(0);
	fn.Append(iHomePath);
	fn.Append(_L("noise.mbm"));
	CFbsBitmap* b = new(ELeave) CFbsBitmap;
	CleanupStack::PushL(b);
	User::LeaveIfError(b->Load(fn));
	
	glGenTextures(1, &iNoise);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, iNoise);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 128, 128, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, b->DataAddress());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	CleanupStack::PopAndDestroy(b);
	
	glGenTextures(1, &iTex);	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, iTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glViewport(0, 0, iSurfSize.iWidth, iSurfSize.iHeight);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	
	iWu = (GLfloat)iSurfSize.iWidth / 2.0;
	iHu = (GLfloat)iSurfSize.iHeight / 2.0;
	}

void COglesCam::TerminateOgles()
	{
	if (eglGetCurrentContext() != EGL_NO_CONTEXT)
		{
		glDeleteTextures(1, &iNoise);
		glDeleteTextures(1, &iTex);
		for (TInt idx=0; idx<KNumProg; ++idx)
			{
			glDeleteProgram(iProg[idx]);		
			glDeleteShader(iFragShader[idx]);
			}
		glDeleteShader(iVertShader);
		}
	}

TBool COglesCam::CreateProgram(TInt aIdx, const char* aSrc)
	{
	if (iVertShader == 0)
		{
		iVertShader = CreateShader(KVertShader, GL_VERTEX_SHADER);
		TRACEd("CreateVertexShader %d", iVertShader)
		if (iVertShader == 0)
			{
			ERR(_L("Vertex shader compile error"))
			return EFalse;
			}
		}
	
	GLuint fs = CreateShader(aSrc, GL_FRAGMENT_SHADER);
    TRACEd("CreateFragmentShader %d", fs)
	if (fs == 0)
		{
		ERR(_L("Fragment shader compile error"))
		return EFalse;
		}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, iVertShader);
	glAttachShader(prog, fs);

	glBindAttribLocation(prog, 0, "vPos");
	glBindAttribLocation(prog, 1, "vUV");
	
	TRACEd("glLinkProgram %d", prog)
	glLinkProgram(prog);
	GLint ok;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok)
		{
		GLint len = 0;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
		if (len > 1)
			{
			char* info = new char[len];
			if (info)
				{
				glGetProgramInfoLog(prog, len, NULL, info);
				TRACE(info);

				delete[] info;
				}
			}
		TRACEd("glDeleteProgram: %d", prog)
		glDeleteProgram(prog);
		
		TRACEd("glDeleteShader: %d", fs)
		glDeleteShader(fs);
		
		ERR(_L("GL program link error"))
		return EFalse;
		}
	
	iProg[aIdx] = prog;
	iFragShader[aIdx] = fs;
	return ETrue;
	}

GLuint COglesCam::CreateShader(const char* aSrc, GLenum aType)
	{
	GLuint s = glCreateShader(aType);
	glShaderSource(s, 1, &aSrc, NULL);
	glCompileShader(s);
	GLint ok;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok)
		{
		GLint len = 0;
		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
		if (len > 1)
			{
			char* info = new char[len];
			if (info)
				{
				glGetShaderInfoLog(s, len, NULL, info);
				TRACE(info);

				TBuf<256> msg;
				msg.Copy(TPtr8((TUint8*)info, len));
				INFO(msg)
				
				delete[] info;
				}
			}
		TRACEd("glDeleteShader: %d", s)
		glDeleteShader(s);
		s = 0;
		}

	return s;
	}

void COglesCam::ChangeProgram(TInt aIdx)
	{
	if ((aIdx == KNumProg-1) && (iProg[KNumProg-1]==0))
		{
		return;
		}

	TRACEd("glUseProgram %d", iProg[aIdx])
	glUseProgram(iProg[aIdx]);
	
	glUniform1i(glGetUniformLocation(iProg[aIdx], "vTex"), 0);
	glUniform1i(glGetUniformLocation(iProg[aIdx], "vNoise"), 1);

	const GLfloat KMat[] = {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};
	GLint loc = glGetUniformLocation(iProg[aIdx], "vMat");	
	glUniformMatrix4fv(loc, 1, GL_FALSE, KMat);
	}

TBool COglesCam::LoadCustomProgram(const TDesC& aFile)
	{
	const TInt custId = KNumProg - 1;
	if (iProg[custId] !=0)
		{
		TRACEd("glDeleteProgram: %d", iProg[custId])
		glDeleteProgram(iProg[custId]);
		
		TRACEd("glDeleteShader: %d", iFragShader[custId])
		glDeleteShader(iFragShader[custId]);
		
		iProg[custId] = 0;
		iFragShader[custId] = 0;
		}
	
	TBuf<256> msg;
	RFile file;
	TInt err = file.Open(iFs, aFile, EFileRead);
	if (err != KErrNone)
		{
		return EFalse;
		}
	
	TInt bytes;
	file.Size(bytes);
	char* buf = (char*)User::Alloc(bytes + 1);
	if (!buf)
		{
		file.Close();
		return EFalse;
		}
	
	TPtr8 ptr((TUint8*)buf, bytes);
	err = file.Read(ptr);
	if (err != KErrNone)
		{
		User::Free(buf);
		file.Close();
		return EFalse;
		}
	buf[ptr.Length()] = '\0';
	if (!CreateProgram(custId, buf))
		{
		User::Free(buf);
		file.Close();
		return EFalse;
		}
	
	ChangeProgram(custId);
	
	User::Free(buf);
	file.Close();
	
	return ETrue;
	}

void COglesCam::DrawTex(TInt x, TInt y, TInt w, TInt h)
	{
	++iFrameCount;
	
	TTime now;
	now.HomeTime();
	if (now.MicroSecondsFrom(iLastTime) >= 100000)
		{
		GLint prog;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		GLint loc = glGetUniformLocation(prog, "vTime");
		GLfloat num = (GLfloat) iFrameCount;
		glUniform1f(loc, num);
		
		iLastTime = now;
		}
	
	const GLfloat gx = -1.0 + (GLfloat) x / iWu;
	const GLfloat gy = -1.0 + (GLfloat)(iSurfSize.iHeight - y) / iHu;
	const GLfloat gw = (GLfloat)w / iWu;
	const GLfloat gh = (GLfloat)h / iHu;

	const GLfloat KVertices[] = {
		gx+gw, gy-gh, 0.0,
		gx+gw, gy, 0.0,
		gx, gy-gh, 0.0,
		gx, gy, 0.0
	};
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, KVertices);
	glEnableVertexAttribArray(0);

	const GLfloat KTexCoord[] = {
		1.0, 1.0,
		1.0, 0.0,
		0.0, 1.0,
		0.0, 0.0
	};
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, KTexCoord);
	glEnableVertexAttribArray(1);

	const GLshort KIndices[] = {0, 1, 2, 3};
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, KIndices);
	}

 void COglesCam::ReserveComplete(TInt aError)
	{
	if (aError != KErrNone)
		{
	    TRACEd("ReserveComplete err: %d", aError)
	    ERR(_L("Unable to acquire camera device"))
	    User::LeaveIfError(aError);
		}
	 
	iCam->PowerOn();
	}
 
 void COglesCam::PowerOnComplete(TInt aError)
	{
	if (aError != KErrNone)
		{
	    TRACEd("PowerOnComplete err: %d", aError)
	    ERR(_L("Unable to switch on camera device"))
		return;
		}

	TRAPD(err, iCam->StartViewFinderBitmapsL(iPreviewSize));
	}

 void COglesCam::PostProcessFrame(CFbsBitmap& aFrame)
	{
	const TSize size = aFrame.SizeInPixels();
	const TDisplayMode mode= aFrame.DisplayMode();
	
	switch (mode)
		{
		case EColor64K:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.iWidth, size.iHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, aFrame.DataAddress());
			break;
		case EColor16M:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.iWidth, size.iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, aFrame.DataAddress());
			break;
		case EColor16MU:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.iWidth, size.iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, aFrame.DataAddress());
			break;
		default:
			ERR(_L("Camera format not supported"))
		}
	
	DrawTex(0, 0, KPreviewSize.iWidth, KPreviewSize.iHeight);
	}
 
 void COglesCam::ViewFinderFrameReady(CFbsBitmap& aFrame)
	{
	PostProcessFrame(aFrame);
	if (iCaptureImage)
		{
		iCam->StopViewFinder();
		CaptureImage();
		}

	eglSwapBuffers(iDisp, iSurf);
	}
 
 void COglesCam::ImageReady(CFbsBitmap* aBitmap, HBufC8* aData, TInt aError)
	{
	}
 
 void COglesCam::FrameBufferReady(MFrameBuffer* aFrameBuffer,TInt aError)
	{
	}

 void COglesCam::CaptureImage()
	{
	iCaptureImage = EFalse;
	glReadPixels(0, 0, iPreviewSize.iWidth, iPreviewSize.iHeight, GL_RGBA, GL_UNSIGNED_BYTE, iCapture->DataAddress());
	
	for (TInt i=0; i<15; ++i)
		{
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		eglSwapBuffers(iDisp, iSurf);
		}
	
	iRotator->Rotate(&iStatus, *iCapture, CBitmapRotator::EMirrorHorizontalAxis);
	iState = ERotating;
	SetActive();
	}
 
 void COglesCam::Action(TInt aAction)
	{
	if (aAction == KShutter)
		{
		iCaptureImage = ETrue;
		}
	else if (aAction == KDelete)
		{
		iFs.Delete(iCaptureFile);
		}
	}
 
 void COglesCam::Pause()
	{
	Cancel();
	if (iCam && iCam->ViewFinderActive())
		{
		iCam->StopViewFinder();
		}
	}
 
 void COglesCam::Resume()
	{
	if (iCam && !iCam->ViewFinderActive())
		{
		TRAPD(err, iCam->StartViewFinderBitmapsL(iPreviewSize));
		}
	}
 
 void COglesCam::RunL()
	{
	switch (iState)
		{
		case ERotating:
			RotatingComplete();
			break;
			
		case EConverting:
			ConvertingComplete();
			break;
		}
	}
 
 void COglesCam::DoCancel()
	{
	if (iState == ERotating)
		{
		iRotator->Cancel();
		}
	else if (iState == EConverting)
		{
		iEncoder->Cancel();
		}
	}
 
void COglesCam::RotatingComplete()
	{
	// swap BGRA to ARGB
	TUint32* p = iCapture->DataAddress();	
	for (TInt y=0; y<iPreviewSize.iHeight; ++y)
		{
		for (TInt x=0; x<iPreviewSize.iWidth; ++x)
			{
			const TInt c = *p;
			const TInt r = (c & 0xff) << 16;
			const TInt g = (c & 0xff00);
			const TInt b = (c & 0xff0000) >> 16;
			*p++ = 0xff000000 | r | g | b; 
			}
		}
	
	TTime t;
	t.HomeTime();
	TBuf<32> s;
	TRAPD(err, t.FormatL(s, _L("kameraku_%1%2%3_%H%T%S")));
	iCaptureFile.SetLength(0);
	iCaptureFile.AppendFormat(_L("%S%S.jpg"), &KSavePath, &s);
	
	TRAP(err, iEncoder = CImageEncoder::FileNewL(iFs, iCaptureFile, _L8("image/jpeg")));
	if (err == KErrNone)
		{
		iEncoder->Convert(&iStatus, *iCapture);
		iState = EConverting;
		SetActive();
		}
	else
		{
		iState = EIdle;
		
		TBuf<64> msg;
		msg.AppendFormat(_L("Unable to capture image err: %d"), err);
		ERR(msg)
		
		TRAPD(err, iCam->StartViewFinderBitmapsL(iPreviewSize));
		}
	}

void COglesCam::ConvertingComplete()
	{
	iState = EIdle;
	
	delete iEncoder;
	iEncoder = NULL;
	
	iCallback->ImageReady(iCapture);
	TRAPD(err, iCam->StartViewFinderBitmapsL(iPreviewSize));
	}

void COglesCam::SaveConfig(const TFileName& aFx)
	{
	TFileName cfg;
	cfg.Append(iHomePath);
	cfg.Append(_L("kameraku.cfg"));
	
	RFile file;
	TInt err = file.Replace(iFs, cfg, EFileWrite);
	if (err != KErrNone)
		{
		file.Close();
		return;
		}
	
	TBuf8<256> buf;
	buf.Copy(aFx);
	file.Write(buf);
	file.Close();
	}

TBool COglesCam::LoadConfig(TFileName& aFx)
	{
	TFileName cfg;
	cfg.Append(iHomePath);
	cfg.Append(_L("kameraku.cfg"));
	
	RFile file;
	TInt err = file.Open(iFs, cfg, EFileRead);
	if (err != KErrNone)
		{
		return EFalse;
		}
	
	TBuf8<256> buf;
	file.Read(buf);
	if (buf.Length() != 0)
		{
		aFx.Copy(buf);
		}
	file.Close();
	return ETrue;
	}
