/*!
	@file
	@author		Albert Semenov
	@date		03/2008
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
#include "MyGUI_ActionController.h"
#include "MyGUI_Widget.h"
#include "MyGUI_WidgetManager.h"

namespace MyGUI
{

	namespace action
	{

		void actionWidgetHide(Widget* _widget, ControllerItem* _controller)
		{
			_widget->setVisible(false);
		}

		void actionWidgetShow(Widget* _widget, ControllerItem* _controller)
		{
			_widget->setVisible(true);
		}

		void actionWidgetDestroy(Widget* _widget, ControllerItem* _controller)
		{
			WidgetManager::getInstance().destroyWidget(_widget);
		}

		void linearMoveFunction(const IntCoord& _startRect, const IntCoord& _destRect, IntCoord& _result, float _k)
		{
			_result.set(
				_startRect.left   - int( float(_startRect.left   - _destRect.left)   * _k ),
				_startRect.top    - int( float(_startRect.top    - _destRect.top)    * _k ),
				_startRect.width  - int( float(_startRect.width  - _destRect.width)  * _k ),
				_startRect.height - int( float(_startRect.height - _destRect.height) * _k )
			);
		}

        void frictionalMoveFunction(const IntCoord& _startRect, const IntCoord& _destRect, const IntCoord& _controlRect, IntCoord& _result, float _current_time)
        {
            float k = (2 - _current_time)*_current_time;
            linearMoveFunction(_startRect, _destRect, _result, k);
        }

		void inertionalMoveFunction(const IntCoord& _startRect, const IntCoord& _destRect, const IntCoord& _controlRect, IntCoord& _result, float _current_time)
		{
#ifndef M_PI
			const float M_PI = 3.141593f;
#endif
			float k = sin(M_PI * _current_time - M_PI / 2.0f);
			if (k < 0) k = (-pow(-k, 0.7f) + 1) / 2;
			else k = (pow(k, 0.7f) + 1) / 2;
			linearMoveFunction(_startRect, _destRect, _result, k);
		}

        void curveMoveFunction(const IntCoord& _startRect, const IntCoord& _destRect, const IntCoord& _controlRect, IntCoord& _result, float _current_time)
        {
            _result.left  = int(pow(1-_current_time, 2) * _startRect.left + 2 *_current_time * (1 - _current_time) * _controlRect.left + pow(_current_time, 2) * _destRect.left);
            _result.top   = int(pow(1-_current_time, 2) * _startRect.top + 2 * _current_time * (1 - _current_time) * _controlRect.top + pow(_current_time, 2) * _destRect.top);
            _result.width = _startRect.width + (_destRect.width - _startRect.width)*_current_time;
            _result.height = _startRect.height + (_destRect.height - _startRect.height)*_current_time;
        }

	} // namespace action

} // namespace MyGUI
