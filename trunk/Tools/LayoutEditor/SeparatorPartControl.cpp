#include "Precompiled.h"
#include "SeparatorPartControl.h"
#include "CommandManager.h"

namespace tools
{

	SeparatorPartControl::SeparatorPartControl(const std::string& _layout, MyGUI::Widget* _parent) :
		wraps::BaseLayout(_layout, _parent),
		mMiddlePanel(nullptr),
		mRightPanel(nullptr),
		mRightSeparatorH(nullptr),
		mLeftSeparatorH(nullptr),
		mLeftPanel(nullptr),
		mCurOprLeftPanel(nullptr),
		mCurOprRightPanel(nullptr),
		mCurOprSeparatorH(nullptr),
		mMinSizeLeft(0),
		mMinSizeRight(0)
	{
		assignWidget(mLeftPanel, "Left");
		assignWidget(mMiddlePanel, "Middle");
		assignWidget(mRightPanel, "Right");
		assignWidget(mLeftSeparatorH, "LeftSeparatorH");
		assignWidget(mRightSeparatorH, "RightSeparatorH");
		
		

		mMinSizeLeft = MyGUI::utility::parseValue<int>(mMiddlePanel->getUserString("MinSize"));
		mMinSizeRight = MyGUI::utility::parseValue<int>(mRightPanel->getUserString("MinSize"));

		mRightSeparatorH->eventMouseButtonPressed += MyGUI::newDelegate(this, &SeparatorPartControl::notifyMouseButtonPressed);
		mRightSeparatorH->eventMouseDrag += MyGUI::newDelegate(this, &SeparatorPartControl::notifyMouseDrag);

		mLeftSeparatorH->eventMouseButtonPressed += MyGUI::newDelegate(this, &SeparatorPartControl::notifyMouseButtonPressed);
		mLeftSeparatorH->eventMouseDrag += MyGUI::newDelegate(this, &SeparatorPartControl::notifyMouseDrag);

		CommandManager::getInstance().getEvent("Command_ShowHierarchy")->connect(this, &SeparatorPartControl::CommandShowHierarchy);

		SetHierarchyVisible(false);
	}

	SeparatorPartControl::~SeparatorPartControl()
	{
		mRightSeparatorH->eventMouseButtonPressed -= MyGUI::newDelegate(this, &SeparatorPartControl::notifyMouseButtonPressed);
		mRightSeparatorH->eventMouseDrag -= MyGUI::newDelegate(this, &SeparatorPartControl::notifyMouseDrag);

		mLeftSeparatorH->eventMouseButtonPressed -= MyGUI::newDelegate(this, &SeparatorPartControl::notifyMouseButtonPressed);
		mLeftSeparatorH->eventMouseDrag -= MyGUI::newDelegate(this, &SeparatorPartControl::notifyMouseDrag);
	}

	void SeparatorPartControl::notifyMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
	{

		if (_id == MyGUI::MouseButton::Left)
		{
			if (_sender == mLeftSeparatorH)
			{
				mCurOprSeparatorH = mLeftSeparatorH;
				mCurOprLeftPanel = mLeftPanel;
				mCurOprRightPanel = mMiddlePanel;

			}
			else if (_sender == mRightSeparatorH)
			{
				mCurOprSeparatorH = mRightSeparatorH;
				mCurOprLeftPanel = mMiddlePanel;
				mCurOprRightPanel = mRightPanel;
			}
			mLastMousePosition = MyGUI::InputManager::getInstance().getMousePosition();
		}

		
	}

	void SeparatorPartControl::notifyMouseDrag(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id)
	{

		if (_id == MyGUI::MouseButton::Left)
		{
			MyGUI::IntPoint mousePosition = MyGUI::InputManager::getInstance().getMousePosition();
			int delta = mousePosition.left - mLastMousePosition.left;
			mLastMousePosition = mousePosition;

			MyGUI::IntCoord leftPanel = mCurOprLeftPanel->getCoord();
			MyGUI::IntCoord rightPanel = mCurOprRightPanel->getCoord();
			MyGUI::IntCoord separatorHPanel = mCurOprSeparatorH->getCoord();

			leftPanel.width += delta;
			separatorHPanel.left += delta;
			rightPanel.left += delta;
			rightPanel.width -= delta;

			int diffLeft = mMinSizeLeft - leftPanel.width;
			if (diffLeft > 0)
			{
				leftPanel.width += diffLeft;
				separatorHPanel.left += diffLeft;
				rightPanel.left += diffLeft;
				rightPanel.width -= diffLeft;
			}

			int diffRight = mMinSizeRight - rightPanel.width;
			if (diffRight > 0)
			{
				leftPanel.width -= diffRight;
				separatorHPanel.left -= diffRight;
				rightPanel.left -= diffRight;
				rightPanel.width += diffRight;
			}

			mCurOprLeftPanel->setCoord(leftPanel);
			mCurOprRightPanel->setCoord(rightPanel);
			mCurOprSeparatorH->setCoord(separatorHPanel);
		}
		
	}


	void SeparatorPartControl::CommandShowHierarchy(const MyGUI::UString& _commandName, bool& _result)
	{	
		SetHierarchyVisible(!mLeftPanel->getVisible());
	}

	void SeparatorPartControl::SetHierarchyVisible(bool bVisible)
	{
		if (bVisible)
		{
			if (!mLeftPanel->getVisible())
			{
				mLeftPanel->setVisible(true);
				mLeftSeparatorH->setVisible(true);

				MyGUI::IntCoord leftPanel = mMiddlePanel->getCoord();
				leftPanel.left = mLeftPanel->getCoord().left + mLeftPanel->getCoord().width + mLeftSeparatorH->getCoord().width ;
				leftPanel.width -= mLeftPanel->getCoord().width + mLeftSeparatorH->getCoord().width;

				mMiddlePanel->setCoord(leftPanel);
			}
		}
		else
		{
			if (mLeftPanel->getVisible())
			{
				mLeftPanel->setVisible(false);
				mLeftSeparatorH->setVisible(false);

				MyGUI::IntCoord leftPanel = mMiddlePanel->getCoord();
				leftPanel.left = mLeftPanel->getCoord().left;
				leftPanel.width += mLeftPanel->getCoord().width + mLeftSeparatorH->getCoord().width;

				mMiddlePanel->setCoord(leftPanel);
			}
		}

	}
}
