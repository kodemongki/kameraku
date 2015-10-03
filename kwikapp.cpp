/** $id: iwanj@users.sourceforge.net
*/
#include <AknCommonDialogs.h>
#include <CAknFileSelectionDialog.h>
#include <AknGlobalConfirmationQuery.h>
#include <eikstart.h>
#include <coemain.h>
#include "kwikapp.h"
#include "oglescam.h"
#include "uipanel.h"
#include <kameraku.rsg>
#include <apgcli.h>

const TUid KUidKwikApp = {0x2004505E};

////////////////////////////////////////////////////////////
// App
CApaDocument* CKwikApp::CreateDocumentL()
    {
    return new(ELeave) CKwikAppDoc(*this);
    }

TUid CKwikApp::AppDllUid() const
    {
    return KUidKwikApp;
    }

////////////////////////////////////////////////////////////
// Doc
CKwikAppDoc::CKwikAppDoc(CEikApplication& aApp)
    :CAknDocument(aApp)
    {
    }

CEikAppUi* CKwikAppDoc::CreateAppUiL()
    {
    return new(ELeave) CKwikAppUi;
    }

////////////////////////////////////////////////////////////
// Ui
void CKwikAppUi::ConstructL()
    {
    BaseConstructL();
    SetOrientationL(CAknAppUi::EAppUiOrientationLandscape);    
    iAppView = CKwikAppView::NewL(ClientRect(), this);
    }

CKwikAppUi::~CKwikAppUi()
    {
    if (iAppView)
        {
        delete iAppView;
        iAppView = NULL;
        }
    }

void CKwikAppUi::HandleCommandL(TInt aCommand)
    {
    switch( aCommand )
        {
        case EEikCmdExit:
            Exit();
            break;
        default:
            break;
        }
    }

void CKwikAppUi::HandleForegroundEventL(TBool aForeground)
	{
	if (aForeground)
		{
		iAppView->BroughtToForeground();
		}
	else
		{
		iAppView->BroughtToBackground();
		}
	}

void CKwikAppUi::OnExit()
	{
	Exit();
	}

////////////////////////////////////////////////////////////
// View
CKwikAppView* CKwikAppView::NewL(const TRect& aRect, MAppExitCallback* aCallback)
    {
    CKwikAppView* self = new(ELeave) CKwikAppView(aCallback);
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    CleanupStack::Pop(self);
    return self;
    }

CKwikAppView::CKwikAppView(MAppExitCallback* aCallback):
	iCallback(aCallback)
	{
	}

CKwikAppView::~CKwikAppView()
    {
	delete iLongTap;
	delete iOglesCam;
	delete iUiPanel;
    }

void CKwikAppView::ConstructL(const TRect& /*aRect*/)
    {
    CreateWindowL();
    SetExtentToWholeScreen();
    Window().SetBackgroundColor();
    ActivateL();

    iOglesCam = COglesCam::NewL(Window(), this);    
    iUiPanel = CUiPanel::NewL(CCoeEnv::Static()->ScreenDevice());
    iLongTap = CAknLongTapDetector::NewL(this);
    }

void CKwikAppView::Draw(const TRect& /*aRect*/) const
    {
	if (iUiPanel)
		{
		iUiPanel->Draw();
		}
    }

void CKwikAppView::HandleLongTapEventL( const TPoint& aRelPos, const TPoint& aAbsPos)
	{
#ifdef ENABLE_PRO	
	TFileName fn;
	_LIT(KDlgTitle, "Select custom shader file:");
	if (AknCommonDialogs::RunSelectDlgLD(fn, R_FILE_SELECT_DIALOG, KDlgTitle))
		{
		TBool ok = iOglesCam->LoadCustomProgram(fn);
		if (ok)
			{
			// remember for the next time
			iOglesCam->SaveConfig(fn);
			}
		}
#endif	
	}

void CKwikAppView::HandlePointerEventL(const TPointerEvent& aEvent)
	{
#ifdef ENABLE_PRO	
	if (iUiPanel->OverBlob(aEvent.iPosition))
		{
		iLongTap->PointerEventL(aEvent);
		}
#endif
	
	if (aEvent.iType == TPointerEvent::EButton1Down)
		{
		TInt which;
		if (iUiPanel->HitBlob(aEvent.iPosition, which))
			{
			DrawNow();
			if (which >= 0 && which < KNumProg-1)
				{
				iOglesCam->ChangeProgram(which);
				}
			else if (which == KNumProg-1)
				{
#ifdef ENABLE_PRO
				if (iOglesCam->CustomProgram() == 0)
					{
					INFO(_L("Press and hold this button to set your custom shader effect"))
					}
				else
					{
					iOglesCam->ChangeProgram(which);
					}
#else				
				INFO(_L("Custom shader effect only available in paid version"))
#endif						
				}

			}
		else if (iUiPanel->HitControl(aEvent.iPosition, which))
			{
			DrawNow();
			if (which == KShutter)
				{
				iOglesCam->Action(KShutter);
				}
			else if (which == KDelete)
				{
				CAknGlobalConfirmationQuery* q = CAknGlobalConfirmationQuery::NewL();
				CleanupStack::PushL(q);
				TRequestStatus s;
				q->ShowConfirmationQueryL(s, _L("Delete image?"), R_AVKON_SOFTKEYS_OK_CANCEL);
				User::WaitForRequest(s);
				CleanupStack::PopAndDestroy(q);
				if (s.Int() == EAknSoftkeyOk)
					{
					iOglesCam->Action(KDelete);
					iUiPanel->SetMode(EViewfinder);
					DrawNow();
					}
				}
			else if (which == KBack)
				{
				INFO(_L("Image saved to Media Gallery"))
				}
			else if (which == KExit)
				{
				CAknGlobalConfirmationQuery* q = CAknGlobalConfirmationQuery::NewL();
				CleanupStack::PushL(q);
				TRequestStatus s;
				q->ShowConfirmationQueryL(s, _L("Exit application?"), R_AVKON_SOFTKEYS_OK_CANCEL);
				User::WaitForRequest(s);
				CleanupStack::PopAndDestroy(q);
				if (s.Int() == EAknSoftkeyOk)
					{
					iCallback->OnExit();
					}
				}
			}
		else if (iUiPanel->HitMongki(aEvent.iPosition))
			{
			VisitMongkiL();
			}
		}
	}

void CKwikAppView::BroughtToForeground()
	{
	iOglesCam->Resume();
	}

void CKwikAppView::BroughtToBackground()
	{
	iOglesCam->Pause();
	}

void CKwikAppView::ImageReady(CFbsBitmap* aBitmap)
	{
	iUiPanel->SetImage(aBitmap);
	DrawNow();
	}

void CKwikAppView::VisitMongkiL()
	{
	RApaLsSession session;
	User::LeaveIfError(session.Connect());
	CleanupClosePushL(session);
	 
	TUid uid;
	TDataType dataType(_L8("text/html"));
	session.AppForDataType(dataType,uid);
	 
	TThreadId threadId;
	User::LeaveIfError(session.StartDocument(_L("http://kodemongki.blogspot.com"), dataType, threadId));
	 
	CleanupStack::PopAndDestroy();
	}

////////////////////////////////////////////////////////////
// Main
LOCAL_C CApaApplication* NewApplication()
    {
    return new CKwikApp;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }

