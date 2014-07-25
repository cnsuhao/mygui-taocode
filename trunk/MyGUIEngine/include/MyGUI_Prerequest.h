/*!
	@file
	@author		Denis Koronchik
	@author		Georgiy Evmenov
	@author		Ну и я чуть чуть =)
	@date		09/2007
*/
/*
	This file is part of MyGUI.

	MyGUI is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	MyGUI is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with MyGUI.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MYGUI_PREREQUEST_H__
#define __MYGUI_PREREQUEST_H__

#include "MyGUI_Platform.h"

#if MYGUI_COMPILER == MYGUI_COMPILER_MSVC
#	ifdef MYGUI_CHECK_MEMORY_LEAKS
#		define _CRTDBG_MAP_ALLOC
#		include <crtdbg.h>
#	endif
#endif

#define MYGUI_DEFINE_VERSION(major, minor, patch) ((major << 16) | (minor << 8) | patch)

#ifndef MYGUI_DONT_REPLACE_NULLPTR
	#if MYGUI_COMPILER == MYGUI_COMPILER_MSVC
		#ifndef _MANAGED
			#ifndef _NATIVE_NULLPTR_SUPPORTED
				#define nullptr 0
			#endif
		#endif
	#else
		#define nullptr 0
	#endif
#endif

#if MYGUI_COMPILER == MYGUI_COMPILER_MSVC

#pragma warning (push)
#pragma warning (disable : 4251)
#pragma warning (disable : 4275)
#pragma warning (disable : 4290)
#pragma warning (disable : 4661)
#pragma warning (disable : 4819)
#pragma warning (disable : 4996)

#endif

#ifdef MYGUI_RENDER_UE
#include "UEMalloc.h"
#endif

namespace MyGUI
{
	class Gui;
	class LogManager;
	class InputManager;
	class SubWidgetManager;
	class LayerManager;
	class SkinManager;
	class WidgetManager;
	class FontManager;
	class ControllerManager;
	class PointerManager;
	class ClipboardManager;
	class LayoutManager;
	class PluginManager;
	class DynLibManager;
	class LanguageManager;
	class ResourceManager;
	class RenderManager;
	class FactoryManager;
	class TextureManager;
	class ToolTipManager;
    class ITexture;

	class Widget;
	class Button;
	class Window;
	class ListBox;
	class EditBox;
	class ComboBox;
	class TextBox;
	class TabControl;
	class TabItem;
	class ProgressBar;
	class ItemBox;
	class MultiListBox;
	class MultiListItem;
	class ImageBox;
	class MenuControl;
	class MenuItem;
	class PopupMenu;
	class MenuBar;
	class ScrollBar;
	class ScrollView;
	class DDContainer;
	class Canvas;

	// Define version
	#define MYGUI_VERSION_MAJOR 3
	#define MYGUI_VERSION_MINOR 2
	#define MYGUI_VERSION_PATCH 0

	#define MYGUI_VERSION    MYGUI_DEFINE_VERSION(MYGUI_VERSION_MAJOR, MYGUI_VERSION_MINOR, MYGUI_VERSION_PATCH)

	// Disable warnings for MSVC compiler
#if MYGUI_COMPILER == MYGUI_COMPILER_MSVC

// disable: "<type> needs to have dll-interface to be used by clients'
// Happens on STL member variables which are not public therefore is ok
#	pragma warning (disable : 4251)

// also some warnings are disabled in CMake

#endif

} // namespace MyGUI

#include "MyGUI_DeprecatedTypes.h"

#include <string>
#include <list>
#include <set>
#include <map>
#include <vector>
#include <deque>
#include <exception>
#include <math.h>

#ifdef MYGUI_CUSTOM_ALLOCATOR
#    include "MyGUI_CustomAllocator.h"
#else // MYGUI_CUSTOM_ALLOCATOR
#    include "MyGUI_Allocator.h"
#endif // MYGUI_CUSTOM_ALLOCATOR

// этот дефайн для того чтобы в самом гуе показывалось имя файла где вызывается new
#if MYGUI_COMPILER == MYGUI_COMPILER_MSVC
#	ifdef MYGUI_CHECK_MEMORY_LEAKS
#		define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#		define new DEBUG_NEW
#	endif
#endif

namespace MyGUI
{ 
    template <typename T, typename A = Allocator<T> > 
    struct deque 
    { 
        typedef typename std::deque<T, A> type;    
        typedef typename std::deque<T, A>::iterator iterator;
        typedef typename std::deque<T, A>::const_iterator const_iterator;
    }; 

    template <typename T, typename A = Allocator<T> > 
    struct vector 
    { 
        typedef typename std::vector<T, A> type;
        typedef typename std::vector<T, A> AVector;
        typedef typename std::vector<T, A>::iterator iterator;
        typedef typename std::vector<T, A>::const_iterator const_iterator;
    }; 

    template <typename T, typename A = Allocator<T> > 
    struct list 
    { 
        typedef typename std::list<T, A> type;
        typedef typename std::list<T, A>::iterator iterator;
        typedef typename std::list<T, A>::const_iterator const_iterator;
    }; 

    template <typename T, typename P = std::less<T>, typename A = Allocator<T> > 
    struct set 
    { 
        typedef typename std::set<T, P, A> type;
        typedef typename std::set<T, P, A>::iterator iterator;
        typedef typename std::set<T, P, A>::const_iterator const_iterator;
    }; 

    template <typename K, typename V, typename P = std::less<K>, typename A = Allocator<std::pair<const K, V> > > 
    struct map 
    { 
        typedef typename std::map<K, V, P, A> type;
        typedef typename std::map<K, V, P, A>::iterator iterator;
        typedef typename std::map<K, V, P, A>::const_iterator const_iterator;
    }; 

    template <typename K, typename V, typename P = std::less<K>, typename A = Allocator<std::pair<const K, V> > > 
    struct multimap 
    { 
        typedef typename std::multimap<K, V, P, A> type;
        typedef typename std::multimap<K, V, P, A>::iterator iterator;
        typedef typename std::multimap<K, V, P, A>::const_iterator const_iterator;
    }; 

}

#endif // __MYGUI_PREREQUEST_H__
