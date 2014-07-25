/*!
	@file
	@author		Albert Semenov
	@date		01/2008
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
#include "MyGUI_ControllerRotate.h"
#include "MyGUI_Gui.h"
#include "MyGUI_InputManager.h"
#include "MyGUI_WidgetManager.h"
#include "MyGUI_Widget.h"

namespace MyGUI
{

	ControllerRotate::ControllerRotate() :
		mCenter(FloatPoint(0,0)),
		mAngle(0),
        mCoef(1),
        mTime(0),
        mElapsedTime(0),
        mRotatoSkin(NULL)
	{
	}

	ControllerRotate::~ControllerRotate()
	{
	}

	void ControllerRotate::prepareItem(Widget* _widget)
	{
		if (!_widget->getVisible())
		{
			_widget->setVisible(true);
		}

        MyGUI::ISubWidget* main = _widget->getSubWidgetMain();
        if (main->isType<RotatingSkin>())
        {
            mRotatoSkin = main->castType<RotatingSkin>();
        }

		eventPreAction(_widget, this);
	}

	bool ControllerRotate::addTime(Widget* _widget, float _time)
	{
        if (mRotatoSkin == NULL)
            return false;

        mRotatoSkin->setCenter(mCenter);

        mElapsedTime += _time;
        if (mElapsedTime >= mTime)
        {
            if (mRotatoSkin->getAngle() != mAngle)
                mRotatoSkin->setAngle(mAngle);

            eventUpdateAction(_widget, this);
            eventPostAction(_widget, this);
            return false;
        }

        float fCurAngle = mRotatoSkin->getAngle();
        if (mAngle > fCurAngle)
        {
            fCurAngle += _time*mCoef;
            if (mAngle > fCurAngle)
            {
                mRotatoSkin->setAngle(fCurAngle);
            }
            else
            {
                mRotatoSkin->setAngle(mAngle);
            }
        } 
        else if (mAngle < fCurAngle)
        {
            fCurAngle -= _time*mCoef;
            if (mAngle < fCurAngle)
            {
                mRotatoSkin->setAngle(fCurAngle);
            }
            else
            {
                mRotatoSkin->setAngle(mAngle);
            }
        }

        eventUpdateAction(_widget, this);
        return true;
	}

	void ControllerRotate::setProperty(const std::string& _key, const std::string& _value)
	{
		if (_key == "Center")
			setCenter(utility::parseValue<FloatPoint>(_value));
		else if (_key == "Angle")
			setAngle(utility::parseValue<float>(_value));
        else if (_key == "Coef")
            setCoef(utility::parseValue<float>(_value));
        else if (_key == "Time")
            setTime(utility::parseValue<float>(_value));
	}

    void ControllerRotate::setCenter(const FloatPoint& _value)
	{
		mCenter = _value;
	}

	void ControllerRotate::setAngle(float _value)
	{
		mAngle = _value;
	}

    void ControllerRotate::setCoef(float _value)
    {
        mCoef = _value;
    }

    void ControllerRotate::setTime(float _value)
    {
        mTime = _value;
    }

} // namespace MyGUI
