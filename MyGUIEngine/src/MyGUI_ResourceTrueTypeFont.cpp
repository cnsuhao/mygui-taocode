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
#include "MyGUI_ResourceTrueTypeFont.h"
#include "MyGUI_DataManager.h"
#include "MyGUI_DataStreamHolder.h"
#include "MyGUI_RenderManager.h"
#include "MyGUI_Bitwise.h"
#include "MyGUI_FontManager.h"
#include "MyGUI_FontDataStream.h"
#include "MyGUI_Gui.h"

#ifdef MYGUI_USE_FREETYPE

	#include <freetype/ftglyph.h>
	#include <freetype/tttables.h>
	#include <freetype/ftbitmap.h>
	#include <freetype/ftwinfnt.h>

	// The following macro enables a workaround for a bug in FreeType's bytecode interpreter that, when using certain fonts at
	// certain sizes, causes FreeType to start measuring and rendering some glyphs inconsistently after certain other glyphs have
	// been loaded. See FreeType bug #35374 for details: https://savannah.nongnu.org/bugs/?35374
	//
	// To reproduce the bug, first disable the workaround by defining MYGUI_USE_FREETYPE_BYTECODE_BUG_FIX to 0. Then load the
	// DejaVu Sans font at 10 pt using default values for all other properties. Observe that the glyphs for the "0", 6", "8", and
	// "9" characters are now badly corrupted when rendered.
	//
	// This bug still exists as of FreeType 2.4.8 and there are currently no plans to fix it. If the bug is ever fixed, this
	// workaround should be disabled, as it causes fonts to take longer to load.
	//
	// The bug can currently also be suppressed by disabling FreeType's bytecode interpreter altogether. To do so, remove the
	// TT_CONFIG_OPTION_BYTECODE_INTERPRETER macro in the "ftoption.h" FreeType header file. Once this is done, this workaround can
	// be safely disabled. Note that disabling FreeType's bytecode interpreter will cause rendered text to look somewhat different.
	// Whether it looks better or worse is a matter of taste and may also depend on the font.
	#ifndef MYGUI_USE_FREETYPE_BYTECODE_BUG_FIX
		#define MYGUI_USE_FREETYPE_BYTECODE_BUG_FIX 0
	#endif

#endif // MYGUI_USE_FREETYPE

namespace MyGUI
{

#ifndef MYGUI_USE_FREETYPE
	ResourceTrueTypeFont::ResourceTrueTypeFont()
	{
	}

	ResourceTrueTypeFont::~ResourceTrueTypeFont()
	{
	}

	void ResourceTrueTypeFont::deserialization(xml::ElementPtr _node, Version _version)
	{
		Base::deserialization(_node, _version);
		MYGUI_LOG(Error, "ResourceTrueTypeFont: TrueType font '" << getResourceName() << "' disabled. Define MYGUI_USE_FREETYE if you need TrueType fonts.");
	}

	GlyphInfo* ResourceTrueTypeFont::getGlyphInfo(Char _id)
	{
		return nullptr;
	}

	ITexture* ResourceTrueTypeFont::getTextureFont()
	{
		return nullptr;
	}

	int ResourceTrueTypeFont::getDefaultHeight()
	{
		return 0;
	}

	vector<std::pair<Char, Char> >::type ResourceTrueTypeFont::getCodePointRanges() const
	{
		return vector<std::pair<Char, Char> >::type();
	}

	Char ResourceTrueTypeFont::getSubstituteCodePoint() const
	{
		return Char();
	}

	void ResourceTrueTypeFont::initialise()
	{
	}

	void ResourceTrueTypeFont::setSource(const std::string& _value)
	{
	}

	void ResourceTrueTypeFont::setSize(float _value)
	{
	}

	void ResourceTrueTypeFont::setResolution(uint _value)
	{
	}

	void ResourceTrueTypeFont::setHinting(const std::string& _value)
	{
	}

	void ResourceTrueTypeFont::setAntialias(bool _value)
	{
	}

	void ResourceTrueTypeFont::setTabWidth(float _value)
	{
	}

	void ResourceTrueTypeFont::setOffsetHeight(int _value)
	{
	}

	void ResourceTrueTypeFont::setSubstituteCode(int _value)
	{
	}

	void ResourceTrueTypeFont::setDistance(int _value)
	{
	}

	void ResourceTrueTypeFont::addCodePointRange(Char _first, Char _second)
	{
	}

