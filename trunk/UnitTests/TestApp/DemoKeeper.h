#ifndef __DEMO_KEEPER_H__
#define __DEMO_KEEPER_H__

#include "Base/BaseDemoManager.h"
#include "DataListUI.h"

namespace demo
{
	class DemoKeeper :
		public base::BaseDemoManager
	{
	public:
		DemoKeeper();

		virtual void createScene();
		virtual void destroyScene();

	private:
		virtual void setupResources();

	private:
		DataListUI* mDataListUI;
	};

} // namespace demo

#endif // __DEMO_KEEPER_H__
