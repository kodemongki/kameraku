/** $id: iwanj@users.sourceforge.net
*/
#ifndef KWIKAPP_H
#define KWIKAPP_H

#include <aknapp.h>
#include <akndoc.h>
#include <aknappui.h>
#include <coecntrl.h>
#include <aknlongtapdetector.h> 
#include "callback.h"

////////////////////////////////////////////////////////////
// App
class CKwikApp: public CAknApplication
    {
public:
    TUid AppDllUid() const;

protected:
    CApaDocument* CreateDocumentL();
    };

////////////////////////////////////////////////////////////
// Doc
class CKwikAppDoc: public CAknDocument
    {
public:
    CKwikAppDoc(CEikApplication& aApp);
    CEikAppUi* CreateAppUiL();
    };

////////////////////////////////////////////////////////////
// Ui
class MAppExitCallback
	{
public:
	virtual void OnExit() = 0;
	};

class CKwikAppView;
class CKwikAppUi: public CAknAppUi, public MAppExitCallback
    {
public:
    void ConstructL();
    virtual ~CKwikAppUi();
    void OnExit();
    
private:
    void HandleCommandL(TInt aCommand);
    void HandleForegroundEventL(TBool aForeground);
    
private:
    CKwikAppView* iAppView;
    };

////////////////////////////////////////////////////////////
// View
class COglesCam;
class CUiPanel;
class CKwikAppView: public CCoeControl, public MAknLongTapDetectorCallBack, public MOglesCamCallback
    {
    friend class COglesCam;
public:
    static CKwikAppView* NewL(const TRect& aRect, MAppExitCallback* aCallback);
    void Draw(const TRect& aRect) const;
	~CKwikAppView();
	void HandlePointerEventL(const TPointerEvent& aEvent);
	void HandleLongTapEventL( const TPoint& aRelPos, const TPoint& aAbsPos);
	void BroughtToForeground();
	void BroughtToBackground();
	
	void ImageReady(CFbsBitmap* aBitmap);
	
private:
	CKwikAppView(MAppExitCallback* aCallback);
    void ConstructL(const TRect& aRect);
    void VisitMongkiL();
    
private:
    COglesCam* iOglesCam;
    CUiPanel* iUiPanel;
    CAknLongTapDetector* iLongTap;
    MAppExitCallback* iCallback;
    };

#endif
