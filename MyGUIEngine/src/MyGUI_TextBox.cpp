/*!
	@file
	@author		Albert Semenov
	@date		12/2007
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
#include "MyGUI_TextBox.h"
#include "MyGUI_LanguageManager.h"
#include "MyGUI_Constants.h"
#include "MyGUI_Gui.h"

namespace MyGUI
{

    TextBox::TextBox() : mAutoDisappear(false), mDisappearTime(0), mFadeTime(0), mElapsedTime(0)
	{
	}

	IntCoord TextBox::getTextRegion()
	{
		return (nullptr == getSubWidgetText()) ? IntCoord() : getSubWidgetText()->getCoord();
	}

	IntSize TextBox::getTextSize()
	{
		return (nullptr == getSubWidgetText()) ? IntSize() : getSubWidgetText()->getTextSize();
	}

	void TextBox::setTextAlign(Align _value)
	{
		if (getSubWidgetText() != nullptr)
			getSubWidgetText()->setTextAlign(_value);
	}

	Align TextBox::getTextAlign()
	{
		if (getSubWidgetText() != nullptr)
			return getSubWidgetText()->getTextAlign();
		return Align::Default;
	}

	void TextBox::setTextColour(const Colour& _value)
	{
		if (nullptr != getSubWidgetText())
			getSubWidgetText()->setTextColour(_value);
	}

	const Colour& TextBox::getTextColour()
	{
		return (nullptr == getSubWidgetText()) ? Colour::Zero : getSubWidgetText()->getTextColour();
	}

	void TextBox::setFontName(const std::string& _value)
	{
		if (nullptr != getSubWidgetText())
			getSubWidgetText()->setFontName(_value);
	}

	const std::string& TextBox::getFontName()
	{
		if (nullptr == getSubWidgetText())
			return Constants::getEmptyString();
		return getSubWidgetText()->getFontName();
	}

	void TextBox::setFontHeight(int _height)
	{
		if (nullptr != getSubWidgetText())
			getSubWidgetText()->setFontHeight(_height);
	}

	int TextBox::getFontHeight()
	{
		return (nullptr == getSubWidgetText()) ? 0 : getSubWidgetText()->getFontHeight();
	}

	void TextBox::setCaption(const UString& _caption)
	{
		if (nullptr != getSubWidgetText())
			getSubWidgetText()->setCaption(_caption);
	}

	const UString& TextBox::getCaption()
	{
		if (nullptr == getSubWidgetText())
			return Constants::getEmptyUString();
		return getSubWidgetText()->getCaption();
	}

	void TextBox::setCaptionWithReplacing(const std::string& _value)
	{
		// replace "\\n" with char '\n'
		size_t pos = _value.find("\\n");
		if (pos == std::string::npos)
		{
			setCaption(LanguageManager::getInstance().replaceTags(_value));
		}
		else
		{
			std::string value(_value);
			while (pos != std::string::npos)
			{
				value[pos++] = '\n';
				value.erase(pos, 1);
				pos = value.find("\\n");
			}
			setCaption(LanguageManager::getInstance().replaceTags(value));
		}
	}

	void TextBox::setTextShadowColour(const Colour& _value)
	{
		if (nullptr != getSubWidgetText())
			getSubWidgetText()->setShadowColour(_value);
	}

	const Colour& TextBox::getTextShadowColour()
	{
		return (nullptr == getSubWidgetText()) ? Colour::Black : getSubWidgetText()->getShadowColour();
	}

	void TextBox::setTextShadow(bool _value)
	{
		if (nullptr != getSubWidgetText())
			getSubWidgetText()->setShadow(_value);
	}

	bool TextBox::getTextShadow()
	{
		return (nullptr == getSubWidgetText()) ? false : getSubWidgetText()->getShadow();
	}

	void TextBox::setPropertyOverride(const std::string& _key, const std::string& _value)
	{
		/// @wproperty{TextBox, TextColour, Colour} Цвет текста.
		if (_key == "TextColour")
			setTextColour(utility::parseValue<Colour>(_value));

		/// @wproperty{TextBox, TextAlign, Align} Выравнивание текста.
		else if (_key == "TextAlign")
			setTextAlign(utility::parseValue<Align>(_value));

		/// @wproperty{TextBox, FontName, string} Имя шрифта.
		else if (_key == "FontName")
			setFontName(_value);

		/// @wproperty{TextBox, FontHeight, int} Высота шрифта.
		else if (_key == "FontHeight")
			setFontHeight(utility::parseValue<int>(_value));

		/// @wproperty{TextBox, Caption, string} Содержимое по? редактирован?.
		else if (_key == "Caption")
			setCaptionWithReplacing(_value);

		/// @wproperty{TextBox, TextShadowColour, Colour} Цвет тени текста.
		else if (_key == "TextShadowColour")
			setTextShadowColour(utility::parseValue<Colour>(_value));

		/// @wproperty{TextBox, TextShadow, bool} Режи?показа тени текста.
        else if (_key == "TextShadow")
            setTextShadow(utility::parseValue<bool>(_value));

        else if (_key == "AutoDisappear")
            setAutoDisappear(utility::parseValue<bool>(_value));

        else if (_key == "DisappearTime")
            setDisappearTime(utility::parseValue<float>(_value));

        else if (_key == "FadeTime")
            setFadeTime(utility::parseValue<float>(_value));

		else
		{
			Base::setPropertyOverride(_key, _value);
			return;
		}

		eventChangeProperty(this, _key, _value);
	}

    void TextBox::setAutoDisappear(bool _value)
    {
        if (mAutoDisappear == _value)
            return;

        mAutoDisappear = _value;

        if (mAutoDisappear)
        {
            Gui::getInstance().eventFrameStart += newDelegate(this, &TextBox::FrameEnd);
        } 
        else
        {
            Gui::getInstance().eventFrameStart -= newDelegate(this, &TextBox::FrameEnd);
        }
    }

    void TextBox::setDisappearTime(float _value)
    {
        mDisappearTime = _value;
    }

    void TextBox::setFadeTime(float _value)
    {
        mFadeTime = _value;
    }

    void TextBox::resetElapsedTime()
    {
        if (!mAutoDisappear) return;

        mElapsedTime = 0;
        setAlpha(ALPHA_MAX);
        setVisible(true);
    }

    void TextBox::FrameEnd(float _time)
    {
        if (!mAutoDisappear || !getVisible()) return;

        if (mFadeTime > mDisappearTime)
        {
            mFadeTime = mDisappearTime;
        }

        mElapsedTime += 1000 * _time;
        
        if(mElapsedTime >= mDisappearTime)
        {
            setVisible(false);
        }
        else if(mFadeTime > 0 && mElapsedTime > (mDisappearTime - mFadeTime))
        {
            float fRatio = (mDisappearTime - mElapsedTime) / mFadeTime;
            setAlpha(fRatio);
        }
    }

} // namespace MyGUI
