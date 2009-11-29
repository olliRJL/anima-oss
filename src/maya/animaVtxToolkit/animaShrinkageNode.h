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

#ifndef _animaShrinkageNode
#define _animaShrinkageNode

#include <maya/MPxNode.h>
#include <maya/MPxDeformerNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h> 
#include <maya/MFnNumericAttribute.h>
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
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MColorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>
#include <maya/MGlobal.h>
#include <maya/MColor.h>
#include <maya/MFnMesh.h>

#include <iostream>
#include <math.h>
 
class animaShrinkage : public MPxNode
{
public:
				animaShrinkage();
	virtual			~animaShrinkage(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& block );

	static  void*		creator();
	static  MStatus		initialize();

	MFloatArray		origEdgeLengths;
	MFloatArray		edgeShrinkage;

	float			*vtxShrinkage;
	int			*vtxShrinkCount;

public:

	static	MObject		aOrigMesh;
	static	MObject		aInMesh;
	static	MObject		aOutMesh;

	static	MObject		aShrinkMin;
	static  MObject		aShrinkMax;		

	static  MObject		aStretchMin;
	static  MObject		aStretchMax;

	static	MTypeId		id;	
};

#endif
