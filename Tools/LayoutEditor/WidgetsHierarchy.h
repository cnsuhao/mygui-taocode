#ifndef _ZH_WIDGETSHIERARCHY_H_
#define _ZH_WIDGETSHIERARCHY_H_

#include "BaseLayout/BaseLayout.h"
#include "MyGUI_TreeControl.h"
#include "WidgetContainer.h"
#include "sigslot.h"

namespace tools
{
	class WidgetsHierarchy:
		public wraps::BaseLayout,
		public sigslot::has_slots<>
	{
	public:
		WidgetsHierarchy(MyGUI::Widget* _parent = nullptr);
		virtual ~WidgetsHierarchy();

	private:
		void updateWidgetsHierarchy();
		void createWidgetsTree(WidgetContainer* _container, MyGUI::TreeControl::Node* _parentNode, bool _print_name, bool _print_type, bool _print_skin);
		void notifyChangeWidgets();
		void notifySettingsChanged(const std::string& _path);
		void notifyChangeSelectedWidget(MyGUI::Widget* _currentWidget);
		void onClickTreeItem(MyGUI::Widget* _sender);
		void onClickVisibleCheckBox(MyGUI::Widget* _sender);
		std::string getDescriptionString(MyGUI::Widget* _widget, bool _print_name, bool _print_type, bool _print_skin);


	private:
		MyGUI::TreeControl* mWidgetsTree;
		std::map<MyGUI::Widget*, WidgetContainer*> mTreeItemToWidgetMap;

	};
}

#endif