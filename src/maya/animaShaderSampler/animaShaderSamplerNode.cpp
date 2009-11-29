//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Anima Vitae Ltd. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Anima Vitae Ltd. nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "animaShaderSamplerNode.h"

MTypeId     animaShaderSampler::id(0x00114973);

MObject     animaShaderSampler::aSamplePosMatrixArray;
MObject     animaShaderSampler::aUArray;
MObject     animaShaderSampler::aVArray;
MObject     animaShaderSampler::aSG;
MObject     animaShaderSampler::aInColor;
MObject     animaShaderSampler::aOutColorArray;

animaShaderSampler::animaShaderSampler() {}

animaShaderSampler::~animaShaderSampler() {}

MStatus animaShaderSampler::connectionMade(const MPlug& plug, const MPlug& otherPlug, bool asSrc)
{
	
	MStatus returnStatus;
	if (plug == aInColor)
	{
		cout << "[aSS] connected "<< otherPlug.name().asChar() << " to inColor" << endl;
		inColorConnectionName = otherPlug.name();
	}
	MPlug inColPlug(plug.node(),aInColor);
	if (plug == inColPlug.child(0) || plug == inColPlug.child(1) || plug == inColPlug.child(2))
	{
		cout << "[aSS] connected "<< otherPlug.name().asChar() << " to a component of inColor" << endl;
		cout << "[aSS] we don't support connections to color components (yet) - please connect to \".inColor\"" << endl;
	}

	return MPxNode::connectionMade(plug, otherPlug, asSrc);
}

MStatus animaShaderSampler::connectionBroken(const MPlug& plug, const MPlug& otherPlug, bool asSrc)
{ 
	MStatus returnStatus;
	if (plug == aInColor)
	{
		inColorConnectionName = "";
	}
	
	forceUpdate = true;
	
	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
}

