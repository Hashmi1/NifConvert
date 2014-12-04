# include <windows.h>
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
# include "Common\Nif\NifCollisionUtility.h"
#include "Common\Nif\MaterialTypeHandling.h"
# include <iostream>
# include <string>
using namespace std;

string GetExeFileName()
{
  char buffer[MAX_PATH];
  GetModuleFileNameA( NULL, buffer, MAX_PATH );
  string pth = string(buffer);
  return pth.substr(0,pth.find_last_of("\\"));
}

static void HK_CALL errorReport(const char* msg, void* userArgGivenToInit)
{
	string	tString("Havok System returns error:\r\n\r\n");

	cout <<  tString  <<  msg << endl;
}

int convert_shape (string source_file, string destination_file, string tmplate, string txtr_prefix)
{
	string fl_name = destination_file.substr(destination_file.find_last_of("\\"),destination_file.length() - destination_file.find_last_of("\\"));
	cout << "Converting " << fl_name << endl;
	NifConvertUtility utility;	
	utility.setTexturePath(txtr_prefix);
	return utility.convertShape(source_file,destination_file,tmplate);
	
}

int add_collision(string source_file, string destination_file, string tmplate)
{	
	string fl_name = destination_file.substr(destination_file.find_last_of("\\"),destination_file.length() - destination_file.find_last_of("\\"));
	cout << "Adding collision to " << fl_name << endl;
	NifCollisionUtility		ncUtility(*(NifUtlMaterialList::getInstance()));
	map<int, unsigned int>	materialMap;

	MaterialTypeHandling	matHandling = MaterialTypeHandling::NCU_MT_SINGLE;
	materialMap[-1] = ((unsigned int)0xdf02f237);

	ncUtility.setCollisionNodeHandling(CollisionNodeHandling::NCU_CN_COLLISION);
	ncUtility.setMaterialTypeHandling (matHandling, materialMap);
	ncUtility.setDefaultMaterial      (materialMap[-1]);
	ncUtility.setMergeCollision       (TRUE);
	ncUtility.setReorderTriangles     (TRUE);

	return ncUtility.addCollision(source_file,destination_file,tmplate);
}

int make_door(string source_file, string destination_file, string tmplate)
{
	string fl_name = destination_file.substr(destination_file.find_last_of("\\"),destination_file.length() - destination_file.find_last_of("\\"));
	cout << "Making animated door from: " << fl_name << endl;
	NifConvertUtility utility;		
	return utility.convert_animated_door(source_file,destination_file,tmplate);
}

enum Mode
{
	NotAssigned,
	STAT,
	DOOR_ANIM,
	DOOR_LOAD

};



int main(int argc, char* argv[])
{
	
	  cout << endl << "Copyright (c) 2012-2013, NIFUtilsSuite. All rights reserved." << endl;
	  cout << "All WARRANTIES ARE HEREBY DISCLAIMED" << endl << endl;
	  
	  
	
		//  initialize Havok  (HK_MEMORY_USAGE bytes of physics solver buffer)
	hkMemoryRouter*		pMemoryRouter(hkMemoryInitUtil::initDefault(hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(HK_MEMORY_USAGE)));
	hkBaseSystem::init(pMemoryRouter, errorReport);

	Mode mode = NotAssigned;
	string txtr_prefix;
	string file_in;
	string file_out;
	
	for(int i = 0; i < argc; i++) 
	{      
	  string arg = argv[i];
	  
	  if(arg.compare("?") == 0 || arg.compare("/?") == 0 || arg.compare("-?") == 0 || arg.compare("-help") == 0 || arg.compare("/help") == 0 )
	  {
		  
		  cout << endl << "Usage: " << endl;
		  cout << "NifConvert -in input_file_name -out output_file_name -[MODE] -txtr_prefix texture_prefix" << endl;
		  cout << endl << "-MODE can be either one of:" << endl;
		  cout << "  -stat" << endl;
		  cout << "  -door_anim" << endl << endl;
		  cout << endl << "All paths must be full-qualified paths (e.g. c:\\games\\skyrim\\data\\meshes\\example.nif ) " << endl;
		  cout << "texture_prefix is the path to append to textures in the nif. e.g textures\\architecture\\" << endl;
		  return 0;
	  }

	  

	  if(arg.compare("-in") == 0)
	  {
		  file_in = argv[i+1];
		  i++;
	  }
	  else if(arg.compare("-out") == 0)
	  {
		  file_out = argv[i+1];
		  i++;
	  }
	  else if(arg.compare("-stat") == 0)
	  {
		  if (mode != NotAssigned)
		  {
			  cout << " More than one mode selected " << endl;
			  return -1;
		  }

		  mode = STAT;

	  }
	  else if(arg.compare("-door_anim") == 0)
	  {
		  if (mode != NotAssigned)
		  {
			  cout << " More than one mode selected " << endl;
			  return -1;
		  }

		  mode = DOOR_ANIM;
	  }
	  else if(arg.compare("-door_load") == 0)
	  {
		  if (mode != NotAssigned)
		  {
			  cout << " More than one mode selected " << endl;
			  return -1;
		  }

		  mode = DOOR_LOAD;
	  }
	  else if(arg.compare("-txtr_prefix") == 0)
	  {
		  txtr_prefix = argv[i+1];
		  i++;
	  }
	  
	}

	int ret = 0;

	if (mode == NotAssigned || file_in.empty() || file_out.empty() || txtr_prefix.empty())
	{
		cout << "One or more essential arguments were not supplied. Exiting" << endl;
		return -1;
	}

	

	if (mode == STAT  || mode == DOOR_LOAD)
	{
		string template_ = GetExeFileName() + "\\template_stat.nif";
		int s = convert_shape(file_in,file_out,template_,txtr_prefix);
		if (s != NCU_OK)
		{
			remove(file_out.c_str());
			return -1;
		}
		int s2 = add_collision(file_in,file_out,template_);
		if (s2 != NCU_OK)
		{
			remove(file_out.c_str());
			return -1;
		}
	}
	else if (mode == DOOR_ANIM)
	{
		string template_ = GetExeFileName() + "\\template_stat.nif";
		int s = convert_shape(file_in,file_out,template_,txtr_prefix);
		if (s != NCU_OK)
		{
			remove(file_out.c_str());
			return -1;
		}
		template_ = GetExeFileName() + "\\template_door_collision.nif";
		int s2 = add_collision(file_in,file_out,template_);
		if (s2 != NCU_OK)
		{
			remove(file_out.c_str());
			return -1;
		}
		template_ = GetExeFileName() + "\\template_door.nif";
		int s3 = make_door(file_out,file_out,template_);
		if (s3 != NCU_OK)
		{
			remove(file_out.c_str());
			cout << "Failed conv" << endl;
			return -1;
		}
	}
	
		
	return 0;
}