/*!
	@file
	@author		Albert Semenov
	@date		08/2008
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
#include "MyGUI_ScrollView.h"
#include "MyGUI_SkinManager.h"
#include "MyGUI_ISubWidgetText.h"
#include "MyGUI_ScrollBar.h"
#include "MyGUI_InputManager.h"

namespace MyGUI
{

	const int SCROLL_VIEW_MOUSE_WHEEL = 50; // колличество пикселей для колеса мыши
	const int SCROLL_VIEW_SCROLL_PAGE = 16; // колличество пикселей для кнопок скрола

	ScrollView::ScrollView() :
		mContentAlign(Align::Center),
		mRealClient(nullptr),
        mAutoVScroll(false),
        mAutoHScroll(false),
        mDragBound(0)
	{
		mChangeContentByResize = false;
		mContentAlign = Align::Center;
	}

	void ScrollView::initialiseOverride()
	{
		Base::initialiseOverride();

		// FIXME нам нужен фокус клавы
		setNeedKeyFocus(true);

		///@wskin_child{ScrollView, Widget, Client} Клиентская зона.
		assignWidget(mClient, "Client");
		MyGUI::Widget* realClientOwner = this;
		if (mClient != nullptr)
		{
            mClient->eventMouseWheel += newDelegate(this, &ScrollView::notifyMouseWheel);
            mClient->eventMouseDrag += newDelegate(this, &ScrollView::notifyMouseDrag);
            mClient->eventMouseButtonPressed += newDelegate(this, &ScrollView::notifyMousePressed);
            mClient->eventMouseButtonReleased += newDelegate(this, &ScrollView::notifyMouseReleased);
			realClientOwner = mClient;
		}

		// создаем холcт, реальный владелец детей
		mRealClient = realClientOwner->createWidget<Widget>("Default", IntCoord(), Align::Default);
        mRealClient->eventMouseWheel += newDelegate(this, &ScrollView::notifyMouseWheel);
        mRealClient->eventMouseDrag += newDelegate(this, &ScrollView::notifyMouseDrag);
        mRealClient->eventMouseButtonPressed += newDelegate(this, &ScrollView::notifyMousePressed);
        mRealClient->eventMouseButtonReleased += newDelegate(this, &ScrollView::notifyMouseReleased);
		setWidgetClient(mRealClient);

		///@wskin_child{ScrollView, ScrollBar, VScroll} Вертикальная полоса прокрутки.
		assignWidget(mVScroll, "VScroll");
		if (mVScroll != nullptr)
		{
			mVScroll->eventScrollChangePosition += newDelegate(this, &ScrollView::notifyScrollChangePosition);
		}

		///@wskin_child{ScrollView, ScrollBar, HScroll} Горизонтальная полоса прокрутки.
		assignWidget(mHScroll, "HScroll");
		if (mHScroll != nullptr)
		{
			mHScroll->eventScrollChangePosition += newDelegate(this, &ScrollView::notifyScrollChangePosition);
		}

		updateView();
	}

	void ScrollView::shutdownOverride()
	{
		mVScroll = nullptr;
		mHScroll = nullptr;
		mClient = nullptr;
		mRealClient = nullptr;

		Base::shutdownOverride();
	}

	void ScrollView::setPosition(const IntPoint& _point)
	{
		Base::setPosition(_point);
	}

	void ScrollView::setSize(const IntSize& _size)
	{
		Base::setSize(_size);

		updateView();
	}

	void ScrollView::setCoord(const IntCoord& _coord)
	{
		Base::setCoord(_coord);

		updateView();
	}

	void ScrollView::notifyScrollChangePosition(ScrollBar* _sender, size_t _position)
	{
		if (mRealClient == nullptr)
			return;

		if (_sender == mVScroll)
		{
			IntPoint point = mRealClient->getPosition();
			point.top = -(int)_position;
			mRealClient->setPosition(point);
		}
		else if (_sender == mHScroll)
		{
			IntPoint point = mRealClient->getPosition();
			point.left = -(int)_position;
			mRealClient->setPosition(point);
		}
	}

	void ScrollView::notifyMouseWheel(Widget* _sender, int _rel)
	{
		if (mRealClient == nullptr)
			return;

		if (mVRange != 0)
		{
			IntPoint point = mRealClient->getPosition();
			int offset = -point.top;
			if (_rel < 0) offset += SCROLL_VIEW_MOUSE_WHEEL;
			else  offset -= SCROLL_VIEW_MOUSE_WHEEL;

			if (offset < 0) offset = 0;
			else if (offset > (int)mVRange) offset = mVRange;

			if (offset != point.top)
			{
				point.top = -offset;
				if (mVScroll != nullptr)
				{
					mVScroll->setScrollPosition(offset);
				}
				mRealClient->setPosition(point);
			}
		}
		else if (mHRange != 0)
		{
			IntPoint point = mRealClient->getPosition();
			int offset = -point.left;
			if (_rel < 0) offset += SCROLL_VIEW_MOUSE_WHEEL;
			else  offset -= SCROLL_VIEW_MOUSE_WHEEL;

			if (offset < 0) offset = 0;
			else if (offset > (int)mHRange) offset = mHRange;

			if (offset != point.left)
			{
				point.left = -offset;
				if (mHScroll != nullptr)
				{
					mHScroll->setScrollPosition(offset);
				}
				mRealClient->setPosition(point);
			}
		}
	}

	IntSize ScrollView::getContentSize()
	{
		return mRealClient == nullptr ? IntSize() : mRealClient->getSize();
	}

	IntPoint ScrollView::getContentPosition()
	{
		return mRealClient == nullptr ? IntPoint() : (IntPoint() - mRealClient->getPosition());
	}

	void ScrollView::setContentPosition(const IntPoint& _point)
	{
		if (mRealClient != nullptr)
			mRealClient->setPosition(IntPoint() - _point);
	}

	IntSize ScrollView::getViewSize()
	{
		return mClient == nullptr ? getSize() : mClient->getSize();
	}

	size_t ScrollView::getVScrollPage()
	{
		return SCROLL_VIEW_SCROLL_PAGE;
	}

	size_t ScrollView::getHScrollPage()
	{
		return SCROLL_VIEW_SCROLL_PAGE;
	}

	void ScrollView::updateView()
	{
		updateScrollSize();
		updateScrollPosition();
	}

	void ScrollView::setVisibleVScroll(bool _value)
	{
		mVisibleVScroll = _value;
		updateView();
	}

    void ScrollView::setVisibleHScroll(bool _value)
    {
        mVisibleHScroll = _value;
        updateView();
    }

	void ScrollView::setAutoVScroll(bool _value)
	{
		mAutoVScroll = _value;
        if (mAutoVScroll && mVisibleVScroll)
            setVisibleVScroll(false);
	}

    void ScrollView::setAutoHScroll(bool _value)
    {
        mAutoHScroll = _value;
        if (mAutoHScroll && mVisibleHScroll)
            setVisibleHScroll(false);
    }

    void ScrollView::setDragBound(int _value)
    {
        mDragBound = _value;
    }

	void ScrollView::setCanvasAlign(Align _value)
	{
        mContentAlign = _value;
        updateView();
	}

	void ScrollView::setCanvasSize(const IntSize& _value)
	{
		if (mRealClient != nullptr)
			mRealClient->setSize(_value);
		updateView();
	}

	IntSize ScrollView::getCanvasSize()
	{
		return mRealClient == nullptr ? IntSize() : mRealClient->getSize();
	}

	void ScrollView::setPropertyOverride(const std::string& _key, const std::string& _value)
	{
		/// @wproperty{ScrollView, VisibleVScroll, bool} Видимость вертикальной полосы прокрутки.
		if (_key == "VisibleVScroll")
			setVisibleVScroll(utility::parseValue<bool>(_value));

        else if (_key == "AutoVScroll")
            setAutoVScroll(utility::parseValue<bool>(_value));

        else if (_key == "AutoHScroll")
            setAutoHScroll(utility::parseValue<bool>(_value));

        else if (_key == "DragBound")
            setDragBound(utility::parseValue<int>(_value));

		/// @wproperty{ScrollView, VisibleHScroll, bool} Видимость горизонтальной полосы прокрутки.
		else if (_key == "VisibleHScroll")
			setVisibleHScroll(utility::parseValue<bool>(_value));

		/// @wproperty{ScrollView, CanvasAlign, Align} Выравнивание содержимого.
		else if (_key == "CanvasAlign")
			setCanvasAlign(utility::parseValue<Align>(_value));

		/// @wproperty{ScrollView, CanvasSize, int int} Размер содержимого.
		else if (_key == "CanvasSize")
			setCanvasSize(utility::parseValue<IntSize>(_value));

		else
		{
			Base::setPropertyOverride(_key, _value);
			return;
		}

		eventChangeProperty(this, _key, _value);
	}

	void ScrollView::setPosition(int _left, int _top)
	{
		setPosition(IntPoint(_left, _top));
	}

	void ScrollView::setSize(int _width, int _height)
	{
		setSize(IntSize(_width, _height));
	}

	void ScrollView::setCoord(int _left, int _top, int _width, int _height)
	{
		setCoord(IntCoord(_left, _top, _width, _height));
	}

	bool ScrollView::isVisibleVScroll() const
	{
		return mVisibleVScroll;
	}

	bool ScrollView::isVisibleHScroll() const
	{
		return mVisibleHScroll;
	}

	Align ScrollView::getCanvasAlign() const
	{
		return mContentAlign;
	}

	void ScrollView::setCanvasSize(int _width, int _height)
	{
		setCanvasSize(IntSize(_width, _height));
	}

	Align ScrollView::getContentAlign()
	{
		return mContentAlign;
	}

	void ScrollView::setViewOffset(const IntPoint& _value)
	{
		IntPoint value = _value;
		IntPoint currentOffset = mRealClient->getPosition();

		if (mHRange != 0)
		{
			if (value.left > 0)
				value.left = 0;
			else if (value.left < -(int)mHRange)
				value.left = -(int)mHRange;
		}
		else
		{
			value.left = currentOffset.left;
		}

		if (mVRange != 0)
		{
			if (value.top > 0)
				value.top = 0;
			else if (value.top < -(int)mVRange)
				value.top = -(int)mVRange;
		}
		else
		{
			value.top = currentOffset.top;
		}

		if (mHScroll != nullptr)
			mHScroll->setScrollPosition(-value.left);

		if (mVScroll != nullptr)
			mVScroll->setScrollPosition(-value.top);

		mRealClient->setPosition(value);
	}

	IntPoint ScrollView::getViewOffset() const
	{
		return mRealClient->getPosition();
	}

	IntCoord ScrollView::getViewCoord() const
	{
		return mClient == nullptr ? getCoord() : mClient->getCoord();
	}

	ScrollBar* ScrollView::getVScroll()
	{
		return mVScroll;
	}

    void ScrollView::notifyMousePressed(Widget* _sender, int _left, int _top, MouseButton _id)
    {
        if (_id != MouseButton::Left)
            return;
        mContentPosBeforeDrag = getContentPosition();
    }

    void ScrollView::notifyMouseReleased(Widget* _sender, int _left, int _top, MouseButton _id)
    {
        if (_id != MouseButton::Left)
            return;

        IntPoint contentPos = getContentPosition();

        if (contentPos.top < 0)
            contentPos.top = 0;
        else if (contentPos.top > (int)mVRange)
            contentPos.top = mVRange;

        if (contentPos.left < 0)
            contentPos.left = 0;
        else if (contentPos.left > (int)mHRange)
            contentPos.left = mHRange;

        if (contentPos != getContentPosition())
        {
            setContentPosition(contentPos);
        }

        if (isVisibleVScroll() && mAutoVScroll)
            setVisibleVScroll(false);
        if (isVisibleHScroll() && mAutoHScroll)
            setVisibleHScroll(false);
    }

    void ScrollView::notifyMouseDrag(Widget* _sender, int _left, int _top, MouseButton _id)
    {
        if (_id != MouseButton::Left)
            return;

        const IntPoint& lastPressed = InputManager::getInstance().getLastPressedPosition(MouseButton::Left);
        IntPoint contentPos = mContentPosBeforeDrag;

        if (mVRange != 0)
        {
            if (!isVisibleVScroll() && mAutoVScroll)
                setVisibleVScroll(true);

            contentPos.top = contentPos.top - _top + lastPressed.top;

            if (contentPos.top < 0)
                contentPos.top = -1 * dragDamping(-1 * contentPos.top, mDragBound);
            else if (contentPos.top > (int)mVRange)
                contentPos.top = mVRange + dragDamping(contentPos.top - mVRange, mDragBound);
        }

        if (mHRange != 0)
        {
            if (!isVisibleHScroll() && mAutoHScroll)
                setVisibleHScroll(true);

            contentPos.left = contentPos.left - _left + lastPressed.left;

            if (contentPos.left < 0)
                contentPos.left = -1 * dragDamping(-1 * contentPos.left, mDragBound);
            else if (contentPos.left > (int)mHRange)
                contentPos.left = mHRange + dragDamping(contentPos.left - mHRange, mDragBound);

        }

        if (contentPos != mContentPosBeforeDrag)
        {
            if (mVScroll != nullptr && mVRange != 0)
            {
                if (contentPos.top < 0)
                    mVScroll->setScrollPosition(0);
                else if (contentPos.top > mVRange)
                    mVScroll->setScrollPosition(mVRange);
                else
                    mVScroll->setScrollPosition(contentPos.top);
            }

            if (mHScroll != nullptr && mHRange != 0)
            {
                if (contentPos.left < 0)
                    mHScroll->setScrollPosition(0);
                else if (contentPos.left > mHRange)
                    mHScroll->setScrollPosition(mHRange);
                else
                    mHScroll->setScrollPosition(contentPos.left);
            }

            setContentPosition(contentPos);
        }
    }

} // namespace MyGUI