	void ResourceTrueTypeFont::removeCodePointRange(Char _first, Char _second)
	{
	}

#else // MYGUI_USE_FREETYPE
	namespace
	{

		template<typename T>
		void setMax(T& _var, const T& _newValue)
		{
			if (_var < _newValue)
				_var = _newValue;
		}

		std::pair<const Char, const uint8> charMaskData[] =
		{
			std::make_pair(FontCodeType::Selected, (const uint8)'\x88'),
			std::make_pair(FontCodeType::SelectedBack, (const uint8)'\x60'),
			std::make_pair(FontCodeType::Cursor, (const uint8)'\xFF'),
			std::make_pair(FontCodeType::Tab, (const uint8)'\x00')
		};

		const map<const Char, const uint8>::type charMask(charMaskData, charMaskData + sizeof charMaskData / sizeof *charMaskData);

		const uint8 charMaskBlack = (const uint8)'\x00';
		const uint8 charMaskWhite = (const uint8)'\xFF';

		template<bool LAMode>
		struct PixelBase
		{
			// Returns PixelFormat::R8G8B8A8 when LAMode is false, or PixelFormat::L8A8 when LAMode is true.
			static PixelFormat::Enum getFormat();

			// Returns 4 when LAMode is false, or 2 when LAMode is true.
			static size_t getNumBytes();

		protected:
			// Sets a pixel in _dest as 4 or 2 bytes: L8L8L8A8 if LAMode is false, or L8A8 if LAMode is true.
			// Automatically advances _dest just past the pixel written.
			static void set(uint8*& _dest, uint8 _luminance, uint8 _alpha);
		};

		template<>
		struct PixelBase<false>
		{
			static size_t getNumBytes()
			{
				return 4;
			}

			static PixelFormat::Enum getFormat()
			{
				return PixelFormat::R8G8B8A8;
			}

		protected:
			static void set(uint8*& _dest, uint8 _luminance, uint8 _alpha)
			{
				*_dest++ = _luminance;
				*_dest++ = _luminance;
				*_dest++ = _luminance;
				*_dest++ = _alpha;
			}
		};

		template<>
		struct PixelBase<true>
		{
			static size_t getNumBytes()
			{
				return 2;
			}

			static PixelFormat::Enum getFormat()
			{
				return PixelFormat::L8A8;
			}

		protected:
			static void set(uint8*& _dest, uint8 _luminance, uint8 _alpha)
			{
				*_dest++ = _luminance;
				*_dest++ = _alpha;
			}
		};

		template<bool LAMode, bool FromSource = false, bool Antialias = false>
		struct Pixel : PixelBase<LAMode>
		{
			// Sets a pixel in _dest as 4 or 2 bytes: L8L8L8A8 if LAMode is false, or L8A8 if LAMode is true.
			// Sets luminance from _source if both FromSource and Antialias are true; otherwise, uses the specified value.
			// Sets alpha from _source if FromSource is true; otherwise, uses the specified value.
			// Automatically advances _source just past the pixel read if FromSource is true.
			// Automatically advances _dest just past the pixel written.
			static void set(uint8*& _dest, uint8 _luminance, uint8 _alpha, uint8*& _source);
		};

		template<bool LAMode, bool Antialias>
		struct Pixel<LAMode, false, Antialias> : PixelBase<LAMode>
		{
			// Sets the destination pixel using the specified luminance and alpha. Source is ignored, since FromSource is false.
			static void set(uint8*& _dest, uint8 _luminance, uint8 _alpha, uint8* = nullptr)
			{
				PixelBase<LAMode>::set(_dest, _luminance, _alpha);
			}
		};

		template<bool LAMode>
		struct Pixel<LAMode, true, false> : PixelBase<LAMode>
		{
			// Sets the destination pixel using the specified _luminance and using the alpha from the specified source.
			static void set(uint8*& _dest, uint8 _luminance, uint8, uint8*& _source)
			{
				PixelBase<LAMode>::set(_dest, _luminance, *_source++);
			}
		};

		template<bool LAMode>
		struct Pixel<LAMode, true, true> : PixelBase<LAMode>
		{
			// Sets the destination pixel using both the luminance and alpha from the specified source, since Antialias is true.
			static void set(uint8*& _dest, uint8, uint8, uint8*& _source)
			{
				PixelBase<LAMode>::set(_dest, *_source, *_source);
				++_source;
			}
		};

	}

