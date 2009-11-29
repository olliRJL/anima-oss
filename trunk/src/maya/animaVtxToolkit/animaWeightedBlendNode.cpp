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

#include "animaWeightedBlendNode.h"

MTypeId     animaWeightedBlend::id(0x00114971);

// attributes
MObject     animaWeightedBlend::aTargetA;
MObject     animaWeightedBlend::aWeightA;
MObject     animaWeightedBlend::aTargetB;
MObject     animaWeightedBlend::aWeightB;
MObject     animaWeightedBlend::aTargetC;
MObject     animaWeightedBlend::aWeightC;
MObject     animaWeightedBlend::aTargetsSequential;
MObject     animaWeightedBlend::aBaseMeshDeforms;
MObject     animaWeightedBlend::aUseVertexColorScale;

animaWeightedBlend::animaWeightedBlend() {}

animaWeightedBlend::~animaWeightedBlend() {}

MStatus animaWeightedBlend::deform(MDataBlock& block,
				MItGeometry& iter,
				const MMatrix& m,
				unsigned int multiIndex)
{
	MStatus returnStatus;

	// get attributes
	
	// targetA
	MPlug targetAPlug(thisMObject(),aTargetA);
	bool targetAOn = targetAPlug.isConnected();
	bool Adirty = !block.isClean(aTargetA);
	// targetB
	MPlug targetBPlug(thisMObject(),aTargetB);
	bool targetBOn = targetBPlug.isConnected();
	bool Bdirty = !block.isClean(aTargetB);
	// targetC
	MPlug targetCPlug(thisMObject(),aTargetC);
	bool targetCOn = targetCPlug.isConnected();
	bool Cdirty = !block.isClean(aTargetC);
	// inputMesh
	MPlug inputGeomPlug(thisMObject(),inputGeom);
	bool inputOn = inputGeomPlug.isConnected();
	bool inputDirty = !block.isClean(inputGeom);
	
	MDataHandle envData = block.inputValue(envelope, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float env = envData.asFloat();	
	
	MDataHandle weightAData = block.inputValue(aWeightA, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float wA = weightAData.asFloat();	
	
	MDataHandle weightBData = block.inputValue(aWeightB, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float wB = weightBData.asFloat();	
	
	MDataHandle weightCData = block.inputValue(aWeightC, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float wC = weightCData.asFloat();	
	
	MDataHandle targetSequentialData = block.inputValue(aTargetsSequential, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	bool targetsSequential = targetSequentialData.asBool();	

	MDataHandle baseMeshDeformsData = block.inputValue(aBaseMeshDeforms, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	bool baseDeforms = baseMeshDeformsData.asBool();	
	
	MDataHandle useVColData = block.inputValue(aUseVertexColorScale, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	bool useVCol = useVColData.asBool();	
	
	MColorArray vertexColors;
	if (Adirty || Bdirty || Cdirty || useVCol || baseDeforms)
	{
		MObject inputObject;
		inputGeomPlug.getValue( inputObject ); 
		MFnMesh inputMesh( inputObject );
		
		if(useVCol)
		{
			inputMesh.getVertexColors(vertexColors);
			// if no vtxColors! 
			if (vertexColors.length() == 0)
				useVCol = false;
		}

		if (Adirty || Bdirty || Cdirty || baseDeforms)
		{
			MFloatPointArray points;
			inputMesh.getPoints(points,MSpace::kObject);
			
			if(targetAOn && (Adirty || baseDeforms))
			{
				MDataHandle targetAData = block.inputValue(aTargetA, &returnStatus);
				if (MS::kSuccess != returnStatus) return returnStatus;
				MObject targetObjA = targetAData.asMesh();	
				MFnMesh targetMeshA(targetObjA);
				// clear arrays
				deltaA.clear();
				// get target points
				MFloatPointArray targetPoints;
				targetMeshA.getPoints(targetPoints,MSpace::kObject);
				if (targetPoints.length() == points.length())
				{
					for (int i=0; i<points.length(); i++)
					{
						deltaA.append(targetPoints[i]-points[i]);
					}
				}
				else
					cout << "[aWB] Error: targetA has nonmatching number on vertices" << endl;	
			}
			if(targetBOn && (Bdirty || baseDeforms))
			{
				MDataHandle targetBData = block.inputValue(aTargetB, &returnStatus);
				if (MS::kSuccess != returnStatus) return returnStatus;
				MObject targetObjB = targetBData.asMesh();	
				MFnMesh targetMeshB(targetObjB);
				// clear arrays
				deltaB.clear();
				// get target points
				MFloatPointArray targetPoints;
				targetMeshB.getPoints(targetPoints,MSpace::kObject);
				if (targetPoints.length() == points.length())
				{
					for (int i=0; i<points.length(); i++)
					{
						deltaB.append(targetPoints[i]-points[i]);
					}
				}
				else
					cout << "[aWB] Error: targetB has nonmatching number on vertices" << endl;	
			}
			if(targetCOn && (Cdirty || baseDeforms))
			{
				MDataHandle targetCData = block.inputValue(aTargetC, &returnStatus);
				if (MS::kSuccess != returnStatus) return returnStatus;
				MObject targetObjC = targetCData.asMesh();	
				MFnMesh targetMeshC(targetObjC);
				// clear arrays
				deltaC.clear();
				// get target points
				MFloatPointArray targetPoints;
				targetMeshC.getPoints(targetPoints,MSpace::kObject);
				if (targetPoints.length() == points.length())
				{
					for (int i=0; i<points.length(); i++)
					{
						deltaC.append(targetPoints[i]-points[i]);
					}
				}
				else
					cout << "[aWB] Error: targetC has nonmatching number on vertices" << endl;	
			}
		}
	}

	if(!targetsSequential)
	{
		// one targer per color component mode
		for ( ; !iter.isDone(); iter.next())
		{
			int i = iter.index();
			MPoint pt = iter.position();

			if (useVCol)
			{
				if (targetAOn)
					pt += env*wA*vertexColors[i].r*deltaA[i];
				if (targetBOn)
					pt += env*wB*vertexColors[i].g*deltaB[i];
				if (targetCOn)
					pt += env*wC*vertexColors[i].b*deltaC[i];
			}
			else
			{
				if (targetAOn)
					pt += env*wA*deltaA[i];
				if (targetBOn)
					pt += env*wB*deltaB[i];
				if (targetCOn)
					pt += env*wC*deltaC[i];
			}
			iter.setPosition(pt);
		}
	}
	else
	{
		// sequential targets mode
		if (0.0f <= wA && wA <= wB && wB <= wC)
		{
			for ( ; !iter.isDone(); iter.next())
			{
				int i = iter.index();
				MPoint pt = iter.position();
	
				float w;
				if (useVCol)
					w = env*vertexColors[i].r;
				else
					w = env;	
				if (targetAOn && w <= wA)
					pt += w/wA*deltaA[i];	
				else if (targetAOn && targetBOn && wA < w && w <= wB) {
					float rw = (w-wA)/(wB-wA);
					pt += (1.0f-rw)*deltaA[i]+rw*deltaB[i];	
				}	
				else if (!targetBOn && wA <= w)
					pt += deltaA[i];	
				else if (targetBOn && targetCOn && wB < w) {
					float rw = (w-wB)/(wC-wB);
					pt += (1.0f-rw)*deltaB[i]+rw*deltaC[i];	
				}
				else if (!targetCOn && wB <= w)
					pt += deltaB[i];	
	
				iter.setPosition(pt);
			}
		}
		else
			cout << "[aWB] Error: bad target weights for sequential mode" << endl;	
	}
	
	return returnStatus;
}

void* animaWeightedBlend::creator()
{
	return new animaWeightedBlend();
}

MStatus animaWeightedBlend::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	MFnMatrixAttribute mAttr;
	MStatus	stat;

	aTargetA = tAttr.create("targetA", "taa", MFnData::kMesh);
	tAttr.setStorable(true);

	aWeightA = nAttr.create("weightA", "wea", MFnNumericData::kFloat, 1.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);

	aTargetB = tAttr.create("targetB", "tab", MFnData::kMesh);
	tAttr.setStorable(true);

	aWeightB = nAttr.create("weightB", "web", MFnNumericData::kFloat, 1.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	
	aTargetC = tAttr.create("targetC", "tac", MFnData::kMesh);
	tAttr.setStorable(true);

	aWeightC = nAttr.create("weightC", "wec", MFnNumericData::kFloat, 1.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	
	aTargetsSequential = nAttr.create("targetsSequential", "tsq", MFnNumericData::kBoolean, false);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	
	
	aBaseMeshDeforms = nAttr.create("baseMeshDeforms", "bmd", MFnNumericData::kBoolean, false);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);


	aUseVertexColorScale = nAttr.create("useVertexColorScale", "uvc", MFnNumericData::kBoolean, false);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);

	// Add the attributes we have created to the node
	addAttribute(aTargetA);
	addAttribute(aWeightA);
	addAttribute(aTargetB);
	addAttribute(aWeightB);
	addAttribute(aTargetC);
	addAttribute(aWeightC);
	addAttribute(aTargetsSequential);
	addAttribute(aBaseMeshDeforms);
	addAttribute(aUseVertexColorScale);

	attributeAffects(aTargetA, outputGeom);
	attributeAffects(aWeightA, outputGeom);
	attributeAffects(aTargetB, outputGeom);
	attributeAffects(aWeightB, outputGeom);
	attributeAffects(aTargetC, outputGeom);
	attributeAffects(aWeightC, outputGeom);
	attributeAffects(aTargetsSequential, outputGeom);
	attributeAffects(aBaseMeshDeforms, outputGeom);
	attributeAffects(aUseVertexColorScale, outputGeom);

	return MS::kSuccess;
}
