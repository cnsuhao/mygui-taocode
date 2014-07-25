/*!
	@file
	@author		LiMingming
	@date		01/2013
*/

#include "Precompiled.h"
#include "PropertyFieldSelectSkin.h"
#include "Localise.h"
#include "UndoManager.h"
#include "Parse.h"
#include "CommandManager.h"
#include "WidgetSelectorManager.h"
#include "GroupMessage.h"

namespace tools
{

	const std::string DEFAULT_STRING = "[DEFAULT]";

	PropertyFieldSelectSkin::PropertyFieldSelectSkin(MyGUI::Widget* _parent) :
		BaseLayout("PropertyFieldSelectSkin.layout", _parent),
		mText(nullptr),
		mField(nullptr),
		mButton(nullptr),
		mCurrentWidget(nullptr)
	{
		assignWidget(mText, "Text");
		assignWidget(mField, "Field");
		assignWidget(mButton, "Button");

		mField->eventEditTextChange += newDelegate (this, &PropertyFieldSelectSkin::notifyTryApplyProperties);
		mField->eventEditSelectAccept += newDelegate (this, &PropertyFieldSelectSkin::notifyForceApplyProperties);

		mButton->eventMouseButtonClick += newDelegate (this, &PropertyFieldSelectSkin::notifyMouseButtonClick);

	}

	PropertyFieldSelectSkin::~PropertyFieldSelectSkin()
	{
	}

	void PropertyFieldSelectSkin::initialise(const std::string& _type)
	{
		mType = _type;
	}

	void PropertyFieldSelectSkin::setTarget(MyGUI::Widget* _currentWidget)
	{
		mCurrentWidget = _currentWidget;

		updateButton();
	}

	void PropertyFieldSelectSkin::notifyApplyProperties(MyGUI::Widget* _sender, bool _force)
	{
		std::string DEFAULT_VALUE = replaceTags("ColourDefault") + DEFAULT_STRING;

		std::string value = mField->getOnlyText();
		if (value == DEFAULT_STRING && mField->getCaption() == DEFAULT_VALUE)
			value = "";

		onAction(value, _force);

		UndoManager::getInstance().addValue(PR_PROPERTIES);
	}

	void PropertyFieldSelectSkin::onAction(const std::string& _value, bool _force)
	{
		EditorWidgets* ew = &EditorWidgets::getInstance();
		WidgetContainer* widgetContainer = ew->find(mCurrentWidget);

		bool goodData = onCheckValue();

		if (goodData)
		{
			std::istringstream str(_value);
			str >>msg;
		}
	}

	void PropertyFieldSelectSkin::notifyTryApplyProperties(MyGUI::EditBox* _sender)
	{
		notifyApplyProperties(_sender, false);
	}

	void PropertyFieldSelectSkin::notifyForceApplyProperties(MyGUI::EditBox* _sender)
	{
		notifyApplyProperties(_sender, true);
	}

	bool PropertyFieldSelectSkin::onCheckValue()
	{
		bool success = true;
     	return success;
	}

	MyGUI::IntSize PropertyFieldSelectSkin::getContentSize()
	{
		return MyGUI::IntSize(0, mMainWidget->getHeight());
	}

	void PropertyFieldSelectSkin::setCoord(const MyGUI::IntCoord& _coord)
	{
		mMainWidget->setCoord(_coord);
	}

	void PropertyFieldSelectSkin::setValue(const std::string& _value)
	{
		std::string DEFAULT_VALUE = replaceTags("ColourDefault") + DEFAULT_STRING;

		if (_value.empty())
		{
			mField->setCaption(DEFAULT_VALUE);
		}
		else
		{
			mField->setOnlyText(_value);
			onCheckValue();
		}
	}

	void PropertyFieldSelectSkin::setName(const std::string& _value)
	{
		mName = _value;
		mText->setCaption(_value);
	}

	void PropertyFieldSelectSkin::notifyMouseButtonClick(MyGUI::Widget* _sender)
	{
		notifyActionSkin("",msg,true);
	}

	
	void PropertyFieldSelectSkin::notifyActionSkin(const std::string& _type, const std::string& _value, bool _final)
	{
		if (_final)
		{
			WidgetContainer* widgetContainer = EditorWidgets::getInstance().find(mCurrentWidget);

			widgetContainer->setSkin(_value);
			if (isSkinExist(widgetContainer->getSkin()) || widgetContainer->getSkin().empty())
			{
				WidgetSelectorManager::getInstance().saveSelectedWidget();

				MyGUI::xml::Document* savedDoc = EditorWidgets::getInstance().savexmlDocument();
				EditorWidgets::getInstance().clear();
				EditorWidgets::getInstance().loadxmlDocument(savedDoc);
				delete savedDoc;

				WidgetSelectorManager::getInstance().restoreSelectedWidget();
			}
			else
			{
				std::string mess = MyGUI::utility::toString("Skin '", widgetContainer->getSkin(), "' not found. This value will be saved.");
				GroupMessage::getInstance().addMessage(mess, MyGUI::LogLevel::Error);
			}

			UndoManager::getInstance().addValue(PR_PROPERTIES);
		}
	}

	bool PropertyFieldSelectSkin::isSkinExist(const std::string& _skinName)
	{
		return _skinName == "Default" ||
			MyGUI::SkinManager::getInstance().isExist(_skinName) ||
			(MyGUI::LayoutManager::getInstance().isExist(_skinName) && checkTemplate(_skinName));
	}

	bool PropertyFieldSelectSkin::checkTemplate(const std::string& _skinName)
	{
		MyGUI::ResourceLayout* templateInfo = MyGUI::LayoutManager::getInstance().getByName(_skinName, false);
		if (templateInfo != nullptr)
		{
			const MyGUI::VectorWidgetInfo& data = templateInfo->getLayoutData();
			for (MyGUI::VectorWidgetInfo::const_iterator container = data.begin(); container != data.end(); ++container)
			{
				if (container->name == "Root")
					return true;
			}
		}

		return false;
	}


	void PropertyFieldSelectSkin::updateButton()
	{
		WidgetContainer* widgetContainer = EditorWidgets::getInstance().find(mCurrentWidget);
		mButton->setCaption(replaceTags("SelectSkin"));
	}

	void PropertyFieldSelectSkin::setVisible(bool _value)
	{
		mMainWidget->setVisible(_value);
	}

	bool PropertyFieldSelectSkin::getVisible()
	{
		return mMainWidget->getVisible();
	}

}