	const int ResourceTrueTypeFont::mDefaultGlyphSpacing = 1;
	const float ResourceTrueTypeFont::mDefaultTabWidth = 8.0f;
	const float ResourceTrueTypeFont::mSelectedWidth = 1.0f;
	const float ResourceTrueTypeFont::mCursorWidth = 2.0f;

	ResourceTrueTypeFont::ResourceTrueTypeFont() :
        mFontDataImpl(nullptr),
        mCurGlyphTexture(nullptr),
        mPtNextX(0),
        mPtNextY(0),
        mTextureSize(256),
        mFontAscent(0),
        mFontDescent(0),
        mFtLoadFlags(FT_LOAD_DEFAULT),
		mSize(0),
		mResolution(96),
		mHinting(HintingUseNative),
		mAntialias(false),
        mLAMode(true),
		mSpaceWidth(0.0f),
		mGlyphSpacing(-1),
		mTabWidth(0.0f),
		mOffsetHeight(0),
		mSubstituteCodePoint(static_cast<Char>(FontCodeType::NotDefined)),
		mDefaultHeight(0),
		mSubstituteGlyphInfo(nullptr)
	{
        mFontDataImpl = FontManager::getInstance().getFontImpl();
	}

	ResourceTrueTypeFont::~ResourceTrueTypeFont()
	{
		cleanupGlyphTextureAtlas();

        FT_Done_Face(mFontFace);
	}

	void ResourceTrueTypeFont::deserialization(xml::ElementPtr _node, Version _version)
	{
		Base::deserialization(_node, _version);

		xml::ElementEnumerator node = _node->getElementEnumerator();
		while (node.next())
		{
			if (node->getName() == "Property")
			{
				const std::string& key = node->findAttribute("key");
				const std::string& value = node->findAttribute("value");
				if (key == "Source")
					setSource(value);
				else if (key == "Size")
					setSize(utility::parseFloat(value) * Gui::getInstance().getZoomScale());
				else if (key == "Resolution")
					setResolution(utility::parseUInt(value));
				else if (key == "Antialias")
					setAntialias(utility::parseBool(value));
				else if (key == "TabWidth")
					setTabWidth(utility::parseFloat(value) * Gui::getInstance().getZoomScale());
				else if (key == "OffsetHeight")
					setOffsetHeight(utility::parseInt(value) * Gui::getInstance().getZoomScale());
				else if (key == "SubstituteCode")
					setSubstituteCode(utility::parseInt(value));
				else if (key == "Distance")
					setDistance(utility::parseInt(value) * Gui::getInstance().getZoomScale());
				else if (key == "Hinting")
					setHinting(value);
				else if (key == "SpaceWidth")
				{
					mSpaceWidth = utility::parseFloat(value) * Gui::getInstance().getZoomScale();
					MYGUI_LOG(Warning, _node->findAttribute("type") << ": Property '" << key << "' in font '" << _node->findAttribute("name") << "' is deprecated; remove it to use automatic calculation.");
				}
				else if (key == "CursorWidth")
				{
					MYGUI_LOG(Warning, _node->findAttribute("type") << ": Property '" << key << "' in font '" << _node->findAttribute("name") << "' is deprecated; value ignored.");
				}
			}
		}

		initialise();
	}

	GlyphInfo* ResourceTrueTypeFont::getGlyphInfo(Char _id)
	{
        GlyphMap::iterator glyphIter = mGlyphMap.find(_id);

        if (glyphIter != mGlyphMap.end())
            return &glyphIter->second;

		return mSubstituteGlyphInfo;
	}

	ITexture* ResourceTrueTypeFont::getTextureFont()
	{
		return mGlyphTextureAtlas.front()->mTexture;
	}

	int ResourceTrueTypeFont::getDefaultHeight()
	{
		return mDefaultHeight;
	}

	vector<std::pair<Char, Char> >::type ResourceTrueTypeFont::getCodePointRanges() const
	{
		vector<std::pair<Char, Char> >::type result;

		return result;
	}

	Char ResourceTrueTypeFont::getSubstituteCodePoint() const
	{
		return mSubstituteCodePoint;
	}

