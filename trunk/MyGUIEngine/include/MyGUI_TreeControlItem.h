/*!
	@file
	@author     Pavel Turin
	@date       08/2009
*/
#ifndef __MYGUI_TREE_CONTROL_ITEM_H__
#define __MYGUI_TREE_CONTROL_ITEM_H__

#include "MyGUI_TreeControl.h"
#include "MyGUI_Button.h"

namespace MyGUI
{

	class MYGUI_EXPORT TreeControlItem :
		public Button
	{
		MYGUI_RTTI_DERIVED( TreeControlItem )

	public:
		TreeControlItem();

		TreeControl::Node* getNode() const;

        virtual void updateState(bool bVisible, bool bSelect);

        virtual bool canBeSelected()
        {
            return mbCanBeSelected;
        }

        Button* getButtonExpandCollapse() const
        {
            return mpButtonExpandCollapse;
        }

        virtual bool canBeExpanded()
        {
            return mpButtonExpandCollapse != nullptr;
        }

        void setCanBeSelected(bool bValue)
        {
            mbCanBeSelected = bValue;
        }

		void setAutoHideButton(bool bValue)
		{
			mbAutoHideButton = bValue;
		}

	protected:
		virtual void initialiseOverride();
		virtual void shutdownOverride();

        void notifyMouseSetFocus(Widget* pSender, Widget* pPreviousWidget);
        void notifyMouseLostFocus(Widget* pSender, Widget* pNextWidget);
        void notifyMouseWheel(Widget* pSender, int nValue);

        virtual void setPropertyOverride(const std::string& _key, const std::string& _value);

    private:
        Button* mpButtonExpandCollapse;
        bool    mbCanBeSelected;
		bool	mbAutoHideButton;

	};

}

#endif // __TREE_CONTROL_ITEM_H__
