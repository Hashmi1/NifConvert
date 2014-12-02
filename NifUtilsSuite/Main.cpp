#include "Common/Base/hkBase.h"
#include "Common/Base/System/hkBaseSystem.h"
#include "Common/Base/Memory/System/Util/hkMemoryInitUtil.h"
#include "Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h"
#include "Common/Base/keycode.cxx"
#ifdef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_ANIMATION
#endif
#ifndef HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#define HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#endif
#include "Common/Base/Config/hkProductFeatures.cxx" 
//
////-----  GLOBALS  -------------------------------------------------------------
//CNifUtilsSuiteApp	theApp;		//  The one and only CNifUtilsSuiteApp object
//
////-----  DEFINES  -------------------------------------------------------------
#define HK_MAIN_CALL		_cdecl
#define HK_MEMORY_USAGE		500000
//
#if defined( HK_ATOM )
extern "C" int __cdecl ADP_Close( void );
#endif

# include "Common\Nif\NifConvertUtility.h"
# include <iostream>
# include <string>
using namespace std;

int main()
{
	NifConvertUtility utility;

	string line;
	cout << " Hello World" << endl;
	getline(cin,line);
	
	return 0;
}