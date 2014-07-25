/*!
	@file
	@author     Pavel Turin
	@date       08/2009
*/

#include "MyGUI_Precompiled.h"
#include "MyGUI_TreeControl.h"
#include "MyGUI_TreeControlItem.h"
#include "MyGUI.h"

namespace MyGUI
{
	TreeControl::Node::Node(TreeControl* pOwner) :
		GenericNode<Node, TreeControl>(pOwner),
		mbIsPrepared(false),
		mbIsExpanded(true),
        mpItemWidget(nullptr)
	{
	}

	TreeControl::Node::Node(const UString& strText, Node* pParent) :
		GenericNode<Node, TreeControl>(strText, pParent),
		mbIsPrepared(false),
		mbIsExpanded(false),
        mpItemWidget(nullptr)
	{
	}

	TreeControl::Node::~Node()
	{
        if (mpItemWidget)
        {
            mpItemWidget->setVisible(false);
        }
	}

	void TreeControl::Node::prepare()
	{
		if (mbIsPrepared || !mpOwner)
			return;

		mpOwner->eventTreeNodePrepare(mpOwner, this);
		mbIsPrepared = true;
	}

	size_t TreeControl::Node::prepareChildren()
	{
		prepare();

		size_t nResult = 0;

        VectorNodePtr& NodeVector = getChildren();
		for (VectorNodePtr::iterator Iterator = NodeVector.begin(); Iterator != NodeVector.end(); ++Iterator)
		{
			TreeControl::Node* pChild = *Iterator;

			nResult++;

			pChild->prepare();
			if (pChild->isExpanded())
				nResult += pChild->prepareChildren();
		}

		return nResult;
	}

	void TreeControl::Node::setExpanded(bool bIsExpanded)
	{
		if (mbIsExpanded == bIsExpanded)
			return;

		mbIsExpanded = bIsExpanded;

		invalidate();
	}

    void TreeControl::Node::setItemWidget(TreeControlItem* pItemWidget)
    {
        if (mpItemWidget)
        {
            mpItemWidget->setVisible(false);
        }

        mpItemWidget = pItemWidget;

        pItemWidget->setUserData(this);

        invalidate();
    }	
    
	TreeControl::TreeControl() :
		mpWidgetScroll(nullptr),
		mbScrollAlwaysVisible(true),
		mbInvalidated(false),
		mbRootVisible(false),
		mnWheelStep(1),
		mnScrollRange(-1),
		mpFocusNode(nullptr),
		mpSelection(nullptr),
		mpRoot(nullptr),
        mnLevelOffset(0),
        mnDragBound(0),
        mnPreDragScrollPos(0),
        mClient(nullptr),
        mItemContainer(nullptr),
		mbIsAutoResizeItem(true),
		mbIsAutoScrollToBottom(false),
		mbNeedScrollToBottom(false)
	{
	}

	void TreeControl::initialiseOverride()
	{
		Base::initialiseOverride();

		// FIXME перенесенн?из конструктора, проверит?смен?скин?
		mpRoot = new Node(this);

		//FIXME
		setNeedKeyFocus(true);

		assignWidget(mpWidgetScroll, "VScroll");
		if (mpWidgetScroll != nullptr)
		{
			mpWidgetScroll->eventScrollChangePosition += newDelegate(this, &TreeControl::notifyScrollChangePosition);
			mpWidgetScroll->eventMouseButtonPressed += newDelegate(this, &TreeControl::notifyMousePressed);
		}

		assignWidget(mClient, "Client");
		if (mClient != nullptr)
		{
			mClient->eventMouseButtonPressed += newDelegate(this, &TreeControl::notifyMousePressed);
			setWidgetClient(mClient);
		}

        assignWidget(mItemContainer, "ItemContainer");
        if (mItemContainer != nullptr)
        {
            mItemContainer->eventMouseWheel += newDelegate(this, &TreeControl::notifyMouseWheel);
            mItemContainer->eventMouseButtonPressed += newDelegate(this, &TreeControl::notifyMousePressed);
            mItemContainer->eventMouseButtonReleased += newDelegate(this, &TreeControl::notifyMouseReleased);
            mItemContainer->eventMouseDrag += newDelegate(this, &TreeControl::notifyMouseDrag);
        }

		MYGUI_ASSERT(nullptr != mpWidgetScroll, "Child VScroll not found in skin (TreeControl must have VScroll)");
        MYGUI_ASSERT(nullptr != mClient, "Child Widget Client not found in skin (TreeControl must have Client)");
        MYGUI_ASSERT(nullptr != mItemContainer, "Child Widget ItemContainer not found in skin (TreeControl must have ItemContainer)");

        float fZoomScale = Gui::getInstance().getZoomScale();
		if (isUserString("WheelStep"))
			mnWheelStep = utility::parseValue<int>(getUserString("WheelStep"));
        if (isUserString("LevelOffset"))
            mnLevelOffset = int(utility::parseValue<int>(getUserString("LevelOffset")) * fZoomScale);
        if (isUserString("DragBound"))
            mnDragBound = int(utility::parseValue<int>(getUserString("DragBound"))* fZoomScale);

		if (mnWheelStep < 1)
			mnWheelStep = 1;

		invalidate();
	}

