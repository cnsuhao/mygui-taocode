/*!
	@file
	@author		Albert Semenov
	@date		06/2009
*/
/*
	This file is part of MyGUI.

	MyGUI is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MyGUI is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with MyGUI.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __MYGUI_FONT_DATA_STREAM_H__
#define __MYGUI_FONT_DATA_STREAM_H__

#include "MyGUI_Prerequest.h"

#include <ft2build.h>

namespace MyGUI
{
    struct FontDataImpl
    {
        FT_Library      mFTLib;

        FontDataImpl()
        {
            FT_Init_FreeType(&mFTLib);
        }

        ~FontDataImpl()
        {
            FT_Done_FreeType(mFTLib);
        }
    };

    struct FontDataStream
    {
        IDataStream*	mDataStream;
        size_t			mBegin;
        size_t			mEnd;
        FT_StreamRec	mStreamRec;
    };

} // namespace MyGUI

#endif // __MYGUI_FONT_DATA_STREAM_H__
