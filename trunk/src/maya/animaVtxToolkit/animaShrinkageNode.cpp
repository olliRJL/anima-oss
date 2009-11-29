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

#include "animaShrinkageNode.h"

MTypeId     animaShrinkage::id(0x00114972);

// attributes
MObject     animaShrinkage::aOrigMesh;
MObject     animaShrinkage::aInMesh;
MObject     animaShrinkage::aOutMesh;
MObject     animaShrinkage::aShrinkMin;
MObject     animaShrinkage::aShrinkMax;
MObject     animaShrinkage::aStretchMin;
MObject     animaShrinkage::aStretchMax;


animaShrinkage::animaShrinkage() {}

animaShrinkage::~animaShrinkage() {}

MStatus	animaShrinkage::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus returnStatus;

	// get attributes
	// origMesh (non-deformed mesh)
	MPlug origMeshPlug(thisMObject(),aOrigMesh);
	bool origMeshOn = origMeshPlug.isConnected();

	MDataHandle origMeshData = block.inputValue(aOrigMesh, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	MObject OrigMeshObj = origMeshData.asMesh();	
	MFnMesh origMesh(OrigMeshObj);

	// inMesh (deformed mesh)
	MPlug inMeshPlug(thisMObject(),aInMesh);
	bool inMeshOn = inMeshPlug.isConnected();

	MDataHandle inMeshData = block.inputValue(aInMesh, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	MObject inMeshObj = inMeshData.asMesh();	
	MFnMesh inMesh(inMeshObj);

	MDataHandle shrinkMinData = block.inputValue(aShrinkMin, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float shMin = shrinkMinData.asFloat();	
	
	MDataHandle shrinkMaxData = block.inputValue(aShrinkMax, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float shMax = shrinkMaxData.asFloat();	

	MDataHandle stretchMinData = block.inputValue(aStretchMin, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float stMin = stretchMinData.asFloat();	

	MDataHandle stretchMaxData = block.inputValue(aStretchMax, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float stMax = stretchMaxData.asFloat();	
	

	if (origMeshOn)
	{
		origEdgeLengths.clear();
		for (int i=0; i<origMesh.numEdges(); i++)
		{
			int2 edgeVtxIndices;
			origMesh.getEdgeVertices(i,edgeVtxIndices);
			MPoint vtx1, vtx2;
			origMesh.getPoint(edgeVtxIndices[0],vtx1);
			origMesh.getPoint(edgeVtxIndices[1],vtx2);
			MVector edgeVec = vtx1-vtx2;
			origEdgeLengths.append((float)edgeVec.length());
		}
	}	


	if (inMeshOn)
	{
		int i;
		vtxShrinkage = new float[inMesh.numVertices()];
		vtxShrinkCount = new int[inMesh.numVertices()];

		for (i=0; i<inMesh.numVertices(); i++)
		{
			vtxShrinkage[i] = 0.0f;
			vtxShrinkCount[i] = 0;
		}
		

		edgeShrinkage.clear();
		for (i=0; i<inMesh.numEdges(); i++)
		{
			int2 edgeVtxIndices;
			inMesh.getEdgeVertices(i,edgeVtxIndices);
			MPoint vtx1, vtx2;
			inMesh.getPoint(edgeVtxIndices[0],vtx1);
			inMesh.getPoint(edgeVtxIndices[1],vtx2);
			MVector edgeVec = vtx1-vtx2;
			float shrinkage = (float)(edgeVec.length()-origEdgeLengths[i]);
			
			vtxShrinkage[edgeVtxIndices[0]] += shrinkage;
			vtxShrinkCount[edgeVtxIndices[0]] += 1;
			vtxShrinkage[edgeVtxIndices[1]] += shrinkage;
			vtxShrinkCount[edgeVtxIndices[1]] += 1;
		}


		for (i=0; i<inMesh.numVertices(); i++)
		{
			MColor shrinkCol;
			float vtxShrink = vtxShrinkage[i] / (float)vtxShrinkCount[i];
			if (vtxShrink<0.0f)
				shrinkCol = MColor((-1.0f*vtxShrink-shMin)/(shMax-shMin), 0, 0);
			else if (vtxShrink>0.0f)
				shrinkCol = MColor(0,((vtxShrink)-stMin)/(stMax-stMin), 0);
			else
				shrinkCol = MColor(0, 0, 0);
			
			inMesh.setVertexColor(shrinkCol,i);
		}
	}	
	
	MDataHandle outMeshDataHandle = block.outputValue(aOutMesh);

 	outMeshDataHandle.set(inMeshObj);
	outMeshDataHandle.setClean();	
	
	return returnStatus;
}

void* animaShrinkage::creator()
{
	return new animaShrinkage();
}

MStatus animaShrinkage::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	MFnMatrixAttribute mAttr;
	MStatus	stat;

	aOrigMesh = tAttr.create("origMesh", "orm", MFnData::kMesh);
	tAttr.setStorable(true);

	aInMesh = tAttr.create("inMesh", "im", MFnData::kMesh);
	tAttr.setStorable(true);
	aOutMesh = tAttr.create("outMesh", "om", MFnData::kMesh);
	tAttr.setStorable(true);
	tAttr.setReadable(true);
	tAttr.setWritable(false);

	aShrinkMin = nAttr.create("shrinkMin", "smi", MFnNumericData::kFloat, 0.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);

	aShrinkMax = nAttr.create("shrinkMax", "sma", MFnNumericData::kFloat, 1.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);

	aStretchMin = nAttr.create("stretchMin", "stm", MFnNumericData::kFloat, 0.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);

	aStretchMax = nAttr.create("stretchMax", "stx", MFnNumericData::kFloat, 1.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);



	// Add the attributes we have created to the node
	addAttribute(aOrigMesh);
	addAttribute(aInMesh);
	addAttribute(aOutMesh);
	
	addAttribute(aShrinkMin);
	addAttribute(aShrinkMax);
	addAttribute(aStretchMin);
	addAttribute(aStretchMax);
	
	attributeAffects(aOrigMesh, aOutMesh);
	attributeAffects(aInMesh, aOutMesh);
	attributeAffects(aShrinkMin, aOutMesh);
	attributeAffects(aShrinkMax, aOutMesh);
	attributeAffects(aStretchMin, aOutMesh);
	attributeAffects(aStretchMax, aOutMesh);

	return MS::kSuccess;

}
