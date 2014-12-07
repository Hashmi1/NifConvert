/**
 *  file:   NifConvertUtility.cpp
 *  class:  NifConvertUtility
 *
 *  Utilities converting NIF files from old to new format
 */

//-----  INCLUDES  ------------------------------------------------------------
//  Common includes
#include "Common\Nif\NifConvertUtility.h"
#include "Common\Util\DefLogMessageTypes.h"
#include "version.h"
#include <algorithm>
#include <strstream>
#include <string>

//  Niflib includes
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
#include "obj/BSValueNode.h"
#include "obj/BSXFlags.h"
//-----  DEFINES  -------------------------------------------------------------
//  used namespaces
using namespace Niflib;
using namespace std;

/*---------------------------------------------------------------------------*/
NifConvertUtility::NifConvertUtility()
	:	_vcDefaultColor    (1.0f, 1.0f, 1.0f, 1.0f),
		_vcHandling        (NCU_VC_REMOVE_FLAG),
		_updateTangentSpace(true),
		_reorderProperties (true),
		_forceDDS          (true),
		_cleanTreeCollision(true),
		_logCallback       (NULL)
{}

/*---------------------------------------------------------------------------*/
NifConvertUtility::~NifConvertUtility()
{}

/*---------------------------------------------------------------------------*/
unsigned int NifConvertUtility::convertShape(string fileNameSrc, string fileNameDst, string fileNameTmpl)
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
	if (fileNameSrc.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameDst.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameTmpl.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	cout << "Here5" << endl;
	//  initialize user messages
	_userMessages.clear();
	logMessage(NCU_MSG_TYPE_INFO, "Source:  "      + (fileNameSrc.empty() ? "- none -" : fileNameSrc));
	logMessage(NCU_MSG_TYPE_INFO, "Template:  "    + (fileNameTmpl.empty() ? "- none -" : fileNameTmpl));
	logMessage(NCU_MSG_TYPE_INFO, "Destination:  " + (fileNameDst.empty() ? "- none -" : fileNameDst));
	logMessage(NCU_MSG_TYPE_INFO, "Texture:  "     + (_pathTexture.empty() ? "- none -" : _pathTexture));
	cout << "Here6" << endl;
	//  initialize used texture list
	_usedTextures.clear();
	_newTextures.clear();
	cout << "Here7" << endl;
	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileNameSrc, "source", fakedRoot, &nifInfo)) == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameSrc + "' as input");
		return NCU_ERROR_CANT_OPEN_INPUT;
	}
	cout << "Here8" << endl;
	//  get template nif
	pRootTemplate = DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameTmpl.c_str()));
	if (pRootTemplate == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameTmpl + "' as template");
		return NCU_ERROR_CANT_OPEN_TEMPLATE;
	}
	cout << "Here9" << endl;
	//  get shapes from template
	//  - shape root
	pNiTriShapeTmpl = DynamicCast<NiTriShape>(pRootTemplate->GetChildren().at(0));
	if (pNiTriShapeTmpl == NULL)
	{
		logMessage(NCU_MSG_TYPE_INFO, "Template has no NiTriShape.");
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
	root_bsafade = pRootOutput;
	pRootOutput = convertNiNode(pRootOutput, pNiTriShapeTmpl, pRootOutput);
	cout << "Here15" << endl;
	//  write missing textures to log - as block
	for (auto pIter=_newTextures.begin(), pEnd=_newTextures.end(); pIter != pEnd; ++pIter)
	{
		logMessage(NCU_MSG_TYPE_TEXTURE_MISS, *pIter);
	}
	cout << "Here16" << endl;
	//  set version information
	stringstream	sStream;
	cout << "Here17" << endl;
	sStream << nifInfo.version << ';' << nifInfo.userVersion;
	nifInfo.version      = VER_20_2_0_7;
	nifInfo.userVersion  = 12;
	nifInfo.userVersion2 = 83;
	nifInfo.creator      = "NifConvert";
	nifInfo.exportInfo1  = MASTER_PRODUCT_VERSION_STR;
	nifInfo.exportInfo2  = sStream.str();
	cout << "Here18" << endl;
	//  write modified nif file
	WriteNifTree((const char*) fileNameDst.c_str(), pRootOutput, nifInfo);
	cout << "Here19" << endl;
	return NCU_OK;
}

// This functions turns a static door into an animated skyrim door.

