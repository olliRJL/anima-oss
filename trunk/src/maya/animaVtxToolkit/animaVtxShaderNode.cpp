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

#include "animaVtxShaderNode.h"

 
MTypeId     animaVtxShader::id(0x00114970);

// attributes
MObject     animaVtxShader::aInMesh;
MObject     animaVtxShader::aRefMesh;
MObject     animaVtxShader::aOutMesh;
MObject     animaVtxShader::aTrigger;
MObject     animaVtxShader::aSG;
MObject     animaVtxShader::aInColor;

animaVtxShader::animaVtxShader() {}

animaVtxShader::~animaVtxShader() {}

MStatus animaVtxShader::connectionMade(const MPlug& plug, const MPlug& otherPlug, bool asSrc)
{ 
	MStatus returnStatus;
	if (plug == aInColor)
	{
		cout << "[aVS] connected "<< otherPlug.name() << " to inColor" << endl;
		inColorConnectionName = otherPlug.name();
	}
	MPlug inColPlug(plug.node(),aInColor);
	if (plug == inColPlug.child(0) || plug == inColPlug.child(1) || plug == inColPlug.child(2))
	{
		cout << "[aVS] connected "<< otherPlug.name() << " to a component of inColor" << endl;
		cout << "[aVS] we don't support connections to color components (yet) - please connect to \".inColor\"" << endl;
	}

	return MPxNode::connectionMade(plug, otherPlug, asSrc);
}

MStatus animaVtxShader::connectionBroken(const MPlug& plug, const MPlug& otherPlug, bool asSrc)
{ 
	MStatus returnStatus;
	if (plug == aInColor)
	{
		inColorConnectionName = "";
	}
	
	forceUpdate = true;
	
	return MPxNode::connectionBroken(plug, otherPlug, asSrc);
}

