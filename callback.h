/** $id: iwanj@users.sourceforge.net
*/

#ifndef CALLBACK_H_
#define CALLBACK_H_

#include <fbs.h>

class MOglesCamCallback
	{
public:
	virtual void ImageReady(CFbsBitmap* aBitmap) = 0;
	};

#endif