	void ResourceTrueTypeFont::initialise()
	{
		if (mGlyphSpacing == -1)
			mGlyphSpacing = mDefaultGlyphSpacing;

		// If L8A8 (2 bytes per pixel) is supported, use it; otherwise, use R8G8B8A8 (4 bytes per pixel) as L8L8L8A8.
		mLAMode = MyGUI::RenderManager::getInstance().isFormatSupported(Pixel<true>::getFormat(), TextureUsage::Static | TextureUsage::Write);

		// Select and call an appropriate initialisation method. By making this decision up front, we avoid having to branch on
		// these variables many thousands of times inside tight nested loops later. From this point on, the various function
		// templates ensure that all of the necessary branching is done purely at compile time for all combinations.
		int init = (mLAMode ? 2 : 0) | (mAntialias ? 1 : 0);

		switch (init)
		{
		case 0:
			ResourceTrueTypeFont::initialiseFreeType<false, false>();
			break;
		case 1:
			ResourceTrueTypeFont::initialiseFreeType<false, true>();
			break;
		case 2:
			ResourceTrueTypeFont::initialiseFreeType<true, false>();
			break;
		case 3:
			ResourceTrueTypeFont::initialiseFreeType<true, true>();
			break;
		}
	}

	template<bool LAMode, bool Antialias>
	void ResourceTrueTypeFont::initialiseFreeType()
	{
		//-------------------------------------------------------------------//
		// Initialise FreeType and load the font.
		//-------------------------------------------------------------------//

        FT_Library& ftLib = mFontDataImpl->mFTLib;

		uint8* fontBuffer = nullptr;

		loadFace();

		if (mFontFace == nullptr)
		{
			MYGUI_LOG(Error, "ResourceTrueTypeFont: Could not load the font '" << getResourceName() << "'!");
			return;
		}

		//-------------------------------------------------------------------//
		// Calculate the font metrics.
		//-------------------------------------------------------------------//

		// The font's overall ascent and descent are defined in three different places in a TrueType font, and with different
		// values in each place. The most reliable source for these metrics is usually the "usWinAscent" and "usWinDescent" pair of
		// values in the OS/2 header; however, some fonts contain inaccurate data there. To be safe, we use the highest of the set
		// of values contained in the face metrics and the two sets of values contained in the OS/2 header.
		mFontAscent = mFontFace->size->metrics.ascender >> 6;
		mFontDescent = -mFontFace->size->metrics.descender >> 6;

		TT_OS2* os2 = (TT_OS2*)FT_Get_Sfnt_Table(mFontFace, ft_sfnt_os2);

		if (os2 != nullptr)
		{
			setMax(mFontAscent, os2->usWinAscent * mFontFace->size->metrics.y_ppem / mFontFace->units_per_EM);
			setMax(mFontDescent, os2->usWinDescent * mFontFace->size->metrics.y_ppem / mFontFace->units_per_EM);

			setMax(mFontAscent, os2->sTypoAscender * mFontFace->size->metrics.y_ppem / mFontFace->units_per_EM);
			setMax(mFontDescent, -os2->sTypoDescender * mFontFace->size->metrics.y_ppem / mFontFace->units_per_EM);
		}

		// The nominal font height is calculated as the sum of its ascent and descent as specified by the font designer. Previously
		// it was defined by MyGUI in terms of the maximum ascent and descent of the glyphs currently in use, but this caused the
		// font's line spacing to change whenever glyphs were added to or removed from the font definition. Doing it this way
		// instead prevents a lot of layout problems, and it is also more typographically correct and more aesthetically pleasing.
		mDefaultHeight = mFontAscent + mFontDescent;

		// Set the load flags based on the specified type of hinting.
		switch (mHinting)
		{
		case HintingForceAuto:
			mFtLoadFlags = FT_LOAD_FORCE_AUTOHINT;
			break;
		case HintingDisableAuto:
			mFtLoadFlags = FT_LOAD_NO_AUTOHINT;
			break;
		case HintingDisableAll:
			// When hinting is completely disabled, glyphs must always be rendered -- even during layout calculations -- due to
			// discrepancies between the glyph metrics and the actual rendered bitmap metrics.
			mFtLoadFlags = FT_LOAD_NO_HINTING | FT_LOAD_RENDER;
			break;
		case HintingUseNative:
		default:
			mFtLoadFlags = FT_LOAD_DEFAULT;
			break;
		}

		//-------------------------------------------------------------------//
		// Create the glyphs and calculate their metrics.
		//-------------------------------------------------------------------//

#if MYGUI_USE_FREETYPE_BYTECODE_BUG_FIX

		bool isBytecodeAvailable = (mFontFace->face_flags & FT_FACE_FLAG_HINTER) != 0;
		bool isBytecodeUsedByLoadFlags = (mFtLoadFlags & (FT_LOAD_FORCE_AUTOHINT | FT_LOAD_NO_HINTING)) == 0;

		if (isBytecodeAvailable && isBytecodeUsedByLoadFlags)
		{
			for (GlyphMap::iterator iter = mGlyphMap.begin(); iter != mGlyphMap.end(); ++iter)
			{
				if (FT_Load_Glyph(mFontFace, iter->first, mFtLoadFlags) == 0)
				{
					GlyphInfo& info = iter->second;
					GlyphInfo newInfo = createFaceGlyphInfo(0, mFontAscent, mFontFace->glyph);

					if (info.width != newInfo.width)
					{
						texWidth += (int)ceil(newInfo.width) - (int)ceil(info.width);
						info.width = newInfo.width;
					}

					if (info.height != newInfo.height)
					{
						GlyphHeightMap::mapped_type oldHeightMap = glyphHeightMap[(FT_Pos)info.height];
						GlyphHeightMap::mapped_type::iterator heightMapItem = oldHeightMap.find(iter->first);
						glyphHeightMap[(FT_Pos)newInfo.height].insert(*heightMapItem);
						oldHeightMap.erase(heightMapItem);
						info.height = newInfo.height;
					}

					if (info.advance != newInfo.advance)
						info.advance = newInfo.advance;

					if (info.bearingX != newInfo.bearingX)
						info.bearingX = newInfo.bearingX;

					if (info.bearingY != newInfo.bearingY)
						info.bearingY = newInfo.bearingY;
				}
				else
				{
					MYGUI_LOG(Warning, "ResourceTrueTypeFont: Cannot load glyph " << iter->first << " for character " << iter->second.codePoint << " in font '" << getResourceName() << "'.");
				}
			}
		}

#endif // MYGUI_USE_FREETYPE_BYTECODE_BUG_FIX

		// Create the special glyphs They must be created after the standard glyphs so that they take precedence in case of a
		// collision. To make sure that the indices of the special glyphs don't collide with any glyph indices in the font, we must
		// use glyph indices higher than the highest glyph index in the font.
		float height = (float)mDefaultHeight;

        createGlyph(GlyphInfo(static_cast<Char>(FontCodeType::Space), nullptr, mSpaceWidth, height, mSpaceWidth, 0.0f, 0.0f));
		createGlyph(GlyphInfo(static_cast<Char>(FontCodeType::Tab), nullptr, 0.0f, 0.0f, mTabWidth, 0.0f, 0.0f));
		createGlyph(GlyphInfo(static_cast<Char>(FontCodeType::Selected), nullptr, mSelectedWidth, height, 0.0f, 0.0f, 0.0f));
		createGlyph(GlyphInfo(static_cast<Char>(FontCodeType::SelectedBack), nullptr, mSelectedWidth, height, 0.0f, 0.0f, 0.0f));
		createGlyph(GlyphInfo(static_cast<Char>(FontCodeType::Cursor), nullptr, mCursorWidth, height, 0.0f, 0.0f, 0.0f));

		// If a substitute code point has been specified, check to make sure that it exists in the character map. If it doesn't,
		// revert to the default "Not Defined" code point. This is not a real code point but rather an invalid Unicode value that
		// is guaranteed to cause the "Not Defined" special glyph to be created.
		if (mSubstituteCodePoint != FontCodeType::NotDefined && mGlyphMap.find(mSubstituteCodePoint) == mGlyphMap.end())
			mSubstituteCodePoint = static_cast<Char>(FontCodeType::NotDefined);

		// Create the "Not Defined" code point (and its corresponding glyph) if it's in use as the substitute code point.
		if (mSubstituteCodePoint == FontCodeType::NotDefined)
			createFaceGlyph(static_cast<Char>(FontCodeType::NotDefined), mFontAscent, mFontFace, mFtLoadFlags);

		// Cache a pointer to the substitute glyph info for fast lookup.
		mSubstituteGlyphInfo = &mGlyphMap.find(mSubstituteCodePoint)->second;

        // Render the special glyphs
        renderGlyphs<LAMode, Antialias>(mGlyphMap[FontCodeType::Space], mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
        renderGlyphs<LAMode, Antialias>(mGlyphMap[FontCodeType::Tab], mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
        renderGlyphs<LAMode, Antialias>(mGlyphMap[FontCodeType::Selected], mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
        renderGlyphs<LAMode, Antialias>(mGlyphMap[FontCodeType::SelectedBack], mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
        renderGlyphs<LAMode, Antialias>(mGlyphMap[FontCodeType::Cursor], mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
        renderGlyphs<LAMode, Antialias>(mGlyphMap[FontCodeType::NotDefined], mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
	}

	void ResourceTrueTypeFont::loadFace()
	{
        FontDataStream* pStream = FontManager::getInstance().getFontDataStream(mSource);

        FT_Open_Args  args;
        args.flags	= FT_OPEN_STREAM;
        args.stream	= &(pStream->mStreamRec);

        if (FT_Open_Face(mFontDataImpl->mFTLib, &args, 0, &mFontFace) != 0)
            MYGUI_EXCEPT("ResourceTrueTypeFont: Could not load the font '" << getResourceName() << "'!");

        if (mFontFace->face_flags & FT_FACE_FLAG_SCALABLE)
        {
            // The font is scalable, so set the font size by first converting the requested size to FreeType's 26.6 fixed-point
            // format.
            FT_F26Dot6 ftSize = (FT_F26Dot6)(mSize * (1 << 6));

            if (FT_Set_Char_Size(mFontFace, ftSize, 0, mResolution, mResolution) != 0)
                MYGUI_EXCEPT("ResourceTrueTypeFont: Could not set the font size for '" << getResourceName() << "'!");
        }

        cleanupGlyphTextureAtlas();
	}

    void ResourceTrueTypeFont::cleanupGlyphTextureAtlas()
    {
        GlyphTexture* texture = nullptr;
        for (GlyphTextureAltas::iterator itr = mGlyphTextureAtlas.begin(); itr != mGlyphTextureAtlas.end(); ++itr)
        {
            texture = *itr;

            delete[] texture->mMemory;
            texture->mMemory = nullptr;

            RenderManager::getInstance().destroyTexture(texture->mTexture);

            delete texture;
        }

        mPtNextX = mPtNextY = 0;
        mCurGlyphTexture = nullptr;

        mGlyphTextureAtlas.clear();
    }

    void ResourceTrueTypeFont::prepareString(const UString& text)
    {
        if (text.empty())
            return;

        int init = (mLAMode ? 2 : 0) | (mAntialias ? 1 : 0);

        size_t count = text.length();
        for (size_t i = 0; i < count; ++i)
        {
            Char codePoint = text[i];

            if (codePoint == FontCodeType::CR || codePoint == FontCodeType::LF)
                continue;

            if (getGlyphInfo(codePoint) == mSubstituteGlyphInfo)
            {
                GlyphInfo& glyphInfo = createFaceGlyph(codePoint, mFontAscent, mFontFace, mFtLoadFlags);

                switch (init)
                {
                case 0:
                    renderGlyphs<false, false>(glyphInfo, mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
                    break;
                case 1:
                    renderGlyphs<false, true>(glyphInfo, mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
                    break;
                case 2:
                    renderGlyphs<true, false>(glyphInfo, mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
                    break;
                case 3:
                    renderGlyphs<true, true>(glyphInfo, mFontDataImpl->mFTLib, mFontFace, mFtLoadFlags);
                    break;
                }
            }
        }
    }

    void ResourceTrueTypeFont::prepareTexture()
    {
        GlyphTexture* texture = nullptr;
        for (GlyphTextureAltas::iterator itr = mGlyphTextureAtlas.begin(); itr != mGlyphTextureAtlas.end(); ++itr)
        {
            texture = *itr;

            if (texture->mDirty)
            {
                texture->mTexture->loadFromMemory(texture->mMemory);
                texture->mDirty = false;
            }
        }
    }

    template<bool LAMode>
    GlyphTexture* ResourceTrueTypeFont::createFontTexture()
    {
        static int textureGUID = 0;

        GlyphTexture* glyphtexture = new GlyphTexture;

        glyphtexture->mDirty = true;
        glyphtexture->mMemory = new uint8[mTextureSize * mTextureSize * Pixel<LAMode>::getNumBytes()];
        memset(glyphtexture->mMemory, 0, mTextureSize * mTextureSize * Pixel<LAMode>::getNumBytes());

        glyphtexture->mTexture = RenderManager::getInstance().createTexture(MyGUI::utility::toString(++textureGUID, "_TrueTypeFont"));

        glyphtexture->mTexture->createManual(mTextureSize, mTextureSize, TextureUsage::Static | TextureUsage::Write, Pixel<LAMode>::getFormat());

        mGlyphTextureAtlas.push_back(glyphtexture);

        mPtNextX = mPtNextY = 0;

        return glyphtexture;
    }

	GlyphInfo ResourceTrueTypeFont::createFaceGlyphInfo(Char _codePoint, int _fontAscent, FT_GlyphSlot _glyph)
	{
		float bearingX = _glyph->metrics.horiBearingX / 64.0f;

		// The following calculations aren't currently needed but are kept here for future use.
		// float ascent = _glyph->metrics.horiBearingY / 64.0f;
		// float descent = (_glyph->metrics.height / 64.0f) - ascent;

        ITexture* texture = nullptr;
        if (mCurGlyphTexture)
        {
            texture = mCurGlyphTexture->mTexture;
        }

		return GlyphInfo(
			_codePoint,
            texture,
			std::max((float)_glyph->bitmap.width, _glyph->metrics.width / 64.0f),
			std::max((float)_glyph->bitmap.rows, _glyph->metrics.height / 64.0f),
			(_glyph->advance.x / 64.0f) - bearingX,
			bearingX,
			floor(_fontAscent - (_glyph->metrics.horiBearingY / 64.0f) - mOffsetHeight));
	}

	GlyphInfo& ResourceTrueTypeFont::createGlyph(const GlyphInfo& _glyphInfo)
	{
		return mGlyphMap.insert(GlyphMap::value_type(_glyphInfo.codePoint, _glyphInfo)).first->second;
	}

	GlyphInfo& ResourceTrueTypeFont::createFaceGlyph(Char _codePoint, int _fontAscent, const FT_Face& _ftFace, FT_Int32 _ftLoadFlags)
	{
        GlyphMap::iterator itr = mGlyphMap.find(_codePoint);
		if (itr == mGlyphMap.end())
		{
			if (FT_Load_Char(_ftFace, _codePoint, _ftLoadFlags) == 0)
				return createGlyph(createFaceGlyphInfo(_codePoint, _fontAscent, _ftFace->glyph));
			else
				MYGUI_LOG(Warning, "ResourceTrueTypeFont: Cannot load glyph for character " << _codePoint << " in font '" << getResourceName() << "'.");
		}
		else
		{
            return itr->second;
		}

		return *mSubstituteGlyphInfo;
	}

	template<bool LAMode, bool Antialias>
	void ResourceTrueTypeFont::renderGlyphs(GlyphInfo& _glyphInfo, const FT_Library& _ftLibrary, const FT_Face& _ftFace, FT_Int32 _ftLoadFlags)
	{
		FT_Bitmap ftBitmap;
		FT_Bitmap_New(&ftBitmap);

        switch (_glyphInfo.codePoint)
        {
        case FontCodeType::Selected:
        case FontCodeType::SelectedBack:
            {
                renderGlyph<LAMode, false, false>(_glyphInfo, charMaskWhite, charMaskBlack, charMask.find(_glyphInfo.codePoint)->second);

                // Manually adjust the glyph's width to zero. This prevents artifacts from appearing at the seams when
                // rendering multi-character selections.
                GlyphInfo* glyphInfo = getGlyphInfo(_glyphInfo.codePoint);
                glyphInfo->width = 0.0f;
                glyphInfo->uvRect.right = glyphInfo->uvRect.left;
            }
            break;

        case FontCodeType::Cursor:
        case FontCodeType::Tab:
            renderGlyph<LAMode, false, false>(_glyphInfo, charMaskWhite, charMaskBlack, charMask.find(_glyphInfo.codePoint)->second);
            break;

        default:
            if (FT_Load_Char(_ftFace, _glyphInfo.codePoint, _ftLoadFlags | FT_LOAD_RENDER) == 0)
            {
                if (_ftFace->glyph->bitmap.buffer != nullptr)
                {
                    uint8* glyphBuffer = nullptr;

                    switch (_ftFace->glyph->bitmap.pixel_mode)
                    {
                    case FT_PIXEL_MODE_GRAY:
                        glyphBuffer = _ftFace->glyph->bitmap.buffer;
                        break;

                    case FT_PIXEL_MODE_MONO:
                        // Convert the monochrome bitmap to 8-bit before rendering it.
                        if (FT_Bitmap_Convert(_ftLibrary, &_ftFace->glyph->bitmap, &ftBitmap, 1) == 0)
                        {
                            // Go through the bitmap and convert all of the nonzero values to 0xFF (white).
                            for (uint8* p = ftBitmap.buffer, * endP = p + ftBitmap.width * ftBitmap.rows; p != endP; ++p)
                                *p ^= -*p ^ *p;

                            glyphBuffer = ftBitmap.buffer;
                        }
                        break;
                    }

                    if (glyphBuffer != nullptr)
                        renderGlyph<LAMode, true, Antialias>(_glyphInfo, charMaskWhite, charMaskWhite, charMaskWhite, glyphBuffer);
                }
            }
            else
            {
                MYGUI_LOG(Warning, "ResourceTrueTypeFont: Cannot render glyph  for character " << _glyphInfo.codePoint << " in font '" << getResourceName() << "'.");
            }
            break;
        }

		FT_Bitmap_Done(_ftLibrary, &ftBitmap);
	}

	template<bool LAMode, bool UseBuffer, bool Antialias>
	void ResourceTrueTypeFont::renderGlyph(GlyphInfo& _info, uint8 _luminance0, uint8 _luminance1, uint8 _alpha, uint8* _glyphBuffer)
	{
		int width = (int)ceil(_info.width);
		int height = (int)ceil(_info.height);


        if (mCurGlyphTexture == nullptr || mPtNextY + (mDefaultHeight + mGlyphSpacing) > mTextureSize)
        {
            mCurGlyphTexture = createFontTexture<LAMode>();
        }
        else
        {
            if (mPtNextX + width > mTextureSize)
            {
                mPtNextY += (mDefaultHeight + mGlyphSpacing);
                mPtNextX = 0;

                if (mPtNextY + (mDefaultHeight + mGlyphSpacing) > mTextureSize)
                {
                    mCurGlyphTexture = createFontTexture<LAMode>();
                }
            }
        }

        _info.texture = mCurGlyphTexture->mTexture;

		uint8* dest = mCurGlyphTexture->mMemory + (mPtNextY * mTextureSize + mPtNextX) * Pixel<LAMode>::getNumBytes();

		// Calculate how much to advance the destination pointer after each row to get to the start of the next row.
		ptrdiff_t destNextRow = (mTextureSize - width) * Pixel<LAMode>::getNumBytes();

		for (int j = height; j > 0; --j)
		{
			int i;
			for (i = width; i > 1; i -= 2)
			{
				Pixel<LAMode, UseBuffer, Antialias>::set(dest, _luminance0, _alpha, _glyphBuffer);
				Pixel<LAMode, UseBuffer, Antialias>::set(dest, _luminance1, _alpha, _glyphBuffer);
			}

			if (i > 0)
				Pixel<LAMode, UseBuffer, Antialias>::set(dest, _luminance0, _alpha, _glyphBuffer);

			dest += destNextRow;
		}

		// Calculate and store the glyph's UV coordinates within the texture.
		_info.uvRect.left = (float)mPtNextX / mTextureSize; // u1
		_info.uvRect.top = (float)mPtNextY / mTextureSize; // v1
		_info.uvRect.right = (float)(mPtNextX + _info.width) / mTextureSize; // u2
		_info.uvRect.bottom = (float)(mPtNextY + _info.height) / mTextureSize; // v2

        mPtNextX += (width + mGlyphSpacing);

        mCurGlyphTexture->mDirty = true;

        FontManager::getInstance().registerForPrepare(this);
	}

	void ResourceTrueTypeFont::setSource(const std::string& _value)
	{
		mSource = _value;
	}

	void ResourceTrueTypeFont::setSize(float _value)
	{
		mSize = _value;
	}

	void ResourceTrueTypeFont::setResolution(uint _value)
	{
		mResolution = _value;
	}

	void ResourceTrueTypeFont::setHinting(const std::string& _value)
	{
		if (_value == "use_native")
			mHinting = HintingUseNative;
		else if (_value == "force_auto")
			mHinting = HintingForceAuto;
		else if (_value == "disable_auto")
			mHinting = HintingDisableAuto;
		else if (_value == "disable_all")
			mHinting = HintingDisableAll;
		else
			mHinting = HintingUseNative;
	}

	void ResourceTrueTypeFont::setAntialias(bool _value)
	{
		mAntialias = _value;
	}

	void ResourceTrueTypeFont::setTabWidth(float _value)
	{
		mTabWidth = _value;
	}

	void ResourceTrueTypeFont::setOffsetHeight(int _value)
	{
		mOffsetHeight = _value;
	}

	void ResourceTrueTypeFont::setSubstituteCode(int _value)
	{
		mSubstituteCodePoint = _value;
	}

	void ResourceTrueTypeFont::setDistance(int _value)
	{
		mGlyphSpacing = _value;
	}

#endif // MYGUI_USE_FREETYPE

} // namespace MyGUI
