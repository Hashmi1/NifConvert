#include "niflib.h"
#include "obj/NiExtraData.h"
#include "obj/NiTimeController.h"
#include "obj/NiTexturingProperty.h"
#include "obj/BSShaderTextureSet.h"
#include "obj/NiSourceTexture.h"
#include "obj/NiMaterialProperty.h"
#include "obj/NiVertexColorProperty.h"
#include "obj/NiTriStripsData.h"
#include "obj/NiTriStrips.h"
#include "obj/NiBinaryExtraData.h"
#include "obj/bhkCollisionObject.h"
#include "obj/bhkTransformShape.h"
#include "obj/bhkListShape.h"
#include "obj/bhkPackedNiTriStripsShape.h"
#include "obj/hkPackedNiTriStripsData.h"
#include "obj/bhkMoppBvTreeShape.h"
#include "obj/bhkRigidBody.h"

#include "obj/nitrishape.h"
#include "obj/rootcollisionnode.h"
#include "obj/bsfadenode.h"
#include "obj/nialphaproperty.h"
#include "obj/NiTriShapeData.h"
#include "obj/BSLightingShaderProperty.h"
#include "obj/NiTriStrips.h"
#include "obj/bhkShape.h"
# include "obj/BSValueNode.h"

using namespace Niflib;
using namespace std;




unsigned int convertShapeX(string fileNameSrc, string fileNameDst, string fileNameTmpl)
{
	cout << "Here3" << endl;
	NiNodeRef				pRootInput     (NULL);
	NiNodeRef				pRootOutput    (NULL);
	NiNodeRef				pRootTemplate  (NULL);
	NiTriShapeRef			pNiTriShapeTmpl(NULL);
	NiCollisionObjectRef	pRootCollObject(NULL);
	NifInfo					nifInfo;
	vector<NiAVObjectRef>	srcChildList;

	

	bool					fakedRoot      (false);
	cout << "Here4" << endl;
	//  test on existing file names
	if (fileNameSrc.empty())		return -1;
	if (fileNameDst.empty())		return -1;
	if (fileNameTmpl.empty())		return -1;
	cout << "Here5" << endl;
	
	cout << "Here6" << endl;
	
	//  read input NIF
	
	cout << "Here8" << endl;
	//  get template nif
	pRootTemplate = DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameTmpl.c_str()));
	if (pRootTemplate == NULL)
	{
		
	}
	cout << "Here9" << endl;
	//  get shapes from template
	//  - shape root
	pNiTriShapeTmpl = DynamicCast<NiTriShape>(pRootTemplate->GetChildren().at(0));
	if (pNiTriShapeTmpl == NULL)
	{
		
	}
	cout << "Here10" << endl;
	//  get data from input node
	srcChildList    = pRootInput->GetChildren();
	pRootCollObject = pRootInput->GetCollisionObject();
	cout << "Here11" << endl;
	//  template root is used as root of output
	pRootOutput = pRootTemplate;

	//  move date from input to output
	pRootInput ->SetCollisionObject(NULL);
	pRootOutput->SetCollisionObject(pRootCollObject);
	pRootOutput->SetLocalTransform(pRootInput->GetLocalTransform());
	pRootOutput->SetName(pRootInput->GetName());
	cout << "Here12" << endl;
	//  get rid of unwanted subnodes
	pRootOutput->ClearChildren();
	pRootInput->ClearChildren();
	cout << "Here13" << endl;
	//  move children to output
	for (auto pIter=srcChildList.begin(), pEnd=srcChildList.end(); pIter != pEnd; ++pIter)
	{
		pRootOutput->AddChild(*pIter);
		
		
	}
	cout << "Here14" << endl;
	//  iterate over source nodes and convert using template
	//pRootOutput = convertNiNode(pRootOutput, pNiTriShapeTmpl, pRootOutput);
	cout << "Here15" << endl;
	//  write missing textures to log - as block
	
	cout << "Here16" << endl;
	//  set version information
	stringstream	sStream;
	cout << "Here17" << endl;
	sStream << nifInfo.version << ';' << nifInfo.userVersion;
	nifInfo.version      = VER_20_2_0_7;
	nifInfo.userVersion  = 12;
	nifInfo.userVersion2 = 83;
	nifInfo.creator      = "NifConvert";
	nifInfo.exportInfo1  = "1.2.2";
	nifInfo.exportInfo2  = sStream.str();
	cout << "Here18" << endl;
	//  write modified nif file
	WriteNifTree((const char*) fileNameDst.c_str(), pRootOutput, nifInfo);
	cout << "Here19" << endl;
	return 0;
}
