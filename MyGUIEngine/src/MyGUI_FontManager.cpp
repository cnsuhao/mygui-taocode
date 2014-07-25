/*!
	@file
	@author		Albert Semenov
	@date		11/2007
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
#include "MyGUI_Precompiled.h"
#include "MyGUI_FontManager.h"
#include "MyGUI_FactoryManager.h"
#include "MyGUI_XmlDocument.h"

#include "MyGUI_ResourceManualFont.h"
#include "MyGUI_ResourceTrueTypeFont.h"

#include "MyGUI_FontDataStream.h"
#include "MyGUI_DataStream.h"
#include "MyGUI_DataManager.h"

namespace MyGUI
{

	template <> FontManager* Singleton<FontManager>::msInstance = nullptr;
	template <> const char* Singleton<FontManager>::mClassTypeName = "FontManager";

	FontManager::FontManager() :
		mIsInitialise(false),
		mXmlFontTagName("Font"),
		mXmlPropertyTagName("Property"),
		mXmlDefaultFontValue("Default"),
        mFontImpl(nullptr)
	{
        mFontImpl = new FontDataImpl;
	}

    FontManager::~FontManager()
    {
        delete mFontImpl;

        FontDataStreamRegistry::iterator itr;
        for (itr = mFontData.begin(); itr != mFontData.end(); ++itr)
        {
            delete itr->second;
            itr->second = nullptr;
        }

        mFontData.clear();
    }

	void FontManager::initialise()
	{
		MYGUI_ASSERT(!mIsInitialise, getClassTypeName() << " initialised twice");
		MYGUI_LOG(Info, "* Initialise: " << getClassTypeName());

		ResourceManager::getInstance().registerLoadXmlDelegate(mXmlFontTagName) = newDelegate(this, &FontManager::_load);

		std::string resourceCategory = ResourceManager::getInstance().getCategoryName();
		FactoryManager::getInstance().registerFactory<ResourceManualFont>(resourceCategory);
		FactoryManager::getInstance().registerFactory<ResourceTrueTypeFont>(resourceCategory);

		mDefaultName = "Default";

		MYGUI_LOG(Info, getClassTypeName() << " successfully initialized");
		mIsInitialise = true;
	}

	void FontManager::shutdown()
	{
		MYGUI_ASSERT(mIsInitialise, getClassTypeName() << " is not initialised");
		MYGUI_LOG(Info, "* Shutdown: " << getClassTypeName());

		MyGUI::ResourceManager::getInstance().unregisterLoadXmlDelegate(mXmlFontTagName);

		std::string resourceCategory = ResourceManager::getInstance().getCategoryName();
		FactoryManager::getInstance().unregisterFactory<ResourceManualFont>(resourceCategory);
		FactoryManager::getInstance().unregisterFactory<ResourceTrueTypeFont>(resourceCategory);

		MYGUI_LOG(Info, getClassTypeName() << " successfully shutdown");
		mIsInitialise = false;
	}

	void FontManager::_load(xml::ElementPtr _node, const std::string& _file, Version _version)
	{
#ifndef MYGUI_DONT_USE_OBSOLETE
		loadOldFontFormat(_node, _file, _version, mXmlFontTagName);
#endif // MYGUI_DONT_USE_OBSOLETE

		xml::ElementEnumerator node = _node->getElementEnumerator();
		while (node.next())
		{
			if (node->getName() == mXmlPropertyTagName)
			{
				const std::string& key = node->findAttribute("key");
				const std::string& value = node->findAttribute("value");
#ifdef MYGUI_USE_FREETYPE
				if (key == "Default")
#else
				if (key == "DefaultGenerated")
#endif
					mDefaultName = value;
			}
		}
	}

	void FontManager::setDefaultFont(const std::string& _value)
	{
		mDefaultName = _value;
	}

	IFont* FontManager::getByName(const std::string& _name) const
	{
		IResource* result = nullptr;
		//FIXME для совместимости шрифт может иметь имя Default
		if (!_name.empty() && _name != mXmlDefaultFontValue)
			result = ResourceManager::getInstance().getByName(_name, false);

		if (result == nullptr)
		{
			result = ResourceManager::getInstance().getByName(mDefaultName, false);
			if (!_name.empty() && _name != mXmlDefaultFontValue)
			{
				MYGUI_LOG(Error, "Font '" << _name << "' not found. Replaced with default font.");
			}
		}

		return result ? result->castType<IFont>(false) : nullptr;
	}

    FT_CALLBACK_DEF(unsigned long) ft_stream_read(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
    {
        FontDataStream* pStream = (FontDataStream*)(stream->descriptor.pointer);

        if(!pStream || buffer == 0 || count == 0) 
            return 0;

        if (offset > pStream->mEnd)
            return 0;

        const unsigned int STACK_BUF_SIZE = 1024*2;
        char pStatckBuf[STACK_BUF_SIZE] = {0};
        char* pTempBuf = pStatckBuf;
        if(count > STACK_BUF_SIZE)
        {
            pTempBuf = new char[count];
            memset(pTempBuf, 0, count);
        }

        unsigned long remainCount = count;
        unsigned long totalCount = 0;

        do 
        {
            unsigned long readsize = remainCount;
            if (offset + readsize > pStream->mEnd)
                return 0;

            pStream->mDataStream->seek(offset);
            totalCount += pStream->mDataStream->read(pTempBuf, readsize);

            remainCount -= readsize;
            if (remainCount <= 0 || pStream->mDataStream->eof())
                break;

        } while (true);

        memcpy(buffer, pTempBuf, totalCount);
        if(count > STACK_BUF_SIZE)
        {
            delete[] pTempBuf;
        }

        return totalCount;
    }

    FT_CALLBACK_DEF(void) ft_stream_close(FT_Stream stream)
    {
        FontDataStream* pStream = (FontDataStream*)(stream->descriptor.pointer);

        if(!pStream) 
            return;

        DataManager::getInstance().freeData(pStream->mDataStream);
        
        stream->descriptor.pointer = nullptr;
        stream->size = 0;
        stream->base = 0;
    }

    FontDataStream* FontManager::getFontDataStream(const std::string& _name)
    {
        const char* fontname = _name.c_str();

        FontDataStreamRegistry::iterator itFind = mFontData.find(fontname);
        if(itFind != mFontData.end())
            return itFind->second;

        FontDataStream* pFontDataStream = new FontDataStream;
        mFontData.insert(std::make_pair(fontname, pFontDataStream));

        IDataStream* datastream = DataManager::getInstance().getData(fontname);
        if (datastream == nullptr)
            return nullptr;

        pFontDataStream->mDataStream = datastream;
        pFontDataStream->mBegin = 0;
        pFontDataStream->mEnd = datastream->size();

        FT_StreamRec& ftStreamRec = pFontDataStream->mStreamRec;
        memset(&ftStreamRec, 0, sizeof(FT_StreamRec));

        ftStreamRec.descriptor.pointer = pFontDataStream;
        ftStreamRec.size = datastream->size();
        ftStreamRec.pos = 0;
        ftStreamRec.read = ft_stream_read;
        ftStreamRec.close = ft_stream_close;

        return pFontDataStream;
    }

    FontDataImpl* FontManager::getFontImpl()
    {
        return mFontImpl;
    }

    void FontManager::registerForPrepare(IFont* _font)
    {
        if (_font == nullptr) return;

        mDirtyFont.insert(FontPrepareRegistry::value_type(_font));
    }

    void FontManager::prepareFont()
    {
        for (FontPrepareRegistry::iterator itr = mDirtyFont.begin(); itr != mDirtyFont.end(); ++itr)
        {
            (*itr)->prepareTexture();
        }

        mDirtyFont.clear();
    }

	const std::string& FontManager::getDefaultFont() const
	{
		return mDefaultName;
	}

} // namespace MyGUI
