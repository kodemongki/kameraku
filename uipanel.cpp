/** $id: iwanj@users.sourceforge.net
*/

#include <eikappui.h>
#include <eikapp.h>
#include <eikenv.h>
#include "uipanel.h"

_LIT(KBlobFile, "blob.mbm");
_LIT(KAppTitle, "kameraku 1.0");
_LIT(KAuthor, "kodemongki.mbm");

CBlob::CBlob(const TPoint& aPos):
	iPos(aPos),
	iEnabled(EFalse)
	{
	}

void CBlob::Draw(CWindowGc& aGc)
	{
	aGc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	aGc.SetPenStyle(CGraphicsContext::ENullPen);	
	aGc.SetBrushColor(TRgb(128,128,128,128));
	aGc.DrawRoundRect(TRect(iPos, KBlobSize), TSize(8,8));
	}

void CBlob::SetEnabled(TBool aEnable)
	{
	iEnabled = aEnable;
	}

TBool CBlob::Hit(const TPoint& aPos)
	{
	if (!iEnabled)
		{
		return EFalse;
		}
	
	if (aPos.iX >= iPos.iX && aPos.iY >= iPos.iY && 
		(aPos.iX <= iPos.iX + KBlobSize.iWidth) &&
		(aPos.iY <= iPos.iY + KBlobSize.iHeight))
		{
		return ETrue;
		}
	
	return EFalse;
	}

CBitmapBlob::CBitmapBlob(const TPoint& aPos, CFbsBitmap* aIcon):
	CBlob(aPos),
	iFocused(EFalse),
	iIcon(aIcon)
	{
	}

void CBitmapBlob::SetFocused(TBool aFocus)
	{
	iFocused = aFocus;
	}

void CBitmapBlob::Draw(CWindowGc& aGc)
	{
	CBlob::Draw(aGc);
	if (iIcon)
		{
		aGc.DrawBitmap(TRect(iPos + TPoint(5,5), TSize(54,54)), iIcon);
		}
	if (!iEnabled)
		{
		aGc.SetBrushColor(TRgb(32,32,32,200));
		aGc.DrawRoundRect(TRect(iPos, KBlobSize), TSize(8,8));
		return;
		}
	if (iFocused)
		{
		aGc.SetPenStyle(CGraphicsContext::ESolidPen);
		aGc.SetPenColor(KRgbWhite);
		aGc.SetBrushStyle(CGraphicsContext::ENullBrush);
		aGc.DrawRoundRect(TRect(iPos, KBlobSize), TSize(8,8));
		}
	}

CUiPanel* CUiPanel::NewL(CWsScreenDevice* aScr)
	{
	CUiPanel* self = new(ELeave) CUiPanel(aScr);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	
	return self;
	}

CUiPanel::CUiPanel(CWsScreenDevice* aScr):
	iScr(aScr),
	iGc(CCoeEnv::Static()->SystemGc())
	{
	}

void CUiPanel::ConstructL()
	{
	iSize = iScr->SizeInPixels();
	TFontSpec fspec(_L("Sans"), 16);
	TInt err = iScr->GetNearestFontToDesignHeightInPixels(iFont, fspec);
	User::LeaveIfError(err);

	RFs& fs = CCoeEnv::Static()->FsSession();
	TFileName path;
	fs.PrivatePath(path);
	
	TParsePtrC inst(CEikonEnv::Static()->EikAppUi()->Application()->AppFullName());
	path.Insert(0, inst.Drive());
	TFileName fn;
	fn.Append(path);
	fn.Append(KBlobFile);
	
	for (TInt id=0; id<KNumShaderBlob/2; ++id)
		{
		const TPoint pos = TPoint(iSize.iWidth - KBlobSize.iWidth - 10, 2 + id * (KBlobSize.iHeight + 2));
		CFbsBitmap* icon = new (ELeave) CFbsBitmap;
		CleanupStack::PushL(icon);
		err = icon->Load(fn, id);
		User::LeaveIfError(err);
		
		iShaderBlob[id] = new(ELeave) CBitmapBlob(pos, icon);
		CleanupStack::Pop(icon);
		}
	
	for (TInt id=KNumShaderBlob/2; id<KNumShaderBlob; ++id)
		{
		const TPoint pos = TPoint(iSize.iWidth - 2 * KBlobSize.iWidth - 10 - 2, 2 + (id - KNumShaderBlob/2) * (KBlobSize.iHeight + 2));
		CFbsBitmap* icon = new (ELeave) CFbsBitmap;
		CleanupStack::PushL(icon);
		err = icon->Load(fn, id);
		User::LeaveIfError(err);

		iShaderBlob[id] = new(ELeave) CBitmapBlob(pos, icon);
		CleanupStack::Pop(icon);
		}
	
	for (TInt id=0; id<KNumShaderBlob; ++id)
		{
		iShaderBlob[id]->SetEnabled(ETrue);
		}

	iFocus = 0;
	iShaderBlob[iFocus]->SetFocused(ETrue);

	fn.SetLength(0);
	fn.Append(path);
	fn.Append(KButtonFile);
	
	const TPoint pos1 = TPoint(iSize.iWidth - KBlobSize.iWidth - 10, iSize.iHeight - KBlobSize.iHeight - 2);
	CFbsBitmap* icon = new (ELeave) CFbsBitmap;
	CleanupStack::PushL(icon);
	err = icon->Load(fn, KShutter);
	User::LeaveIfError(err);
	iControl[KShutter] = new(ELeave) CBitmapBlob(pos1, icon);
	CleanupStack::Pop(icon);
	iControl[KShutter]->SetEnabled(ETrue);
	
	const TPoint pos2 = TPoint(iSize.iWidth - 2 * KBlobSize.iWidth - 10 - 2, iSize.iHeight - KBlobSize.iHeight - 2);	
	icon = new (ELeave) CFbsBitmap;
	CleanupStack::PushL(icon);
	err = icon->Load(fn, KDelete);
	User::LeaveIfError(err);
	iControl[KDelete] = new(ELeave) CBitmapBlob(pos2, icon);
	CleanupStack::Pop(icon);
	iControl[KDelete]->SetEnabled(ETrue);
	
	icon = new (ELeave) CFbsBitmap;
	CleanupStack::PushL(icon);
	err = icon->Load(fn, KBack);
	User::LeaveIfError(err);
	iControl[KBack] = new(ELeave) CBitmapBlob(pos1, icon);
	CleanupStack::Pop(icon);
	iControl[KBack]->SetEnabled(ETrue);

	icon = new (ELeave) CFbsBitmap;
	CleanupStack::PushL(icon);
	err = icon->Load(fn, KExit);
	User::LeaveIfError(err);
	iControl[KExit] = new(ELeave) CBitmapBlob(pos2, icon);
	CleanupStack::Pop(icon);
	iControl[KExit]->SetEnabled(ETrue);
	
	fn.SetLength(0);
	fn.Append(path);
	fn.Append(KAuthor);
	
	iIcon = new(ELeave) CFbsBitmap;
	err = iIcon->Load(fn);
	User::LeaveIfError(err);
	
	iMode = EViewfinder;
	}

