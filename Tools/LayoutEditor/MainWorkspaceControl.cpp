#include "Precompiled.h"
#include "MainWorkspaceControl.h"

namespace tools
{

	MainWorkspaceControl::MainWorkspaceControl(MyGUI::Widget* _parent) :
		SeparatorPartControl("MainWorkspaceControl.layout", _parent),
		mToolsControl(nullptr),
		mWorkspaceControl(nullptr),
		mWidgetHierarchy(nullptr)
	{
		assignBase(mToolsControl, "ToolsControl");
		assignBase(mWorkspaceControl, "WorkspaceControl");
		assignBase(mWidgetHierarchy, "HierarchyControl");
	}

	MainWorkspaceControl::~MainWorkspaceControl()
	{
	}

}