	void TreeControl::shutdownOverride()
	{
		mpWidgetScroll = nullptr;
		mClient = nullptr;
        mItemContainer = nullptr;
		// FIXME перенесенн?из деструктор? проверит?смен?скин?
		delete mpRoot;

        Gui::getInstance().eventFrameStart -= newDelegate(this, &TreeControl::notifyFrameEntered);

		Base::shutdownOverride();
	}

	void TreeControl::setRootVisible(bool bValue)
	{
		if (mbRootVisible == bValue)
			return;

		mbRootVisible = bValue;
		invalidate();
	}

    void TreeControl::registerItemWidget(TreeControlItem* pItem)
    {
        if (!pItem)
            return;

        pItem->eventMouseButtonPressed += newDelegate(this, &TreeControl::notifyMousePressed);
        pItem->eventMouseButtonReleased += newDelegate(this, &TreeControl::notifyMouseReleased);
        pItem->eventMouseDrag += newDelegate(this, &TreeControl::notifyMouseDrag);
        pItem->eventMouseButtonDoubleClick += newDelegate(this, &TreeControl::notifyMouseDoubleClick);
        pItem->eventMouseWheel += newDelegate(this, &TreeControl::notifyMouseWheel);
        pItem->eventMouseSetFocus += newDelegate(this, &TreeControl::notifyMouseSetFocus);
        pItem->eventMouseLostFocus += newDelegate(this, &TreeControl::notifyMouseLostFocus);
        if (pItem->canBeExpanded())
        {
			pItem->getButtonExpandCollapse()->eventMouseButtonClick += newDelegate(this, &TreeControl::notifyExpandCollapse);

			if (!pItem->canBeSelected())
				pItem->eventMouseButtonClick += newDelegate(this, &TreeControl::notifyExpandCollapse);
        }

    }

	void TreeControl::setSelection(Node* pSelection)
	{
		if (mpSelection == pSelection)
			return;

        if (!pSelection || !pSelection->getItemWidget() || !pSelection->getItemWidget()->canBeSelected())
        {
            mpSelection = nullptr;
            invalidate();
            return;
        }

		mpSelection = pSelection;
		if (!pSelection->getItemWidget()->canBeSelected())
		{
			pSelection->setExpanded(true);
		}

		pSelection = pSelection->getParent();

		while (pSelection)
		{
			pSelection->setExpanded(true);
			pSelection = pSelection->getParent();
		}

		invalidate();
		eventTreeNodeSelected(this, mpSelection);
	}

	void TreeControl::onMouseWheel(int nValue)
	{
		notifyMouseWheel(nullptr, nValue);

		Widget::onMouseWheel(nValue);
	}

	void TreeControl::onKeyButtonPressed(KeyCode Key, Char Character)
	{
		// TODO

		Widget::onKeyButtonPressed(Key, Character);
	}

	void TreeControl::setSize(const IntSize& Size)
	{
		Widget::setSize(Size);

		invalidate();
	}

	void TreeControl::setCoord(const IntCoord& Bounds)
	{
		Widget::setCoord(Bounds);

		invalidate();
	}

	void TreeControl::notifyFrameEntered(float nTime)
	{
		if (!mbInvalidated)
			return;

        mpRoot->prepareChildren();

		updateItemWidgets();
        updateScroll();

		mbInvalidated = false;
		Gui::getInstance().eventFrameStart -= newDelegate(this, &TreeControl::notifyFrameEntered);
	}

	void TreeControl::updateScroll()
	{
		mnScrollRange = mItemContainer->getHeight() - mClient->getHeight();

		if (!mbScrollAlwaysVisible || mnScrollRange <= 0 || mpWidgetScroll->getLeft() <= mClient->getLeft())
		{
			if (mpWidgetScroll->getVisible())
			{
				mpWidgetScroll->setVisible(false);
                mpWidgetScroll->setScrollPosition(0);
				mClient->setSize(mClient->getWidth() + mpWidgetScroll->getWidth(), mClient->getHeight());
			}
            if (mnScrollRange < 0)
                mnScrollRange = 0;
		}
		else if (!mpWidgetScroll->getVisible())
		{
			mClient->setSize(mClient->getWidth() - mpWidgetScroll->getWidth(), mClient->getHeight());
			mpWidgetScroll->setVisible(true);
        }

        mpWidgetScroll->setScrollRange(mnScrollRange + 1);
        if (mItemContainer->getHeight() > 0)
        {
            mpWidgetScroll->setTrackSize(mpWidgetScroll->getLineSize() * mClient->getHeight() / mItemContainer->getHeight());
        }

        int nPosition = mpWidgetScroll->getScrollPosition();

		if (mbNeedScrollToBottom)
		{
			nPosition = mnScrollRange;
			mpWidgetScroll->setScrollPosition(nPosition);
			mbNeedScrollToBottom = false;
		}
		else
		{
			if (nPosition > mnScrollRange)
				nPosition = mnScrollRange;
			else if (nPosition < 0)
				nPosition = 0;
		}   

        scrollTo(nPosition);

	}

