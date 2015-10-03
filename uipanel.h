/** $id: iwanj@users.sourceforge.net
*/

#ifndef UIPANEL_H_
#define UIPANEL_H_

#include <e32base.h>
#include <w32std.h>

const TInt KNumShaderBlob = 8;
const TSize KBlobSize = TSize(64,64);
_LIT(KButtonFile, "button.mbm");

const TInt KShutter = 0;
const TInt KDelete = 1;
const TInt KBack = 2;
const TInt KExit = 3;

enum TMode
	{
	EViewfinder,
	EImagePreview
	};

class CBlob: public CBase
	{
public:
	CBlob(const TPoint& aPos);
	virtual TBool Hit(const TPoint& aPos);
	virtual void Draw(CWindowGc& aGc);
	void SetEnabled(TBool aEnable);	
	
protected:
	TPoint iPos;
	TBool iEnabled;	
	};

class CBitmapBlob: public CBlob
	{
public:
	CBitmapBlob(const TPoint& aPos, CFbsBitmap* aIcon);
	void Draw(CWindowGc& aGc);
	void SetFocused(TBool aFocus);

private:
	TBool iFocused;
	CFbsBitmap* iIcon;	
	};

class CUiPanel: public CBase
	{
public:
	static CUiPanel* NewL(CWsScreenDevice* aScr);
	~CUiPanel();
	
	void Draw();
	TBool OverBlob(const TPoint& aPos);
	TBool HitBlob(const TPoint& aPos, TInt& aWhich);
	TBool HitControl(const TPoint& aPos, TInt& aWhich);
	TBool HitMongki(const TPoint& aPos);
	void SetImage(CFbsBitmap* aBitmap);
	void SetMode(TInt aMode);
	
private:
	CUiPanel(CWsScreenDevice* aScr);
	void ConstructL();
	
private:
	TSize iSize;
	CWsScreenDevice* iScr;
	CWindowGc& iGc;
	CFont* iFont;

	TInt iFocus;
	CBitmapBlob* iShaderBlob[KNumShaderBlob];
	CBitmapBlob* iControl[4];
	TInt iMode;
	
	CFbsBitmap* iImage;
	TBool iImageReady;
	
	CFbsBitmap* iIcon;
	};

#endif /* UIPANEL_H_ */