MStatus	animaVtxShader::compute(const MPlug& plug, MDataBlock& block)
{
	MStatus returnStatus;

	// get attributes
	MPlug inMeshPlug(thisMObject(),aInMesh);
	bool inMeshOn = inMeshPlug.isConnected();
	
	MPlug refMeshPlug(thisMObject(),aRefMesh);
	bool refMeshOn = refMeshPlug.isConnected();
	
	if (inMeshOn && (!block.isClean(aInMesh) || forceUpdate))
	{
		// inMesh
		MDataHandle inMeshData = block.inputValue(aInMesh, &returnStatus);
		if (MS::kSuccess != returnStatus) return returnStatus;
		inMeshObj = inMeshData.asMesh();	
		inMesh.setObject(inMeshObj);
	
		// clear arrays
		in_uCoords.clear();
		in_vCoords.clear();
		in_filterSizes.clear();
		in_points.clear();
		if (!refMeshOn) in_refPoints.clear();
		in_normals.clear();
		in_tanUs.clear();
		in_tanVs.clear();
		
		// get values from mesh
		int i;	
		for (i=0; i<inMesh.numVertices(); i++)
		{
			MPoint vtx;
			inMesh.getPoint(i,vtx, MSpace::kWorld);
			in_points.append(vtx.x,vtx.y,vtx.z,1.0);
			if (!refMeshOn) in_refPoints.append(vtx.x,vtx.y,vtx.z,1.0);
			
			MVector normal;
			inMesh.getVertexNormal(i,normal, MSpace::kWorld);
			in_normals.append(normal);
		}

		// get uvs from mesh
		in_uCoords.setLength(inMesh.numVertices());
		in_vCoords.setLength(inMesh.numVertices());
		
		MItMeshPolygon itPoly(inMeshObj);
		MIntArray vtxIndices;
		MFloatArray faceUs;
		MFloatArray faceVs;
		// write each tex coord
		while(!itPoly.isDone()) {
			if (itPoly.hasUVs())
			{
				itPoly.getVertices(vtxIndices);
				itPoly.getUVs(faceUs,faceVs);
				int j;
				for (j=0; j<vtxIndices.length(); j++)
				{
					in_uCoords[vtxIndices[j]] = faceUs[j];
					in_vCoords[vtxIndices[j]] = faceVs[j];
				}
			}
			itPoly.next();
		}

	}

	if (refMeshOn && (!block.isClean(aRefMesh) || forceUpdate))
	{
		// inMesh
		MDataHandle refMeshData = block.inputValue(aRefMesh, &returnStatus);
		if (MS::kSuccess != returnStatus) return returnStatus;
		refMeshObj = refMeshData.asMesh();	
		refMesh.setObject(refMeshObj);
	
		// clear arrays
		in_refPoints.clear();
		
		// get values from mesh
		refMesh.getPoints(in_refPoints,MSpace::kWorld);
	}


	MDataHandle triggerData = block.inputValue(aTrigger, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float trig = triggerData.asFloat();	
	
	MDataHandle sgData = block.inputValue(aSG, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	MString SGstr = sgData.asString();  
	
	MDataHandle inColorData = block.inputValue(aInColor, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	MFloatVector inCol = inColorData.asFloatVector();	
	
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
	

	if (inMeshOn && networkToSample != "")
	{
		int i;

		MFloatArray uCoords = in_uCoords;
		MFloatArray vCoords = in_vCoords;
		MFloatArray filterSizes = in_filterSizes;
		MFloatPointArray points = in_points;
		MFloatPointArray refPoints = in_refPoints;
		MFloatVectorArray normals = in_normals;
		MFloatVectorArray tanUs = in_tanUs;
		MFloatVectorArray tanVs = in_tanVs;


		// maybe create attributes for these too?
		bool shadow = 0;
		bool reuse = 0;

		// sample the connected shader
		MDagPath cameraPath;
		M3dView::active3dView().getCamera(cameraPath);
		MMatrix mat = cameraPath.inclusiveMatrix();
		MFloatMatrix cameraMat(mat.matrix);
		
		MFloatVectorArray colors, transps;
		if (MS::kSuccess == MRenderUtil::sampleShadingNetwork(
					networkToSample, 
					in_points.length(),
					shadow,
					reuse,
					cameraMat,
					(points.length()>0)      ? &points : NULL,
					(uCoords.length()>0)     ? &uCoords : NULL,
					(vCoords.length()>0)     ? &vCoords : NULL,
					(normals.length()>0)     ? &normals : NULL,
					(refPoints.length()>0)   ? &refPoints : NULL,
					(tanUs.length()>0)       ? &tanUs : NULL,
					(tanVs.length()>0)       ? &tanVs : NULL,
					(filterSizes.length()>0) ? &filterSizes : NULL,
					colors,
					transps))
		{
			MIntArray vtxs;
			MColorArray Ccols;
			// write the values to vtxColors
			for (i=0; i<in_points.length(); i++)
			{
				vtxs.append(i);
				Ccols.append(MColor(colors[i].x,colors[i].y,colors[i].z,1.0));
			}
			inMesh.setVertexColors(Ccols,vtxs);
		}
			float randu = (float)rand()/(float)RAND_MAX;
	}
	MDataHandle outMeshDataHandle = block.outputValue(aOutMesh);

	outMeshDataHandle.set(inMeshObj);
	outMeshDataHandle.setClean();
		
	forceUpdate = false;
	
	return returnStatus;
}

void* animaVtxShader::creator()
{
	return new animaVtxShader();
}

MStatus animaVtxShader::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute tAttr;
	MFnMatrixAttribute mAttr;
	MFnMessageAttribute meAttr;
	MStatus	stat;

	aInMesh = tAttr.create("inMesh", "im", MFnData::kMesh);
	tAttr.setStorable(true);
	aRefMesh = tAttr.create("refMesh", "rem", MFnData::kMesh);
	tAttr.setStorable(true);
	aOutMesh = tAttr.create("outMesh", "om", MFnData::kMesh);
	tAttr.setStorable(true);
	tAttr.setReadable(true);
	tAttr.setWritable(false);

	aTrigger = meAttr.create("trigger", "tri");
	meAttr.setWritable(true);
	meAttr.setReadable(true);
	meAttr.setHidden(false);
	meAttr.setStorable(true);

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


	// Add the attributes to the node
	addAttribute(aInMesh);
	addAttribute(aRefMesh);
	addAttribute(aOutMesh);
	addAttribute(aTrigger);
	addAttribute(aSG);
	addAttribute(aInColor);

	attributeAffects(aInMesh, aOutMesh);
	attributeAffects(aRefMesh, aOutMesh);
	attributeAffects(aTrigger, aOutMesh);
	attributeAffects(aSG, aOutMesh);
	attributeAffects(aInColor , aOutMesh);

	return MS::kSuccess;
}