CUiPanel::~CUiPanel()
	{
	delete iIcon;

	for (TInt id=0; id<KNumShaderBlob; ++id)
		{
		delete iShaderBlob[id];
		}
	for (TInt id=0; id<3; ++id)
		{
		delete iControl[id];
		}
	if (iFont)
		{
		iScr->ReleaseFont(iFont);
		}
	}

void CUiPanel::Draw()
	{
	// clear ui panel background
	iGc.SetBrushStyle(CGraphicsContext::ESolidBrush);
	iGc.SetPenStyle(CGraphicsContext::ENullPen);
	iGc.SetBrushColor(KRgbBlack);
	iGc.DrawRect(TRect(TPoint(480,0), TSize(iSize.iWidth - 480,360)));
	
	for (TInt id=0; id<KNumShaderBlob; ++id)
		{
		iShaderBlob[id]->Draw(iGc);
		}

	if (iMode == EViewfinder)
		{
		iControl[KShutter]->Draw(iGc);
		iControl[KExit]->Draw(iGc);
		}
	else if (iMode == EImagePreview)
		{
		if (iImageReady)
			{
			iGc.BitBlt(TPoint(), iImage);
			}
		iControl[KDelete]->Draw(iGc);
		iControl[KBack]->Draw(iGc);
		}
	
	// draw title
	iGc.UseFont(iFont);
	iGc.SetPenColor(KRgbWhite);
	iGc.SetPenStyle(CGraphicsContext::ESolidPen);
	iGc.DrawText(KAppTitle, TPoint(10,20));
	iGc.DiscardFont();
	
	// draw title
	iGc.BitBlt(TPoint(10, iSize.iHeight - 68), iIcon);
	}

TBool CUiPanel::HitBlob(const TPoint& aPos, TInt& aIdx)
	{
	for (TInt id=0; id<KNumShaderBlob; ++id)
		{
		if (iShaderBlob[id]->Hit(aPos))
			{
			iShaderBlob[iFocus]->SetFocused(EFalse);
			iShaderBlob[id]->SetFocused(ETrue);
			aIdx = iFocus = id;
			return ETrue;
			}
		}
		
	return EFalse;
	}

TBool CUiPanel::HitMongki(const TPoint& aPos)
	{
	if (aPos.iX >= 10 && aPos.iY >= iSize.iHeight - 68 && 
		(aPos.iX <= 10 + 64) &&
		(aPos.iY <= iSize.iHeight - 4))
		{
		return ETrue;
		}
	
	return EFalse;
	}

TBool CUiPanel::OverBlob(const TPoint& aPos)
	{
	return iShaderBlob[KNumShaderBlob-1]->Hit(aPos);
	}

TBool CUiPanel::HitControl(const TPoint& aPos, TInt& aWhich)
	{
	if (iMode == EViewfinder)
		{
		if (iControl[KShutter]->Hit(aPos))
			{
			aWhich = KShutter;
			iMode = EImagePreview;
			for (TInt id=0; id<KNumShaderBlob; ++id)
				{
				iShaderBlob[id]->SetEnabled(EFalse);
				}
			return ETrue;
			}
		else if (iControl[KExit]->Hit(aPos))
			{
			aWhich = KExit;
			return ETrue;
			}
		}
	else if (iMode == EImagePreview)
		{
			if (iControl[KDelete]->Hit(aPos))
				{
				aWhich = KDelete;
				return ETrue;
				}
			else if (iControl[KBack]->Hit(aPos))
				{
				aWhich = KBack;
				iMode = EViewfinder;
				iImageReady = EFalse;
				for (TInt id=0; id<KNumShaderBlob; ++id)
					{
					iShaderBlob[id]->SetEnabled(ETrue);
					}
				return ETrue;
				}
		}
	
	return EFalse;
	}

void CUiPanel::SetImage(CFbsBitmap* aBitmap)
	{
	iImage = aBitmap;
	iImageReady = ETrue;
	}

void CUiPanel::SetMode(TInt aMode)
	{
	if (aMode == EViewfinder)
		{
		iMode = EViewfinder;
		iImageReady = EFalse;
		for (TInt id=0; id<KNumShaderBlob; ++id)
			{
			iShaderBlob[id]->SetEnabled(ETrue);
			}
		}
	}
