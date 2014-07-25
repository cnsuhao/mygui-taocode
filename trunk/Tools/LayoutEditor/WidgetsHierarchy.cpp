#include "Precompiled.h"
#include "WidgetsHierarchy.h"
#include "SettingsManager.h"
#include "EditorWidgets.h"
#include "WidgetSelectorManager.h"
#include "WidgetContainer.h"
#include "Localise.h"
#include "MyGUI_TreeControlItem.h"
#include "UndoManager.h"

namespace tools
{
	WidgetsHierarchy::WidgetsHierarchy(MyGUI::Widget* _parent /* = nullptr */):
		wraps::BaseLayout("WidgetHierarchy.layout", _parent),
		mWidgetsTree(nullptr)
	{
		assignWidget(mWidgetsTree, "WidgetsTree");

		EditorWidgets::getInstance().eventChangeWidgets += MyGUI::newDelegate(this, &WidgetsHierarchy::notifyChangeWidgets);
		SettingsManager::getInstance().eventSettingsChanged.connect(this, &WidgetsHierarchy::notifySettingsChanged);
		WidgetSelectorManager::getInstance().eventChangeSelectedWidget += MyGUI::newDelegate(this, &WidgetsHierarchy::notifyChangeSelectedWidget);
	}

	WidgetsHierarchy::~WidgetsHierarchy()
	{

	}

	void WidgetsHierarchy::notifyChangeWidgets()
	{
		updateWidgetsHierarchy();
	}

	void WidgetsHierarchy::notifySettingsChanged(const std::string& _path)
	{
		if (_path == "Settings/ShowName" ||
			_path == "Settings/ShowType" ||
			_path == "Settings/ShowSkin")
			updateWidgetsHierarchy();
	}

	void WidgetsHierarchy::updateWidgetsHierarchy()
	{
		bool print_name = SettingsManager::getInstance().getValue<bool>("Settings/ShowName");
		bool print_type = SettingsManager::getInstance().getValue<bool>("Settings/ShowType");
		bool print_skin = SettingsManager::getInstance().getValue<bool>("Settings/ShowSkin");
			
		mWidgetsTree->getRoot()->removeAll(true);
		mTreeItemToWidgetMap.clear();

		EnumeratorWidgetContainer widget = EditorWidgets::getInstance().getWidgets();
		while(widget.next())
		{
			createWidgetsTree(widget.current(), mWidgetsTree->getRoot(), print_name, print_type, print_skin);
		}

	}

	void WidgetsHierarchy::createWidgetsTree(WidgetContainer* _container, MyGUI::TreeControl::Node* _parentNode, bool _print_name, bool _print_type, bool _print_skin)
	{
		bool subChild = !_container->childContainers.empty();

		MyGUI::TreeControlItem* pItemWidget = mWidgetsTree->AddItem(_parentNode, "WidgetTreeGroup", 24);
			
		MyGUI::TextBox* pMsgTextBox = pItemWidget->getSkinWidget<MyGUI::TextBox>("MessageTextBox");
		pMsgTextBox->setCaption(getDescriptionString(_container->getWidget(), _print_name, _print_type, _print_skin));
	
		MyGUI::Button* pCheckBox = pItemWidget->getSkinWidget<MyGUI::Button>("VisibleCheckBox");
		pCheckBox->eventMouseButtonClick += MyGUI::newDelegate(this, &WidgetsHierarchy::onClickVisibleCheckBox);
		pCheckBox->setStateSelected(_container->getVisibleInEditor());
		_container->getWidget()->setVisible(pCheckBox->getStateSelected());
		

		pItemWidget->eventMouseButtonClick += MyGUI::newDelegate(this, &WidgetsHierarchy::onClickTreeItem);

		mTreeItemToWidgetMap.insert(std::map<MyGUI::Widget*, WidgetContainer*>::value_type(pItemWidget, _container));
		

		if (subChild)
		{
			int i = 0;
			for (std::vector<WidgetContainer*>::iterator itr = _container->childContainers.begin(); itr != _container->childContainers.end(); ++itr)
			{
				createWidgetsTree(*itr, pItemWidget->getNode(), _print_name, _print_type, _print_skin);
				i++;
			}

		}

	}

	std::string WidgetsHierarchy::getDescriptionString(MyGUI::Widget* _widget, bool _print_name, bool _print_type, bool _print_skin)
	{
		WidgetContainer* widgetContainer = EditorWidgets::getInstance().find(_widget);

		addUserTag("WidgetName", _print_name ? widgetContainer->getName() : "");
		addUserTag("WidgetType", _print_type ? _widget->getTypeName() : "");
		addUserTag("WidgetSkin", _print_skin ? widgetContainer->getSkin() : "");

		addUserTag("FormatWidgetName", (_print_name && !widgetContainer->getName().empty()) ? "#{PatternWidgetName}" : "");
		addUserTag("FormatWidgetType", _print_type ? "#{PatternWidgetType}" : "");
		addUserTag("FormatWidgetSkin", _print_skin ? "#{PatternWidgetSkin}" : "");

		return replaceTags("MenuItemWidgetInfo");
	}

	void WidgetsHierarchy::onClickTreeItem(MyGUI::Widget* _sender)
	{
		std::map<MyGUI::Widget*, WidgetContainer*>::iterator itr = mTreeItemToWidgetMap.find(_sender);
		if (itr != mTreeItemToWidgetMap.end())
		{
			WidgetSelectorManager::getInstance().setSelectedWidget(itr->second->getWidget());
		}
	}

	void WidgetsHierarchy::notifyChangeSelectedWidget(MyGUI::Widget* _currentWidget)
	{
		MyGUI::Widget* pItemWidget = nullptr;
		MyGUI::TreeControlItem* pItem = nullptr;
		std::map<MyGUI::Widget*, WidgetContainer*>::iterator itr = mTreeItemToWidgetMap.begin();

		for (;itr != mTreeItemToWidgetMap.end(); itr++)
		{
			if (itr->second->getWidget() == _currentWidget)
			{
				pItemWidget = itr->first;
			}
		}

		if (pItemWidget != nullptr)
		{
			pItem = pItemWidget->castType<MyGUI::TreeControlItem>(false);
			mWidgetsTree->setSelection(pItem->getNode());
		}

	}

	void WidgetsHierarchy::onClickVisibleCheckBox(MyGUI::Widget* _sender)
	{
		MyGUI::Button* pChecBox = (_sender->castType<MyGUI::Button>(false));
		pChecBox->setStateSelected(!pChecBox->getStateSelected());
		std::map<MyGUI::Widget*, WidgetContainer*>::iterator itr = mTreeItemToWidgetMap.find(_sender->getParent());
		if (itr != mTreeItemToWidgetMap.end())
		{
			itr->second->setVisibleInEditor(pChecBox->getStateSelected());
			itr->second->getWidget()->setVisible(pChecBox->getStateSelected());
			UndoManager::getInstance().setUnsaved(true);
		}
	}
}	