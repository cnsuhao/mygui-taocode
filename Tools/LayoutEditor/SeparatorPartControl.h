#ifndef _2b447a49_563a_4768_84f6_f7b835381a82_
#define _2b447a49_563a_4768_84f6_f7b835381a82_

#include "BaseLayout/BaseLayout.h"
#include "sigslot.h"

namespace tools
{

	class SeparatorPartControl :
		public wraps::BaseLayout,
		public sigslot::has_slots<>
	{
	public:
		SeparatorPartControl(const std::string& _layout, MyGUI::Widget* _parent);
		virtual ~SeparatorPartControl();

	private:
		void notifyMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
		void notifyMouseDrag(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
		void CommandShowHierarchy(const MyGUI::UString& _commandName, bool& _result);
		void CommandHideHierarchy(const MyGUI::UString& _commandName, bool& _result);
		void SetHierarchyVisible(bool bVisible);

	private:
		MyGUI::Widget* mLeftPanel;
		MyGUI::Widget* mMiddlePanel;
		MyGUI::Widget* mRightPanel;
		MyGUI::Widget* mLeftSeparatorH;
		MyGUI::Widget* mRightSeparatorH;
		

		MyGUI::Widget* mCurOprSeparatorH;
		MyGUI::Widget* mCurOprLeftPanel;
		MyGUI::Widget* mCurOprRightPanel;

		

		int mMinSizeLeft;
		int mMinSizeRight;

		MyGUI::IntCoord mStartLeftPanel;
		MyGUI::IntCoord mStartRightPanel;
		MyGUI::IntCoord mStartSeparatorH;

		MyGUI::IntPoint mLastMousePosition;
	};

}

#endif