    void TreeControl::updateItemWidgets()
    {

        typedef std::pair<VectorNodePtr::iterator, VectorNodePtr::iterator> PairNodeEnumeration;
        typedef std::pair<PairNodeEnumeration, bool> PairNodesVisible;
        typedef list<PairNodesVisible>::type ListNodeEnumeration;
        ListNodeEnumeration EnumerationStack;
        PairNodeEnumeration Enumeration;
        bool bNodesVisibile = true;

        VectorNodePtr vectorNodePtr;
        if (mbRootVisible)
        {
            vectorNodePtr.push_back(mpRoot);
            Enumeration = PairNodeEnumeration(vectorNodePtr.begin(), vectorNodePtr.end());
        }
        else
            Enumeration = PairNodeEnumeration(mpRoot->getChildren().begin(), mpRoot->getChildren().end());

        size_t nLevel = 0;
        int nOffset = 0;

        while (true)
        {
            if (Enumeration.first == Enumeration.second)
            {
                if (EnumerationStack.empty())
                    break;

                Enumeration = EnumerationStack.back().first;
                bNodesVisibile = EnumerationStack.back().second;
                EnumerationStack.pop_back();
                nLevel--;
                continue;
            }

            Node* pNode = *Enumeration.first;
            Enumeration.first++;

            TreeControlItem* pItem = pNode->getItemWidget();
            
            pItem->updateState(bNodesVisibile, pNode == mpSelection);

            if (bNodesVisibile)
            {				
                pItem->setPosition(IntPoint(nLevel * mnLevelOffset, nOffset));
				
                nOffset += pItem->getHeight();
            }
            
            if (pNode->hasChildren())
            {
                EnumerationStack.push_back(PairNodesVisible(Enumeration, bNodesVisibile));

                bNodesVisibile = bNodesVisibile && pNode->isExpanded();
                Enumeration.first = pNode->getChildren().begin();
                Enumeration.second = pNode->getChildren().end();
                nLevel++;
            }
        }
        nOffset = nOffset >= mClient->getHeight() ? nOffset : mClient->getHeight();
        mItemContainer->setSize(mClient->getWidth(), nOffset);
    }

	void TreeControl::invalidate()
	{
		if (mbInvalidated)
			return;

		Gui::getInstance().eventFrameStart += newDelegate(this, &TreeControl::notifyFrameEntered);
		mbInvalidated = true;
	}

	void TreeControl::scrollTo(size_t nPosition)
	{
		mItemContainer->setPosition(mItemContainer->getLeft(), -1 * (int)nPosition);

        sendScrollingEvents(nPosition);
	}

	void TreeControl::sendScrollingEvents(size_t nPosition)
	{
		eventTreeScrolled(this, nPosition);
		if (mpFocusNode)
			eventTreeNodeMouseSetFocus(this, mpFocusNode);
	}

	void TreeControl::notifyMousePressed(Widget* pSender, int nLeft, int nTop, MouseButton nID)
	{
		if ((nID == MouseButton::Left || nID == MouseButton::Right) && pSender != mpWidgetScroll)
		{
			Node* pSelection = mpSelection;
			if (pSender == mClient || pSender == mItemContainer)
				pSelection = nullptr;
			else if (pSender->getVisible())
				pSelection = *pSender->getUserData<Node*>();

			setSelection(pSelection);

            if (nID == MouseButton::Left)
                mnPreDragScrollPos = mpWidgetScroll->getScrollPosition();

            if (nID == MouseButton::Right)
				eventTreeNodeContextMenu(this, mpSelection);
		}
	}

    void TreeControl::notifyMouseReleased(Widget* pSender, int nLeft, int nTop, MouseButton nID)
    {
        if (nID == MouseButton::Left && pSender != mpWidgetScroll)
        {
            int nTopOffSet = mItemContainer->getTop();
            int nBottomOffset = mItemContainer->getBottom() - mClient->getHeight();
            if (nTopOffSet > 0)
            {
                scrollTo(0);
            }
            else if(nTopOffSet < 0 && nBottomOffset < 0)
            {
                if (nTopOffSet >= nBottomOffset)
                {
                    scrollTo(0);
                }
                else if (nTopOffSet < nBottomOffset)
                {
                    scrollTo( nBottomOffset - nTopOffSet );
                }
            }
        }
    }

