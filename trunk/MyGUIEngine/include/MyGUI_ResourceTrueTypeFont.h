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
#ifndef __MYGUI_RESOURCE_TRUE_TYPE_FONT_H__
#define __MYGUI_RESOURCE_TRUE_TYPE_FONT_H__

#include "MyGUI_Prerequest.h"
#include "MyGUI_ITexture.h"
#include "MyGUI_IFont.h"

#ifdef MYGUI_USE_FREETYPE
	#include <ft2build.h>
	#include <freetype/freetype.h>
#endif // MYGUI_USE_FREETYPE

namespace MyGUI
{

    struct FontDataImpl;

    struct GlyphTexture
    {
        ITexture* mTexture;
        uint8* mMemory;
        bool mDirty;

        GlyphTexture() : mTexture(nullptr), mMemory(nullptr), mDirty(false) {}
    };

	class MYGUI_EXPORT ResourceTrueTypeFont :
		public IFont
	{
		MYGUI_RTTI_DERIVED( ResourceTrueTypeFont )

	public:
		ResourceTrueTypeFont();
		virtual ~ResourceTrueTypeFont();

		virtual void deserialization(xml::ElementPtr _node, Version _version);

		// Returns the glyph info for the specified code point, or the glyph info for a substitute glyph if the code point does not
		// exist in this font. Returns nullptr if there is a problem with the font.
		virtual GlyphInfo* getGlyphInfo(Char _id);

		virtual ITexture* getTextureFont();

		// получившаяся высота при генерации в пикселях
		virtual int getDefaultHeight();

        virtual void prepareString(const UString& text);

        virtual void prepareTexture();

		// Returns a collection of code-point ranges that are supported by this font. Each range is specified as [first, second];
		// for example, a range containing a single code point will have the same value for both first and second.
		vector<std::pair<Char, Char> >::type getCodePointRanges() const;

		// Returns the code point that is used as a substitute for code points that don't exist in the font. The default substitute
		// code point is FontCodeType::NotDefined, but it can be customized in the font definition file.
		Char getSubstituteCodePoint() const;

		// создаение ресурса по текущим значениям
		void initialise();

		void setSource(const std::string& _value);
		void setSize(float _value);
		void setResolution(uint _value);
		void setHinting(const std::string& _value);
		void setAntialias(bool _value);
		void setTabWidth(float _value);
		void setOffsetHeight(int _value);
		void setSubstituteCode(int _value);
		void setDistance(int _value);

        void addCodePointRange(Char _first, Char _second) {}
        void removeCodePointRange(Char _first, Char _second) {}

#ifdef MYGUI_USE_FREETYPE
    protected:
        FontDataImpl* mFontDataImpl;
        FT_Face mFontFace;

        typedef list<GlyphTexture*>::type GlyphTextureAltas;
        GlyphTextureAltas mGlyphTextureAtlas;

        GlyphTexture* mCurGlyphTexture;

        uint mPtNextX, mPtNextY;
        uint mTextureSize;

        int mFontAscent, mFontDescent;
        FT_Int32 mFtLoadFlags;
	private:
		enum Hinting
		{
			HintingUseNative,
			HintingForceAuto,
			HintingDisableAuto,
			HintingDisableAll
		};

        void addCodePoint(Char _codePoint) {}
        void removeCodePoint(Char _codePoint) {}

        void clearCodePoints() {}

		// The following variables are set directly from values specified by the user.
		std::string mSource; // Source (filename) of the font.
		float mSize; // Size of the font, in points (there are 72 points per inch).
		uint mResolution; // Resolution of the font, in pixels per inch.
		Hinting mHinting; // What type of hinting to use when rendering the font.
		bool mAntialias; // Whether or not to anti-alias the font by copying its alpha channel to its luminance channel.
        bool mLAMode;
		float mSpaceWidth; // The width of a "Space" character, in pixels. If zero, the default width is used.
		int mGlyphSpacing; // How far apart the glyphs are placed from each other in the font texture, in pixels.
		float mTabWidth; // The width of the "Tab" special character, in pixels.
		int mOffsetHeight; // How far up to nudge text rendered in this font, in pixels. May be negative to nudge text down.
		Char mSubstituteCodePoint; // The code point to use as a substitute for code points that don't exist in the font.

		// The following variables are calculated automatically.
		int mDefaultHeight; // The nominal height of the font in pixels.
		GlyphInfo* mSubstituteGlyphInfo; // The glyph info to use as a substitute for code points that don't exist in the font.

		// The following constants used to be mutable, but they no longer need to be. Do not modify their values!
		static const int mDefaultGlyphSpacing; // How far apart the glyphs are placed from each other in the font texture, in pixels.
		static const float mDefaultTabWidth; // Default "Tab" width, used only when tab width is no specified.
		static const float mSelectedWidth; // The width of the "Selected" and "SelectedBack" special characters, in pixels.
		static const float mCursorWidth; // The width of the "Cursor" special character, in pixels.

	private:
		// A map of glyph indices to glyph info objects.
		typedef map<Char, GlyphInfo>::type GlyphMap;

		template<bool LAMode, bool Antialias>
		void initialiseFreeType();

		// Loads the font face as specified by mSource, mSize, and mResolution. Automatically adjusts code-point ranges according
		// to the capabilities of the font face.
		// Returns a handle to the FreeType face object for the face, or nullptr if the face could not be loaded.
		// Keeps the font file loaded in memory and stores its location in _fontBuffer. The caller is responsible for freeing this
		// buffer when it is done using the face by calling delete[] on the buffer after calling FT_Done_Face() on the face itself.
		void loadFace();

        void cleanupGlyphTextureAtlas();

        template<bool LAMode>
        GlyphTexture* createFontTexture();

		// Creates a GlyphInfo object using the specified information.
		GlyphInfo createFaceGlyphInfo(Char _codePoint, int _fontAscent, FT_GlyphSlot _glyph);

		// Creates a glyph with the specified glyph index and assigns it to the specified code point.
		// Automatically updates _glyphHeightMap, mCharMap, and mGlyphMap with data from the new glyph..
		GlyphInfo& createGlyph(const GlyphInfo& _glyphInfo);

		// Creates a glyph with the specified index from the specified font face and assigns it to the specified code point.
		// Automatically updates _glyphHeightMap with data from the newly created glyph.
		GlyphInfo& createFaceGlyph(Char _codePoint, int _fontAscent, const FT_Face& _ftFace, FT_Int32 _ftLoadFlags);

		// Renders all of the glyphs in _glyphHeightMap into the specified texture buffer using data from the specified font face.
		template<bool LAMode, bool Antialias>
		void renderGlyphs(GlyphInfo& _glyphInfo, const FT_Library& _ftLibrary, const FT_Face& _ftFace, FT_Int32 _ftLoadFlags);

		// Renders the glyph described by the specified glyph info according to the specified parameters.
		// Supports two types of rendering, depending on the value of UseBuffer: Texture block transfer and rectangular color fill.
		// The _luminance0 value is used for even-numbered columns (from zero), while _luminance1 is used for odd-numbered ones.
		template<bool LAMode, bool UseBuffer, bool Antialias>
		void renderGlyph(GlyphInfo& _info, uint8 _luminance0, uint8 _luminance1, uint8 _alpha, uint8* _glyphBuffer = nullptr);

		GlyphMap mGlyphMap; // A map of glyph indices to glyph info objects.

#endif // MYGUI_USE_FREETYPE

	};

} // namespace MyGUI

#endif // __MYGUI_RESOURCE_TRUE_TYPE_FONT_H__