unsigned int NifConvertUtility::convert_animated_door(string fileNameSrc, string fileNameDst, string fileNameTmpl)
{
	NiNodeRef				pRootInput     (NULL);
	NiNodeRef				pRootOutput    (NULL);
	NiNodeRef				pRootTemplate  (NULL);
	NiTriShapeRef			pNiTriShapeTmpl(NULL);
	NiCollisionObjectRef	pRootCollObject(NULL);
	NifInfo					nifInfo;
	vector<NiAVObjectRef>	srcChildList;
	bool					fakedRoot      (false);

	//  test on existing file names
	if (fileNameSrc.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameDst.empty())		return NCU_ERROR_MISSING_FILE_NAME;
	if (fileNameTmpl.empty())		return NCU_ERROR_MISSING_FILE_NAME;

	//  initialize user messages
	_userMessages.clear();
	logMessage(NCU_MSG_TYPE_INFO, "Source:  "      + (fileNameSrc.empty() ? "- none -" : fileNameSrc));
	logMessage(NCU_MSG_TYPE_INFO, "Template:  "    + (fileNameTmpl.empty() ? "- none -" : fileNameTmpl));
	logMessage(NCU_MSG_TYPE_INFO, "Destination:  " + (fileNameDst.empty() ? "- none -" : fileNameDst));
	logMessage(NCU_MSG_TYPE_INFO, "Texture:  "     + (_pathTexture.empty() ? "- none -" : _pathTexture));

	//  initialize used texture list
	_usedTextures.clear();
	_newTextures.clear();

	//  read input NIF
	if ((pRootInput = getRootNodeFromNifFile(fileNameSrc, "source", fakedRoot, &nifInfo)) == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameSrc + "' as input");
		return NCU_ERROR_CANT_OPEN_INPUT;
	}

	//  get template nif
	pRootTemplate = DynamicCast<BSFadeNode>(ReadNifTree((const char*) fileNameTmpl.c_str()));
	if (pRootTemplate == NULL)
	{
		logMessage(NCU_MSG_TYPE_ERROR, "Can't open '" + fileNameTmpl + "' as template");
		return NCU_ERROR_CANT_OPEN_TEMPLATE;
	}

	//  get shapes from template
	//  - shape root

	/*
	pNiTriShapeTmpl = DynamicCast<NiTriShape>(pRootTemplate->GetChildren().at(0));
	if (pNiTriShapeTmpl == NULL)
	{
		logMessage(NCU_MSG_TYPE_INFO, "Template has no NiTriShape.");
	}
	*/

	//  get data from input node
	srcChildList    = pRootInput->GetChildren();
	pRootCollObject = pRootInput->GetCollisionObject();
	
	//  template root is used as root of output
	//pRootOutput = pRootTemplate;
	pRootOutput = new NiNode();

	//  move date from input to output
	pRootInput ->SetCollisionObject(NULL);
	pRootOutput->SetCollisionObject(pRootCollObject);
	pRootOutput->SetLocalTransform(pRootInput->GetLocalTransform());
	pRootOutput->SetName(pRootInput->GetName());

	//  get rid of unwanted subnodes
	pRootOutput->ClearChildren();
	pRootInput->ClearChildren();

	//  move children to output
	for (auto pIter=srcChildList.begin(), pEnd=srcChildList.end(); pIter != pEnd; ++pIter)
	{
		pRootOutput->AddChild(*pIter);			
	}

	vector<NiAVObjectRef>	templateChildren;
	
	templateChildren = pRootTemplate->GetChildren();

	for (auto pIter2=templateChildren.begin(), pEnd2=templateChildren.end(); pIter2 != pEnd2; ++pIter2)
	{
	
		NiNode* node_;
		node_ = DynamicCast<NiNode>(*pIter2);

		if (node_ == NULL)
		{
			continue;
		}

		if (node_->GetName().compare("FortRuinsDoor01") == 0)
		{
			node_->AddChild(DynamicCast<NiAVObject>(pRootOutput));
		}
		//pRootOutput->AddChild(*pIter2);			
	}




	//  iterate over source nodes and convert using template
	//pRootOutput = convertNiNode(pRootOutput, pNiTriShapeTmpl, pRootOutput);

	//  write missing textures to log - as block
	//for (auto pIter=_newTextures.begin(), pEnd=_newTextures.end(); pIter != pEnd; ++pIter)
	{
		//logMessage(NCU_MSG_TYPE_TEXTURE_MISS, *pIter);
	}

	//  set version information
	stringstream	sStream;
	

	sStream << nifInfo.version << ';' << nifInfo.userVersion;
	nifInfo.version      = VER_20_2_0_7;
	nifInfo.userVersion  = 12;
	nifInfo.userVersion2 = 83;
	nifInfo.creator      = "NifConvert";
	nifInfo.exportInfo1  = MASTER_PRODUCT_VERSION_STR;
	nifInfo.exportInfo2  = sStream.str();

	//  write modified nif file
	WriteNifTree((const char*) fileNameDst.c_str(), pRootTemplate, nifInfo);

	return NCU_OK;
}