    void TreeControl::notifyMouseDrag(Widget* _sender, int _left, int _top, MouseButton _id)
    {
        if (_id != MouseButton::Left)
            return;

        const IntPoint& lastPressed = InputManager::getInstance().getLastPressedPosition(MouseButton::Left);

        int nNewScrollPos = mnPreDragScrollPos - _top + lastPressed.top;
        int nDragDamping = 0;
        if (nNewScrollPos > mnScrollRange)
        {
            nDragDamping = dragDamping(nNewScrollPos - mnScrollRange, mnDragBound);
            nNewScrollPos = mnScrollRange;
        }
        else if (nNewScrollPos < 0)
        {
            nDragDamping = -1 * dragDamping(-1 * nNewScrollPos, mnDragBound);
            nNewScrollPos = 0;
        }

        mpWidgetScroll->setScrollPosition(nNewScrollPos);
        scrollTo(nNewScrollPos + nDragDamping);

    }

	void TreeControl::notifyMouseWheel(Widget* pSender, int nValue)
	{
		if (mnScrollRange <= 0)
			return;

		int nPosition = (int)mpWidgetScroll->getScrollPosition();
		if (nValue < 0)
			nPosition += mnWheelStep;
		else
			nPosition -= mnWheelStep;

		if (nPosition >= mnScrollRange)
			nPosition = mnScrollRange;
		else if (nPosition < 0)
			nPosition = 0;

		if ((int)mpWidgetScroll->getScrollPosition() == nPosition)
			return;

		mpWidgetScroll->setScrollPosition(nPosition);

		scrollTo(nPosition);

	}

	void TreeControl::notifyMouseDoubleClick(Widget* pSender)
	{
		if (mpSelection)
			eventTreeNodeActivated(this, mpSelection);
	}

	void TreeControl::notifyMouseSetFocus(Widget* pSender, Widget* pPreviousWidget)
	{
		mpFocusNode = *pSender->getUserData<Node*>();
		eventTreeNodeMouseSetFocus(this, mpFocusNode);
	}

	void TreeControl::notifyMouseLostFocus(Widget* pSender, Widget* pNextWidget)
	{
		if (!pNextWidget || (pNextWidget->getParent() != mClient))
		{
			mpFocusNode = nullptr;
			eventTreeNodeMouseLostFocus(this, nullptr);
		}
	}

	void TreeControl::notifyScrollChangePosition(ScrollBar* pSender, size_t nPosition)
	{
		scrollTo(nPosition);
	}

	void TreeControl::notifyExpandCollapse(Widget* pSender)
	{
		TreeControlItem* pItem = NULL;
		if (pSender->isType<TreeControlItem>())
			pItem = pSender->castType<TreeControlItem>(false);
		else
			pItem = pSender->getParent()->castType<TreeControlItem>(false);

		if (!pItem)
			return;

		Node* pNode = pItem->getNode();
		pNode->setExpanded(!pNode->isExpanded());

		if (!pNode->isExpanded() && mpSelection && mpSelection->hasAncestor(pNode))
		{
            setSelection(pNode);
		}

		invalidate();
	}

	void TreeControl::setPropertyOverride(const std::string& _key, const std::string& _value)
	{
		if (_key == "AutoResizeItem")
		{
			setAutoResizeItem(utility::parseBool(_value));
		}
		else if (_key == "AutoScrollToBottom")
		{
			setAutoScrollToBottom(utility::parseBool(_value));
		}
		else
		{
			Base::setPropertyOverride(_key, _value);
			return;
		}

		eventChangeProperty(this, _key, _value);
	}

	TreeControlItem* TreeControl::AddItem(Node* _pareant, const std::string& _skin, int _itemHeight)
	{
		TreeControl::Node* pNode = new TreeControl::Node();
		_pareant->add(pNode);

		int itemWidth = mItemContainer->getWidth();
		Align align = Align::Top | Align::Left;

		if (mbIsAutoResizeItem)
		{
			itemWidth = mItemContainer->getWidth() - (pNode->getLevel() - 1) *getLevelOffset();
			align = Align::Top | Align::HStretch;
		}
	
		TreeControlItem* pItemWidget = mItemContainer->createWidget<TreeControlItem>(
			_skin,
			0, 0,
			itemWidth, _itemHeight,
			align);

		pNode->setItemWidget(pItemWidget);
		registerItemWidget(pItemWidget);

		if (mbIsAutoScrollToBottom)
			mbNeedScrollToBottom = true;


		return pItemWidget;
	}
}
