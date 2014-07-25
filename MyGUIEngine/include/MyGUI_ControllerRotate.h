#ifndef __MYGUI_CONTROLLER_ROTATE_H__
#define __MYGUI_CONTROLLER_ROTATE_H__

#include "MyGUI_Prerequest.h"
#include "MyGUI_WidgetDefines.h"
#include "MyGUI_ControllerItem.h"
#include "MyGUI_RotatingSkin.h"

namespace MyGUI
{

	/** This controller used for smooth changing alpha of widget in time */
	class MYGUI_EXPORT ControllerRotate :
		public ControllerItem
	{
		MYGUI_RTTI_DERIVED( ControllerRotate )

	public:
		ControllerRotate();
		virtual ~ControllerRotate();

		void setCenter(const FloatPoint& _value);

        void setAngle(float _value);

        void setCoef(float _value);

        void setTime(float _value);

		virtual void setProperty(const std::string& _key, const std::string& _value);

	private:
		bool addTime(Widget* _widget, float _time);
		void prepareItem(Widget* _widget);

	private:
		FloatPoint mCenter;
        float mAngle;
        float mCoef;

        float mTime;
        float mElapsedTime;

        RotatingSkin* mRotatoSkin;
	};

} // namespace MyGUI

#endif // __MYGUI_CONTROLLER_ROTATE_H__
