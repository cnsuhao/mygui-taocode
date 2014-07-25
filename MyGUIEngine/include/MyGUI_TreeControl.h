/*!
	@file
	@author     Pavel Turin
	@date       08/2009
*/
#ifndef __MYGUI_TREE_CONTROL_H__
#define __MYGUI_TREE_CONTROL_H__

#include "MyGUI_Widget.h"
#include "MyGUI_GenericNode.h"

namespace MyGUI
{
	class TreeControlItem;

	class MYGUI_EXPORT TreeControl :
		public Widget
	{
		MYGUI_RTTI_DERIVED( TreeControl )

	public:
		class Node;

		typedef delegates::CMultiDelegate2<TreeControl*, Node*> EventHandle_TreeControlPtrNodePtr;
		typedef delegates::CMultiDelegate2<TreeControl*, size_t> EventHandle_TreeControlPtrSizeT;

		class MYGUI_EXPORT Node :
			public GenericNode<Node, TreeControl>
		{
		public:
			Node();
			Node(TreeControl* pOwner);
			Node(const UString& strText, Node* pParent = nullptr);
			virtual ~Node();

			bool isPrepared() const;
			void setPrepared(bool bIsPrepared);
			void prepare();
			size_t prepareChildren();

			bool isExpanded() const;
			void setExpanded(bool bIsExpanded);

            TreeControlItem* getItemWidget();
            void setItemWidget(TreeControlItem* pItemWidget);

			void setData(Any Data);
			template <typename TYPE>
			TYPE* getData() const;

            Node* getChild(size_t nIndex);
			int getLevel() const;

		private:
			bool mbIsPrepared;
			bool mbIsExpanded;
            TreeControlItem* mpItemWidget;
			Any mData;
		};

		typedef Node::VectorGenericNodePtr VectorNodePtr;

		TreeControl();

		Node* getRoot() const;
		void setRootVisible(bool bValue);
		bool isRootVisible() const;

        void registerItemWidget(TreeControlItem* pItem);

		Node* getSelection() const;
		void setSelection(Node* pSelection);

		void invalidate();

		virtual void setSize(const IntSize& Size);
		virtual void setCoord(const IntCoord& Bounds);

        int getLevelOffset()
        {
            return mnLevelOffset;
        }

		bool getAutoResizeItem() const;
		void setAutoResizeItem(bool bIsAutoResizeItem);
		bool getAutoScrollToBottom() const;
		void setAutoScrollToBottom(bool bIsAutoScrollToBottom);
		TreeControlItem* AddItem(Node* _pareant, const std::string& _skin, int _itemHeight);
		virtual void setPropertyOverride(const std::string& _key, const std::string& _value);

		EventHandle_TreeControlPtrNodePtr eventTreeNodeMouseSetFocus;
		EventHandle_TreeControlPtrNodePtr eventTreeNodeMouseLostFocus;
		EventHandle_TreeControlPtrNodePtr eventTreeNodeSelected;
		EventHandle_TreeControlPtrNodePtr eventTreeNodeActivated;
		EventHandle_TreeControlPtrNodePtr eventTreeNodeContextMenu;
		EventHandle_TreeControlPtrNodePtr eventTreeNodePrepare;
		EventHandle_TreeControlPtrSizeT eventTreeScrolled;

	protected:
		virtual void initialiseOverride();
		virtual void shutdownOverride();

		void notifyMousePressed(Widget* pSender, int nLeft, int nTop, MouseButton nID);
        void notifyMouseReleased(Widget* pSender, int nLeft, int nTop, MouseButton nID);
        void notifyMouseDrag(Widget* _sender, int _left, int _top, MouseButton _id);
		void notifyMouseWheel(Widget* pSender, int nValue);
		void notifyMouseDoubleClick(Widget* pSender);
		void notifyMouseSetFocus(Widget* pSender, Widget* pPreviousWidget);
		void notifyMouseLostFocus(Widget* pSender, Widget* pNextWidget);
		void notifyScrollChangePosition(ScrollBar* pSender, size_t nPosition);
		void notifyExpandCollapse(Widget* pSender);
		void notifyFrameEntered(float nTime);

		virtual void onMouseWheel(int nValue);
		virtual void onKeyButtonPressed(KeyCode Key, Char Character);

	private:

		void updateScroll();
        void updateItemWidgets();

		void scrollTo(size_t nPosition);
		void sendScrollingEvents(size_t nPosition);

		ScrollBar* mpWidgetScroll;
		bool mbScrollAlwaysVisible;
		bool mbInvalidated;
		bool mbRootVisible;
		int mnWheelStep;
		int mnScrollRange;
		Node* mpFocusNode;
		Node* mpSelection;
		Node* mpRoot;
        int mnLevelOffset;
		bool mbIsAutoResizeItem;
		bool mbIsAutoScrollToBottom;
		bool mbNeedScrollToBottom;

        int mnDragBound;
        int mnPreDragScrollPos;

        Widget* mClient;
        Widget* mItemContainer;
	};


	inline TreeControl::Node::Node() :
		mbIsPrepared(false),
		mbIsExpanded(true),
        mpItemWidget(nullptr)
	{ }
	inline bool TreeControl::Node::isPrepared() const
	{
		return mbIsPrepared;
	}
	inline void TreeControl::Node::setPrepared(bool bIsPrepared)
	{
		mbIsPrepared = bIsPrepared;
	}
	inline bool TreeControl::Node::isExpanded() const
	{
		return mbIsExpanded;
	}
    inline TreeControlItem* TreeControl::Node::getItemWidget()
    {
        return mpItemWidget;
    }
    inline void TreeControl::Node::setData(Any Data)
    {
        mData = Data;
    }
    inline TreeControl::Node* TreeControl::Node::getChild(size_t nIndex)
    {

        if (!isPrepared())
            prepareChildren();

        MYGUI_DEBUG_ASSERT(nIndex < mChildren.size(), "Wrong Index to get TreeControlNode!");
        return mChildren[nIndex];
        
    }
	template <typename TYPE>
	TYPE* TreeControl::Node::getData() const
	{
		return mData.castType<TYPE>(true);
	}

	inline int TreeControl::Node::getLevel() const
	{
		int nLevel = 0;
		Node* pNode = getParent();
		while(pNode)
		{
			nLevel++;
			pNode = pNode->getParent();
		}
		return nLevel;
	}

	inline TreeControl::Node* TreeControl::getRoot() const
	{
		return mpRoot;
	}
	inline bool TreeControl::isRootVisible() const
	{
		return mbRootVisible;
	}
    inline TreeControl::Node* TreeControl::getSelection() const
    {
        return mpSelection;
    }
	inline bool TreeControl::getAutoResizeItem() const
	{
		return mbIsAutoResizeItem;
	}
	inline void TreeControl::setAutoResizeItem(bool bIsAutoResizeItem)
	{
		mbIsAutoResizeItem = bIsAutoResizeItem;
	}
	inline bool TreeControl::getAutoScrollToBottom() const
	{
		return mbIsAutoScrollToBottom;
	}
	inline void TreeControl::setAutoScrollToBottom(bool bIsAutoScrollToBottom)
	{
		mbIsAutoScrollToBottom = bIsAutoScrollToBottom;
	}
}

#endif // __TREE_CONTROL_H__
