/*!
	@file
	@author		LiMingming
	@date		01/2013
*/

#ifndef __PROPERTY_FIELD_SELECT_SKIN_H__
#define __PROPERTY_FIELD_SELECT_SKIN_H__

#include "EditorToolTip.h"
#include "BaseLayout/BaseLayout.h"
#include "IPropertyField.h"
#include "sigslot.h"

namespace tools
{

	class PropertyFieldSelectSkin :
		public wraps::BaseLayout,
		public IPropertyField,
		public sigslot::has_slots<>
	{
	public:
		PropertyFieldSelectSkin(MyGUI::Widget* _parent);
		virtual ~PropertyFieldSelectSkin();

		virtual void initialise(const std::string& _type);

		virtual void setTarget(MyGUI::Widget* _currentWidget);
		virtual void setValue(const std::string& _value);
		virtual void setName(const std::string& _value);

		virtual void setVisible(bool _value);
		virtual bool getVisible();

		virtual MyGUI::IntSize getContentSize();
		virtual void setCoord(const MyGUI::IntCoord& _coord);

	private:
		void notifyActionSkin(const std::string& _type, const std::string& _value, bool _final);
		void notifyApplyProperties(MyGUI::Widget* _sender, bool _force);
		void notifyTryApplyProperties(MyGUI::EditBox* _sender);
		void notifyForceApplyProperties(MyGUI::EditBox* _widget);
		void notifyMouseButtonClick(MyGUI::Widget* _sender);
		
		void updateButton();
		bool isSkinExist(const std::string& _skinName);
		bool checkTemplate(const std::string& _skinName);

	protected:
		virtual bool onCheckValue();
		virtual void onAction(const std::string& _value, bool _force);

	protected:
		MyGUI::TextBox* mText;
		MyGUI::EditBox* mField;
		MyGUI::Button* mButton;
		MyGUI::Widget* mCurrentWidget;
		std::string mType;
		std::string mName;
		std::string msg;
	};

}//namespace

#endif //__PROPERTY_FIELD_SELECT_SKIN_H__