MStatus animaShaderSampler::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus returnStatus;
	
	MFloatArray uCoords;
	MFloatArray vCoords;
	MFloatArray filterSizes;
	MFloatPointArray points;
	MFloatPointArray refPoints;
	MFloatVectorArray normals;
	MFloatVectorArray tanUs;
	MFloatVectorArray tanVs;

	int i;

	// get attributes
	MArrayDataHandle samplePosArrayData = data.inputValue(aSamplePosMatrixArray, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	samplePosArrayData.jumpToElement(0);
	for(i=0; i<samplePosArrayData.elementCount(); i++)
	{
		MFloatMatrix posMtx = samplePosArrayData.inputValue().asFloatMatrix();
		// get position from the matrix
		points.append(posMtx(3,0),posMtx(3,1),posMtx(3,2));
		samplePosArrayData.next();
	}
	refPoints = points;

	
	MArrayDataHandle uArrayData = data.inputValue(aUArray, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	uArrayData.jumpToElement(0);
	for(i=0; i<uArrayData.elementCount(); i++)
	{
		uCoords.append(uArrayData.inputValue().asFloat());
		uArrayData.next();
	}

	
	MArrayDataHandle vArrayData = data.inputValue(aVArray, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	vArrayData.jumpToElement(0);
	for(i=0; i<vArrayData.elementCount(); i++)
	{
		vCoords.append(vArrayData.inputValue().asFloat());
		vArrayData.next();
	}

	MDataHandle inColorData = data.inputValue(aInColor, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	MFloatVector inCol = inColorData.asFloatVector();	

	MDataHandle sgData = data.inputValue(aSG, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	MString SGstr = sgData.asString();  
	
	MArrayDataHandle outColorArrayData = data.outputValue(aOutColorArray);
	
	MString networkToSample = "";
	if ( SGstr != "")
		networkToSample = SGstr;
	else
	{
		// get connected shader name
		MPlugArray tmpPlugs;
		MPlug inColPlug = MPlug(thisMObject(),aInColor);
		inColPlug.connectedTo(tmpPlugs,1,0);
		if (tmpPlugs.length())
			networkToSample = tmpPlugs[0].name();
		else
			networkToSample = "";
	}
	
	// maybe add these as attributes on the node??
	bool shadow = 0;
	bool reuse = 0;

	if (outColorArrayData.elementCount()==points.length() && points.length()==vCoords.length() && points.length()==uCoords.length())
	{
		if (networkToSample != "")
		{
			// sample the connected shader
			MDagPath cameraPath;
			M3dView::active3dView().getCamera(cameraPath);
			MMatrix mat = cameraPath.inclusiveMatrix();
			MFloatMatrix cameraMat( mat.matrix );
			
			MFloatVectorArray colors, transps;
			if (MS::kSuccess == MRenderUtil::sampleShadingNetwork(
					networkToSample, 
					points.length(),
					shadow,
					reuse,
					cameraMat,
					(points.length()>0)     ? &points : NULL,
					(uCoords.length()>0)    ? &uCoords : NULL,
					(vCoords.length()>0)    ? &vCoords : NULL,
					(normals.length()>0)    ? &normals : NULL,
					(refPoints.length()>0)  ? &refPoints : NULL,
					(tanUs.length()>0)              ? &tanUs : NULL,
					(tanVs.length()>0)              ? &tanVs : NULL,
					(filterSizes.length()>0) ? &filterSizes : NULL,
					colors,
					transps ) )
			{

				outColorArrayData.jumpToElement(0);
				for(i=0; i<outColorArrayData.elementCount(); i++)
				{
					outColorArrayData.outputValue().set(colors[i]);
					outColorArrayData.next();
				}
				outColorArrayData.setAllClean();

			}
		}
	}
	else
	{
		cout << "[aSS] Error: nonmatching number of input and output array elements" << endl;
		cout << "[aSS] samplePosMatrixArray has " << points.length() << " elements" << endl;
		cout << "[aSS] uArray has " << uCoords.length() << " elements" << endl;
		cout << "[aSS] vArray has " << vCoords.length() << " elements" << endl;
		cout << "[aSS] outColorArray has " << outColorArrayData.elementCount() << " elements" << endl;
	}

	return MS::kSuccess;
}



void* animaShaderSampler::creator()
{
	return new animaShaderSampler();
}

MStatus animaShaderSampler::initialize()		
{

	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;
	MFnMessageAttribute mAttr;
	MFnMatrixAttribute mtxAttr;
	MStatus	stat;
	
	aSamplePosMatrixArray = mtxAttr.create("samplePosMatrixArray", "spm", MFnMatrixAttribute::kFloat);
	mtxAttr.setReadable(true);
	mtxAttr.setKeyable(true);
	mtxAttr.setArray(true);

	aUArray = nAttr.create("uArray", "ua", MFnNumericData::kFloat);
	nAttr.setReadable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.0f);
	nAttr.setArray(true);

	aVArray = nAttr.create("vArray", "va", MFnNumericData::kFloat);
	nAttr.setReadable(true);
	nAttr.setKeyable(true);
	nAttr.setDefault(0.0f);
	nAttr.setArray(true);

	aSG = tAttr.create("SG", "sg", MFnStringData::kString);
	tAttr.setWritable(true);
	tAttr.setReadable(true);
	tAttr.setHidden(false);
	tAttr.setStorable(true);
	
	aInColor = nAttr.createColor("inColor", "inc");
	nAttr.setDefault(0.0f, 0.0f, 0.0f);
	nAttr.setKeyable(true);
	nAttr.setStorable(true);  
	nAttr.setUsedAsColor(true); 
	nAttr.setReadable(true);
	nAttr.setWritable(true);

	aOutColorArray = nAttr.createColor("outColorArray", "oca");
	nAttr.setDefault(0.0f, 0.0f, 0.0f);
	nAttr.setUsedAsColor(true); 
	nAttr.setReadable(true);
	nAttr.setArray(true);

	// Add the attributes to the node
	addAttribute(aSamplePosMatrixArray);
	addAttribute(aUArray);
	addAttribute(aVArray);
	addAttribute(aSG);
	addAttribute(aInColor);
	addAttribute(aOutColorArray);
	
	attributeAffects( aSamplePosMatrixArray, aOutColorArray );
	attributeAffects( aUArray, aOutColorArray );
	attributeAffects( aVArray, aOutColorArray );
	attributeAffects( aInColor, aOutColorArray );

	return MS::kSuccess;
}
