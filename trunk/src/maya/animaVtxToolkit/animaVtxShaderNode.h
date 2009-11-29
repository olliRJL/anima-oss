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

#ifndef _animaVtxShaderNode
#define _animaVtxShaderNode

#include <maya/MPxNode.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MPxLocatorNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h> 
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnNurbsSurfaceData.h>
#include <maya/MItGeometry.h>
#include <maya/MPlug.h>
#include <maya/MSelectionList.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MFloatVector.h>
#include <maya/MVectorArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MColorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MPlugArray.h>
#include <maya/MRenderUtil.h>
#include <maya/MFnStringData.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFloatMatrix.h>

#include <iostream>
#include <math.h>
 
class animaVtxShader : public MPxNode
{
public:
				animaVtxShader();
	virtual			~animaVtxShader(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& block );

	virtual MStatus		connectionMade (const MPlug& plug,
						const MPlug& otherPlug,
						bool	asSrc);

	virtual MStatus		connectionBroken (const MPlug& plug,
						const MPlug& otherPlug,
						bool	asSrc);

	static  void*		creator();
	static  MStatus		initialize();

	MString	inColorConnectionName;

	MFloatArray in_uCoords;
	MFloatArray in_vCoords;
	MFloatArray in_filterSizes;
	MFloatPointArray in_points;
	MFloatPointArray in_refPoints;
	MFloatVectorArray in_normals;
	MFloatVectorArray in_tanUs;
	MFloatVectorArray in_tanVs;

	bool forceUpdate;

	MObject inMeshObj;
	MFnMesh inMesh;

	MObject refMeshObj;
	MFnMesh refMesh;


	float *vtxShrinkage;
	int *vtxShrinkCount;

public:

	static	MObject		aInMesh;
	static	MObject		aRefMesh;
	static	MObject		aOutMesh;

	static	MObject		aTrigger;

	static  MObject		aSG;
	static  MObject		aInColor;

	static	MTypeId		id;	
};

#endif
