/*!
	@file
	@author     Pavel Turin
	@date       08/2009
*/

#include "MyGUI_Precompiled.h"
#include "MyGUI_TreeControlItem.h"

namespace MyGUI
{
    TreeControlItem::TreeControlItem():
        mpButtonExpandCollapse(nullptr),
        mbCanBeSelected(false),
		mbAutoHideButton(false)
	{
	}

	void TreeControlItem::initialiseOverride()
	{
		Base::initialiseOverride();

        assignWidget(mpButtonExpandCollapse, "ButtonExpandCollapse");
        if (mpButtonExpandCollapse != nullptr)
        {
            mpButtonExpandCollapse->eventMouseSetFocus += newDelegate(this, &TreeControlItem::notifyMouseSetFocus);
            mpButtonExpandCollapse->eventMouseLostFocus += newDelegate(this, &TreeControlItem::notifyMouseLostFocus);
            mpButtonExpandCollapse->eventMouseWheel += newDelegate(this, &TreeControlItem::notifyMouseWheel);
        }

	}

	void TreeControlItem::shutdownOverride()
	{
		Base::shutdownOverride();
	}

	TreeControl::Node* TreeControlItem::getNode() const
	{
		return *(const_cast<TreeControlItem*>(this)->getUserData<TreeControl::Node*>());
	}

    void TreeControlItem::updateState(bool bVisible, bool bSelect)
    {

        setVisible(bVisible);
        if (!bVisible)
            return;

        if (canBeSelected())
        {
            setStateSelected(bSelect);
        }

        if (mpButtonExpandCollapse != nullptr)
        {
            TreeControl::Node* pNode = getNode();
            mpButtonExpandCollapse->setStateSelected(!pNode->isExpanded());

			if (mbAutoHideButton)
			{
				if (!pNode->hasChildren())
				{
					mpButtonExpandCollapse->setVisible(false);
				}
				else
				{
					mpButtonExpandCollapse->setVisible(true);
				}
			}
	
        }

    }

    void TreeControlItem::notifyMouseSetFocus(Widget* pSender, Widget* pPreviousWidget)
    {
        if (pSender && pSender->getParent() == this)
            onMouseSetFocus(pPreviousWidget);
    }

    void TreeControlItem::notifyMouseLostFocus(Widget* pSender, Widget* pNextWidget)
    {
        if (pSender && pSender->getParent() == this)
            onMouseLostFocus(pNextWidget);
    }

    void TreeControlItem::notifyMouseWheel(Widget* pSender, int nValue)
    {
        if (pSender && pSender->getParent() == this)
            onMouseWheel(nValue);
    }

    void TreeControlItem::setPropertyOverride(const std::string& _key, const std::string& _value)
    {
        if (_key == "CanBeSelected")
            setCanBeSelected(utility::parseValue<bool>(_value));
		else if (_key == "AutoHideButton")
		{
			setAutoHideButton(utility::parseBool(_value));
		}
        else
        {
            Base::setPropertyOverride(_key, _value);
            return;
        }

        eventChangeProperty(this, _key, _value);
    }


}