/*---------------------------------------------------------------------------*/
void NifConvertUtility::setTexturePath(string pathTexture)
{
	_pathTexture = pathTexture;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setSkyrimPath(string pathSkyrim)
{
	_pathSkyrim = pathSkyrim;
	transform(_pathSkyrim.begin(), _pathSkyrim.end(), _pathSkyrim.begin(), ::tolower);

	size_t	pos(_pathSkyrim.rfind("\\data\\textures"));

	if (pos != string::npos)
	{
		_pathSkyrim.replace(pos, 14, "");
	}
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setVertexColorHandling(VertexColorHandling vcHandling)
{
	_vcHandling = vcHandling;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setDefaultVertexColor(Color4 defaultColor)
{
	_vcDefaultColor = defaultColor;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setUpdateTangentSpace(bool doUpdate)
{
	_updateTangentSpace = doUpdate;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setReorderProperties(bool doReorder)
{
	_reorderProperties = doReorder;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setForceDDS(bool doForce)
{
	_forceDDS = doForce;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setCleanTreeCollision(bool doClean)
{
	_cleanTreeCollision = doClean;
}

/*---------------------------------------------------------------------------*/
vector<string>& NifConvertUtility::getUserMessages()
{
	return _userMessages;
}

/*---------------------------------------------------------------------------*/
set<string>& NifConvertUtility::getUsedTextures()
{
	return _usedTextures;
}

/*---------------------------------------------------------------------------*/
set<string>& NifConvertUtility::getNewTextures()
{
	return _newTextures;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::setLogCallback(void (*logCallback) (const int type, const char* pMessage))
{
	_logCallback = logCallback;
}

/*---------------------------------------------------------------------------*/
NiNodeRef NifConvertUtility::getRootNodeFromNifFile(string fileName, string logPreText, bool& fakedRoot, NifInfo* pNifInfo)
{
	cout << "Here7.1" << endl;

	NiObjectRef		pRootTree (NULL);
	NiNodeRef		pRootInput(NULL);

	//  get input nif

	cout << pNifInfo << endl;
	cout << fileName << endl;

	pRootTree = ReadNifTree((const char*) fileName.c_str(), pNifInfo);
	cout << "Here7.2" << endl;
	//  NiNode as root
	if (DynamicCast<NiNode>(pRootTree) != NULL)
	{
		pRootInput = DynamicCast<NiNode>(pRootTree);
		logMessage(NCU_MSG_TYPE_INFO, logPreText + " root is NiNode.");
	}
	//  NiTriShape as root
	else if (DynamicCast<NiTriShape>(pRootTree) != NULL)
	{
		//  create faked root
		pRootInput = new NiNode();

		//  add root as child
		pRootInput->AddChild(DynamicCast<NiAVObject>(pRootTree));

		//  mark faked root node
		fakedRoot = true;

		logMessage(NCU_MSG_TYPE_INFO, logPreText + " root is NiTriShape.");
	}

	//  no known root type found
	if (pRootInput == NULL)
	{
		logMessage(NCU_MSG_TYPE_WARNING, logPreText + " root has unhandled type: " + pRootTree->GetType().GetTypeName());
	}

	return pRootInput;
}

string toLower(string inp)
{
	string outp = inp;
	for (int i = 0; i < inp.length(); i++)
	{
		outp[i] = tolower(inp.at(i));
	}

	return outp;
}

/*---------------------------------------------------------------------------*/


NiNodeRef NifConvertUtility::convertNiNode(NiNodeRef pSrcNode, NiTriShapeRef pTmplNode, NiNodeRef pRootNode, NiAlphaPropertyRef pTmplAlphaProp)
{
	
	
	NiNodeRef				pDstNode    (pSrcNode);
	

	//pDstNode->SetName(pDstNode->GetName().replace(
	string node_name_in = pDstNode->GetName();
	string node_name_out = "";
	for (int i = 0; i < node_name_in.length(); i++)
	{
		if (node_name_in[i] != '.' && node_name_in[i] != '_' && node_name_in[i] != ' ')
		{
			node_name_out = node_name_out + node_name_in[i];
		}
	}

	pDstNode->SetName(node_name_out);
	node_name_in = node_name_out;

	if (node_name_in.compare("AttachLight") == 0)
	{
		Vector3 tr = pDstNode->GetLocalTranslation();
		tr.z += 10.0f;
		pDstNode->SetLocalTranslation(tr);
		
	}
	
	if (node_name_in.compare("ShadowBox") == 0)
	{
		cout << "Removing ShadowBox" << endl;
		pDstNode->ClearChildren();
	}

	if (toLower(node_name_in).find("fireemit") != -1)
	{
		NiExtraDataRef ed = root_bsafade->GetExtraData().front();
		NiIntegerExtraDataRef iref = DynamicCast<NiIntegerExtraData>(ed);

		iref->SetData(147);
		
		cout << "Adding TorchFlame Addon" << endl;
		BSValueNodeRef candle_flame = new BSValueNode();
		candle_flame->SetName("AddOnNode");
		candle_flame->value = 46;
		pDstNode->AddChild(DynamicCast<NiAVObject>(candle_flame));
	}

	else if (node_name_in.find("CandleFlame") != -1)
	{
		NiExtraDataRef ed = root_bsafade->GetExtraData().front();
		NiIntegerExtraDataRef iref = DynamicCast<NiIntegerExtraData>(ed);

		iref->SetData(147);
		
		cout << "Adding CandleFlame Addon" << endl;
		BSValueNodeRef candle_flame = new BSValueNode();
		candle_flame->SetName("AddOnNode");
		candle_flame->value = 49;
		pDstNode->AddChild(DynamicCast<NiAVObject>(candle_flame));
	}


		

	
	vector<NiAVObjectRef>	srcShapeList(pDstNode->GetChildren());

	//if (!pDstNode->GetType().IsSameType(NiNode::TYPE) && !pDstNode->GetType().IsSameType(BSFadeNode::TYPE) && !pDstNode->GetType().IsSameType(NiTriShape::TYPE) && !pDstNode->GetType().IsSameType(NiTriStrips::TYPE))
	{

	}

	//  find NiAlphaProperty and use as template in sub-nodes
	if (DynamicCast<NiAlphaProperty>(pDstNode->GetPropertyByType(NiAlphaProperty::TYPE)) != NULL)
	{
		pTmplAlphaProp = DynamicCast<NiAlphaProperty>(pDstNode->GetPropertyByType(NiAlphaProperty::TYPE));
	}

	//  unlink protperties -> not used in new format
	pDstNode->ClearProperties();

	//  shift extra data to new version
	pDstNode->ShiftExtraData(VER_20_2_0_7);

	//  unlink children
	pDstNode->ClearChildren();
	pDstNode->ClearEffects();	
	pDstNode->ClearControllers();

	if (!pDstNode->GetType().IsSameType(BSFadeNode::TYPE))
	{
		pDstNode->ClearExtraData();
	}
	//  iterate over source nodes and convert using template
	for (auto  ppIter=srcShapeList.begin(), pEnd=srcShapeList.end(); ppIter != pEnd; ppIter++)
	{
		
		//DynamicCast<NiTriShape>(*ppIter) == NULL && DynamicCast<NiTriStrips>(*ppIter) == NULL ** DynamicCast<NiTriStrips>(*ppIter) != NULL

		

		//  NiTriShape
		if (DynamicCast<NiTriShape>(*ppIter) != NULL)
		{
			pDstNode->AddChild(&(*convertNiTriShape(DynamicCast<NiTriShape>(*ppIter), pTmplNode, pTmplAlphaProp)));
		}
		//  NiTriStrips
		else if (DynamicCast<NiTriStrips>(*ppIter) != NULL)
		{
			pDstNode->AddChild(&(*convertNiTriStrips(DynamicCast<NiTriStrips>(*ppIter), pTmplNode, pTmplAlphaProp)));
		}
		//  RootCollisionNode
		else if ((DynamicCast<RootCollisionNode>(*ppIter) != NULL) && _cleanTreeCollision)
		{
			//  ignore node
		}
		//  NiNode (and derived classes?)
		else if (DynamicCast<NiNode>(*ppIter) != NULL)
		{
			NiNode* node_hashmi = DynamicCast<NiNode>(*ppIter);

						
			if (node_hashmi->GetType().IsSameType(NiNode::TYPE) || node_hashmi->GetType().IsSameType(BSFadeNode::TYPE) || node_hashmi->GetType().IsSameType(BSValueNode::TYPE))
				{
					pDstNode->AddChild(&(*convertNiNode(DynamicCast<NiNode>(*ppIter), pTmplNode, pRootNode, pTmplAlphaProp)));
				}
			

			
		}
	}

	//  remove collision object (newer version)
	if (_cleanTreeCollision)
	{
		pDstNode->SetCollisionObject(NULL);
	}
	else if (DynamicCast<bhkCollisionObject>(pDstNode->GetCollisionObject()) != NULL)
	{
		bhkRigidBodyRef		pBody(DynamicCast<bhkRigidBody>((DynamicCast<bhkCollisionObject>(pDstNode->GetCollisionObject()))->GetBody()));

		if (pBody != NULL)
		{
			parseCollisionTree(pBody->GetShape());
		}
	}

	return pDstNode;
}

/*---------------------------------------------------------------------------*/
NiTriShapeRef NifConvertUtility::convertNiTriShape(NiTriShapeRef pSrcNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp)
{
	if (pSrcNode->GetData()->GetUVSetCount() > 1)
	{
				
		pSrcNode->GetData()->SetUVSetCount(1);
		NiPropertyRef niProp = pSrcNode->GetPropertyByType(NiTexturingProperty::TYPE);
		
		NiTexturingPropertyRef niTexProp;
		if ( niProp != NULL ) {
			niTexProp = DynamicCast<NiTexturingProperty>(niProp);
			
			niTexProp->ClearTexture(TexType::BUMP_MAP);
			niTexProp->ClearTexture(TexType::DARK_MAP);
			niTexProp->ClearTexture(TexType::DECAL_0_MAP);
			niTexProp->ClearTexture(TexType::DECAL_1_MAP);
			niTexProp->ClearTexture(TexType::DECAL_2_MAP);
			niTexProp->ClearTexture(TexType::DETAIL_MAP);
			niTexProp->ClearTexture(TexType::DETAIL_MAP);
			niTexProp->ClearTexture(TexType::GLOSS_MAP);
			niTexProp->ClearTexture(TexType::GLOW_MAP);
			niTexProp->ClearTexture(TexType::NORMAL_MAP);
			niTexProp->ClearTexture(TexType::UNKNOWN2_MAP);
			
			
		}
		

	}

	//  NiTriShape is moved from src to dest. It's unlinked in calling function
	NiTriShapeRef	pDstNode(pSrcNode);

	//  force some data in destination shape
	pDstNode->SetCollisionObject(NULL);  //  no collision object here
	pDstNode->SetFlags          (14);    //  ???

	//  return converted NiTriShape
	return convertNiTri(pDstNode, pTmplNode, pTmplAlphaProp);
}

/*---------------------------------------------------------------------------*/
NiTriShapeRef NifConvertUtility::convertNiTriStrips(NiTriStripsRef pSrcNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp)
{
	NiTriShapeRef			pDstNode   (new NiTriShape());
	NiTriShapeDataRef		pDstGeo    (new NiTriShapeData());
	NiTriStripsDataRef		pSrcGeo    (DynamicCast<NiTriStripsData>(pSrcNode->GetData()));
	vector<NiPropertyRef>	srcPropList(pSrcNode->GetProperties());

	//  copy NiTriStrips to NiTriShape
	pDstNode->SetCollisionObject(NULL);  //  no collision object here
	pDstNode->SetFlags          (14);    //  ???
	pDstNode->SetName           (pSrcNode->GetName());
	pDstNode->SetLocalTransform (pSrcNode->GetLocalTransform());
	pDstNode->SetData           (pDstGeo);

	//  move properties
	for (auto pIter=srcPropList.begin(), pEnd=srcPropList.end(); pIter != pEnd; ++pIter)
	{
		if (DynamicCast<NiVertexColorProperty>(*pIter) != NULL)
		{
			int	iii=0;
		}
		pDstNode->AddProperty(*pIter);
	}
	pSrcNode->ClearProperties();

	//  data node
	if (pSrcGeo != NULL)
	{
		pDstGeo->SetVertices    (pSrcGeo->GetVertices());
		pDstGeo->SetNormals     (pSrcGeo->GetNormals());
		pDstGeo->SetTriangles   (pSrcGeo->GetTriangles());
		pDstGeo->SetVertexColors(pSrcGeo->GetColors());
		pDstGeo->SetUVSetCount  (pSrcGeo->GetUVSetCount());
		for (short idx(0), max(pSrcGeo->GetUVSetCount()); idx < max; ++idx)
		{
			pDstGeo->SetUVSet(idx, pSrcGeo->GetUVSet(idx));
		}
	}  //  if (pSrcGeo != NULL)

	//  return converted NiTriShape
	return convertNiTri(pDstNode, pTmplNode, pTmplAlphaProp);
}

/*---------------------------------------------------------------------------*/
NiTriShapeRef NifConvertUtility::convertNiTri(NiTriShapeRef pDstNode, NiTriShapeRef pTmplNode, NiAlphaPropertyRef pTmplAlphaProp)
{
	BSLightingShaderPropertyRef			pTmplLShader(NULL);
	BSLightingShaderPropertyRef			pDstLShader (NULL);
	NiGeometryDataRef					pDstGeo     (pDstNode->GetData());
	vector<NiPropertyRef>				dstPropList (pDstNode->GetProperties());
	vector<NiPropertyRef>				dstPropArray;
	list<NiExtraDataRef>				dstExtraList(pDstNode->GetExtraData());
	short								bsPropIdx   (0);
	bool								forceAlpha  (pTmplAlphaProp != NULL);
	bool								hasAlpha    (false);
	bool								tanWasCopied(false);

	//  copy new style properties
	if (pDstNode->GetBSProperty(0) != NULL)		dstPropArray.push_back(pDstNode->GetBSProperty(0));
	if (pDstNode->GetBSProperty(1) != NULL)		dstPropArray.push_back(pDstNode->GetBSProperty(1));

	//  look for tangent space data
	for (auto pIter=dstExtraList.begin(), pEnd=dstExtraList.end(); pIter != pEnd; ++pIter)
	{
		if ((DynamicCast<NiBinaryExtraData>(*pIter) != NULL) && (pDstGeo != NULL))
		{
			vector<byte>	vecByte(DynamicCast<NiBinaryExtraData>(*pIter)->GetData());
			vector<Vector3>	vecTan;
			vector<Vector3>	vecBin;
			float*			pStartT((float*) &(vecByte[0]));
			int				numVert(pDstGeo->GetVertexCount() * 3);

			//  extract tangent space
			for (int i(0); i < numVert; i += 3)
			{
				vecTan.push_back(Vector3(pStartT[i],         pStartT[i+1],         pStartT[i+2]));
				vecBin.push_back(Vector3(pStartT[numVert+i], pStartT[numVert+i+1], pStartT[numVert+i+2]));
			}

			//  set tangent space
			pDstGeo->SetTangents  (vecTan);
			pDstGeo->SetBitangents(vecBin);

			//  enable tangent space
			pDstGeo->SetTspaceFlag(0x10);

			tanWasCopied = true;

		}  //  if ((DynamicCast<NiBinaryExtraData>(*pIter) != NULL) && (pDstGeo != NULL))
	}  //  for (auto pIter=dstExtraList.begin(), pEnd=dstExtraList.end(); pIter != pEnd; ++pIter)

	//  remove extra data
	pDstNode->ClearExtraData();

	//  data node
	if (pDstGeo != NULL)
	{
		//  set flags
		if (pTmplNode->GetData() != NULL)
		{
			pDstGeo->SetConsistencyFlags(pTmplNode->GetData()->GetConsistencyFlags());  //  nessessary ???
		}

		//  update tangent space?
		if (_updateTangentSpace && !tanWasCopied && (DynamicCast<NiTriShapeData>(pDstGeo) != NULL))
		{
			//  update tangent space
			if (updateTangentSpace(DynamicCast<NiTriShapeData>(pDstGeo)))
			{
				//  enable tangent space
				pDstGeo->SetTspaceFlag(0x10);
			}
		}  //  if (_updateTangentSpace)
	}  //  if (pDstGeo != NULL)

	//  properties - get them from template
	for (short index(0); index < 2; ++index)
	{
		//  BSLightingShaderProperty
		if (DynamicCast<BSLightingShaderProperty>(pTmplNode->GetBSProperty(index)) != NULL)
		{
			pTmplLShader = DynamicCast<BSLightingShaderProperty>(pTmplNode->GetBSProperty(index));
		}
		//  NiAlphaProperty
		else if (DynamicCast<NiAlphaProperty>(pTmplNode->GetBSProperty(index)) != NULL)
		{
			pTmplAlphaProp = DynamicCast<NiAlphaProperty>(pTmplNode->GetBSProperty(index));
		}
	}  //  for (short index(0); index < 2; ++index)

	//  parse properties of destination node
	//dstPropList = pDstNode->GetProperties();
	pDstNode->ClearProperties();
	pDstNode->SetBSProperties(Niflib::array<2, NiPropertyRef>());

	for (auto ppIter=dstPropList.begin(), pEnd=dstPropList.end(); ppIter != pEnd; ppIter++)
	{
		//  NiAlphaProperty
		if (DynamicCast<NiAlphaProperty>(*ppIter) != NULL)
		{
			NiAlphaPropertyRef	pPropAlpha(DynamicCast<NiAlphaProperty>(*ppIter));

			//  set new property to node
			pDstNode->SetBSProperty(bsPropIdx++, &(*pPropAlpha));

			//  own alpha, reset forced alpha
			forceAlpha = false;

			//  mark alpha property
			hasAlpha = true;
		}
		//  NiTexturingProperty
		else if (DynamicCast<NiTexturingProperty>(*ppIter) != NULL)
		{
			char*						pTextPos (NULL);
			BSShaderTextureSetRef		pDstSText(new BSShaderTextureSet());
			TexDesc						baseTex  ((DynamicCast<NiTexturingProperty>(*ppIter))->GetTexture(BASE_MAP));
			string						texture  (baseTex.source->GetTextureFileName());
			string::size_type			result   (string::npos);

			//  clone shader property from template
			pDstLShader = cloneBSLightingShaderProperty(pTmplLShader);

			//  copy textures from template to copy
			pDstSText->SetTextures(pTmplLShader->GetTextureSet()->GetTextures());

			//  separate filename from path
			result = texture.rfind('\\');
			if (result == string::npos)			result  = texture.find_last_of('/');
			if (result != string::npos)			texture = texture.substr(result + 1);

			//  build texture name
			if (_forceDDS)
			{
				result = texture.rfind('.');
				if (result != string::npos)		texture.erase(result);
				texture += ".dds";
			}

			//  build full path
			texture = _pathTexture + texture;

			//  set new texture map
			pDstSText->SetTexture(0, texture);

			logMessage(NCU_MSG_TYPE_TEXTURE, string("Txt-Used: ") + texture);
			if (!checkFileExists(texture))
			{
				_newTextures.insert(string("Txt-Missed: ") + texture);
			}

			//  build normal map name
			result = texture.rfind('.');
			if (result == string::npos)
			{
				texture += "_n";
			}
			else
			{
				string	extension(texture.substr(result));

				texture.erase(result);
				texture += "_n" + extension;
			}

			//  set new normal map
			pDstSText->SetTexture(1, texture);

			if (!checkFileExists(texture))
			{
				_newTextures.insert(string("Txt-Missed: ") + texture);
			}

			//  add texture set to texture property
			pDstLShader->SetTextureSet(pDstSText);

			//  check for existing vertex colors
			if ((pDstGeo != NULL) && (pDstGeo->GetColors().size() <= 0) && ((pDstLShader->GetShaderFlags2() & Niflib::SLSF2_VERTEX_COLORS) != 0))
			{
				switch (_vcHandling)
				{
					case NCU_VC_REMOVE_FLAG:
					{
						pDstLShader->SetShaderFlags2((SkyrimShaderPropertyFlags2) (pDstLShader->GetShaderFlags2() & ~Niflib::SLSF2_VERTEX_COLORS));
						break;
					}

					case NCU_VC_ADD_DEFAULT:
					{
						pDstGeo->SetVertexColors(vector<Color4>(pDstGeo->GetVertexCount(), _vcDefaultColor));
						break;
					}
				}
			}  //  if ((pDstGeo != NULL) && (pDstGeo->GetColors().size() <= 0) && ...

			//  set new property to node
			pDstNode->SetBSProperty(bsPropIdx++, &(*pDstLShader));
		}
		//  NiMaterialProperty
		else if (DynamicCast<NiMaterialProperty>(*ppIter) != NULL)
		{
			//  remove property from node
			pDstNode->RemoveProperty(*ppIter);
		}
	}  //  for (vector<NiPropertyRef>::iterator  ppIter = dstPropList.begin(); ppIter != dstPropList.end(); ppIter++)

	//  add forced NiAlphaProperty?
	if (forceAlpha)
	{
		NiAlphaPropertyRef	pPropAlpha(new NiAlphaProperty());

		//  set values from template
		pPropAlpha->SetFlags        (pTmplAlphaProp->GetFlags());
		pPropAlpha->SetTestThreshold(pTmplAlphaProp->GetTestThreshold());

		//  set new property to node
		pDstNode->SetBSProperty(bsPropIdx++, &(*pPropAlpha));

		//  mark alpha property
		hasAlpha = true;

	}  //  if (forceAlpha)

	//  add default vertex colors if alpha property and no colors
	if (hasAlpha && (pDstGeo != NULL) && (pDstGeo->GetColors().size() <= 0))
	{
		pDstGeo->SetVertexColors(vector<Color4>(pDstGeo->GetVertexCount(), _vcDefaultColor));

		//  force flag in BSLightingShaderProperty
		if (pDstLShader != NULL)
		{
			pDstLShader->SetShaderFlags2((SkyrimShaderPropertyFlags2) (pDstLShader->GetShaderFlags2() | Niflib::SLSF2_VERTEX_COLORS));
		}
	}

	//  add allowed properties to fee slots
	for (auto pIter=dstPropArray.begin(), pEnd=dstPropArray.end(); (pIter != pEnd) && (bsPropIdx < 2); ++pIter)
	{
		//  skip old style properties
		if ((*pIter)->GetType().GetTypeName() == "NiMaterialProperty")		continue;

		//  add property to free slot
		pDstNode->SetBSProperty(bsPropIdx++, *pIter);
	}

	//  reorder properties - only necessary in case of both are set
	if (_reorderProperties && (pDstNode->GetBSProperty(0) != NULL) && (pDstNode->GetBSProperty(1) != NULL))
	{
		NiPropertyRef	tProp01(pDstNode->GetBSProperty(0));
		NiPropertyRef	tProp02(pDstNode->GetBSProperty(1));

		//  make sure BSLightingShaderProperty comes before NiAlphaProperty - seems a 'must be'
		if ((tProp01->GetType().GetTypeName() == "NiAlphaProperty") &&
			(tProp02->GetType().GetTypeName() == "BSLightingShaderProperty")
		   )
		{
			pDstNode->SetBSProperty(0, tProp02);
			pDstNode->SetBSProperty(1, tProp01);
		}
	}  //  if (_reorderProperties)

	return  pDstNode;
}

/*---------------------------------------------------------------------------*/
bool NifConvertUtility::updateTangentSpace(NiTriShapeDataRef pDataObj)
{
	vector<Vector3>		vecVertices (pDataObj->GetVertices());
	vector<Vector3>		vecNormals  (pDataObj->GetNormals());
	vector<Triangle>	vecTriangles(pDataObj->GetTriangles());
	vector<TexCoord>	vecTexCoords;

	//  get first uv-set if available
	if (pDataObj->GetUVSetCount() > 0)		vecTexCoords = pDataObj->GetUVSet(0);

	//  check on valid input data
	if (vecVertices.empty() || vecTriangles.empty() || vecNormals.size() != vecVertices.size() || vecVertices.size() != vecTexCoords.size())
	{
		logMessage(NCU_MSG_TYPE_INFO, "UpdateTangentSpace: No vertices, normals, coords or faces defined.");
		return false;
	}

	//  prepare result vectors
	vector<Vector3>		vecTangents  = vector<Vector3>(vecVertices.size(), Vector3(0.0f, 0.0f, 0.0f));
	vector<Vector3>		vecBiNormals = vector<Vector3>(vecVertices.size(), Vector3(0.0f, 0.0f, 0.0f));

	for (unsigned int t(0); t < vecTriangles.size(); ++t)
	{
		Vector3		vec21(vecVertices[vecTriangles[t][1]] - vecVertices[vecTriangles[t][0]]);
		Vector3		vec31(vecVertices[vecTriangles[t][2]] - vecVertices[vecTriangles[t][0]]);
		TexCoord	txc21(vecTexCoords[vecTriangles[t][1]] - vecTexCoords[vecTriangles[t][0]]);
		TexCoord	txc31(vecTexCoords[vecTriangles[t][2]] - vecTexCoords[vecTriangles[t][0]]);
		float		radius(((txc21.u * txc31.v - txc31.u * txc21.v) >= 0.0f ? +1.0f : -1.0f));

		Vector3		sdir(( txc31.v * vec21[0] - txc21.v * vec31[0] ) * radius,
					     ( txc31.v * vec21[1] - txc21.v * vec31[1] ) * radius,
					     ( txc31.v * vec21[2] - txc21.v * vec31[2] ) * radius);
		Vector3		tdir(( txc21.u * vec31[0] - txc31.u * vec21[0] ) * radius,
					     ( txc21.u * vec31[1] - txc31.u * vec21[1] ) * radius,
					     ( txc21.u * vec31[2] - txc31.u * vec21[2] ) * radius);

		//  normalize
		sdir = sdir.Normalized();
		tdir = tdir.Normalized();

		for (int j(0); j < 3; ++j)
		{
			vecTangents [vecTriangles[t][j]] += tdir;
			vecBiNormals[vecTriangles[t][j]] += sdir;
		}
	}  //  for (unsigned int t(0); t < vecTriangles.size(); ++t)

	for (unsigned int i(0); i < vecVertices.size(); ++i)
	{
		Vector3&	n(vecNormals[i]);
		Vector3&	t(vecTangents [i]);
		Vector3&	b(vecBiNormals[i]);

		if ((t == Vector3()) || (b == Vector3()))
		{
			t[0] = n[1];
			t[1] = n[2];
			t[2] = n[0];

			b = n.CrossProduct(t);
		}
		else
		{
			t = t.Normalized();
			t = (t - n * n.DotProduct(t));
			t = t.Normalized();

			b = b.Normalized();
			b = (b - n * n.DotProduct(b));
			b = (b - t * t.DotProduct(b));
			b = b.Normalized();
		}
	}  //  for (unsigned int i(0); i < vecVertices.size(); ++i)

	//  set tangents and binormals to object
	pDataObj->SetBitangents(vecBiNormals);
	pDataObj->SetTangents  (vecTangents);

	return true;
}

/*---------------------------------------------------------------------------*/
BSLightingShaderPropertyRef NifConvertUtility::cloneBSLightingShaderProperty(BSLightingShaderPropertyRef pSource)
{
	BSLightingShaderPropertyRef		pDest(new BSLightingShaderProperty);
	BSShaderTextureSetRef			pTSet(pSource->GetTextureSet());

	//  force empty texture set to source (HACK)
	pSource->SetTextureSet(NULL);

	//  copy all members, even inaccessable ones (HACK)
	memcpy(pDest, pSource, sizeof(BSLightingShaderProperty));

	//  reset source texture set
	pSource->SetTextureSet(pTSet);

	return pDest;
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::logMessage(int type, string text)
{
	//  add message to internal storages
	switch (type)
	{
		default:
		case NCU_MSG_TYPE_INFO:
		case NCU_MSG_TYPE_WARNING:
		case NCU_MSG_TYPE_ERROR:
		{
			_userMessages.push_back(text);
			break;
		}

		case NCU_MSG_TYPE_TEXTURE:
		{
			_usedTextures.insert(text);
			break;
		}

		case NCU_MSG_TYPE_TEXTURE_MISS:
		{
//			_newTextures.insert(text);
			break;
		}
	}

	//  send message to callback if given
	if (_logCallback != NULL)
	{
		(*_logCallback)(type, text.c_str());
	}
}

/*---------------------------------------------------------------------------*/
bool NifConvertUtility::checkFileExists(string fileName)
{
	string		path   (_pathSkyrim + "\\Data\\" + fileName);
	ifstream	iStream(path.c_str());

	//  return existance of file
	return iStream.good();
}

/*---------------------------------------------------------------------------*/
void NifConvertUtility::parseCollisionTree(bhkShapeRef pShape)
{
	//  check type of collision mesh
	//bhkMoppBvTreeShape
	if (DynamicCast<bhkMoppBvTreeShape>(pShape) != NULL)
	{
		parseCollisionTree(DynamicCast<bhkMoppBvTreeShape>(pShape)->GetShape());
	}
	//bhkTransformShape
	else if (DynamicCast<bhkTransformShape>(pShape) != NULL)
	{
		parseCollisionTree(DynamicCast<bhkTransformShape>(pShape)->GetShape());
	}
	//bhkListShape
	else if (DynamicCast<bhkListShape>(pShape) != NULL)
	{
		vector<bhkShapeRef>		subShapes(DynamicCast<bhkListShape>(pShape)->GetSubShapes());

		//  parse sub shape(s)
		for (auto pIter=subShapes.begin(), pEnd=subShapes.end(); pIter != pEnd; ++pIter)
		{
			//  parse sub shape(s)
			parseCollisionTree(*pIter);
		}
	}
	//bhkPackedNiTriStripsShape
	else if (DynamicCast<bhkPackedNiTriStripsShape>(pShape) != NULL)
	{
		bhkPackedNiTriStripsShapeRef	pTriShape(DynamicCast<bhkPackedNiTriStripsShape>(pShape));
		hkPackedNiTriStripsDataRef		pTriData (pTriShape->GetData());

		//  move sub-shapes into data
		pTriData->SetSubShapes(pTriShape->GetSubShapes());
		pTriShape->SetSubShapes(vector<OblivionSubShape>());
	}
}
