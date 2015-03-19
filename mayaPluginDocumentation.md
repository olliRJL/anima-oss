


---


# animaShaderSampler #

## animaShaderSampler ##
**Description:**
> A node that samples the connected shader at points specified with connected matrices. Can also use user specified uv-values. Samples multiple points in one go.

**Usage:**
> Check animaShaderSampler.mel

| **INPUTS** | **type** | **description** |
|:-----------|:---------|:----------------|
| .samplePosMatrixArray | matrixArray | array of xform matrices - connect worldMatrices here |
| .uArray| floatArray | array of u-values  |
| .vArray| floatArray | array of v-values  |
| .inColor | float3 | connect shading network here |
| .SG | string | shading group name if one wants to sample a shading group (with lights and all) instead of a plain shading network (overrides the connection at .inColor). Doesn't really make sense here though as there's no normals or anything :D |

| **OUTPUTS** | **type** | **description** |
|:------------|:---------|:----------------|
| .outColorArray | float3Array | array of output colors |


---


# animaVtxToolkit #

## animaWeightedBlend ##
**Description:**
> A blendShape deformer with three target shapes. Can use vtxColors as per vertex weights - components R, G and B relate to targets A, B and C.

> In "targetSequential" mode the targets are driven in a sequential way with the red component. The targets are placed at values specified with the weightA, weightB and weightC attrs. The weights need to be 0.0 <= wA <= wB <= wC.

> Base mesh and the targets must have matching topologies!

**Usage:**
> select mesh -> "deformer -type animaWeightedBlend" -> connect target meshes to the node

> or...

```
// animaWeightedBlend + animaVtxShader usage
// select baseMesh targetMesh (in this order)
// then:

string $seles[] = `ls -sl`;
string $aVS = `createNode animaVtxShader`;
string $orig[] = `duplicate -name ($seles[0]+"_orig") $seles[0]`;
hide $orig[0];
connectAttr -f ($orig[0]+".worldMesh") ($aVS+".inMesh");
connectAttr -f ($aVS+".outMesh") ($seles[0]+".inMesh");
string $defo[] = `deformer -type animaWeightedBlend $seles[0]`;
connectAttr -f ($seles[1]+".worldMesh") ($defo[0]+".targetA");
setAttr ($defo[0]+".useVertexColorScale") 1;
```

| **INPUTS** | **type** | **description** |
|:-----------|:---------|:----------------|
| .targetA | mesh | targetA mesh |
| .targetB | mesh | targetB mesh |
| .targetC | mesh | targetC mesh |
| .weightA | float | targetA weight |
| .weightB | float | targetB weight |
| .weightC | float | targetC weight |
| .targetsSequential | bool | switch for the sequential mode |
| .baseMeshDeforms | bool | tick this on if the input baseMesh deforms - otherwise we optimize by not recomputing the deformation deltas all the time |
| .useVertexColorScale | bool | switch for the "use vtxColors as weights" mode |

## animaVtxShader ##
**Description:**
> Samples the connected shader at the vertices and outputs the results as vertex colors on the mesh.

| **INPUTS** | **type** | **description** |
|:-----------|:---------|:----------------|
| .inMesh | mesh |input mesh - use "worldMesh" |
| .refMesh | mesh | optional "texture reference mesh" - use "worldMesh" |
| .inColor | float3 | connect shading network here |
| .SG | string | shading group name if one wants to sample a shading group (with lights and all) instead of a plain shading network (overrides the connection at .inColor) |
| .trigger | message |use this to force update on the node when nothing is actually "dirtied" in the shader - connect for example time or the worldMatrix of a light |

| **OUTPUTS** | **type** | **description** |
|:------------|:---------|:----------------|
| .outMesh | mesh | the output mesh with the freshly computed vtxColors |


## animaShrinkage ##
**Description:**
> Generates vtxColors based on how much the mesh shrinks/stretches. Red = shrinkage and Green = stretch. Can be used for example to drive deterministic cloth wrinkling effects.

| **INPUTS** | **type** | **description** |
|:-----------|:---------|:----------------|
| .origMesh | mesh | the mesh at "rest pose" (should not deform)|
| .inMesh | mesh | the input mesh (normally the rigged mesh)|
| .shrinkMin | float | amount of shrink which becomes R=0.0 |
| .shrinkMax | float | amount of shrink which becomes R=1.0 |
| .stretchMin | float | amount of stretch which becomes G=0.0 |
| .stretchMax | float | amount of stretch which becomes G=1.0 |

| **OUTPUTS** | **type** | **description** |
|:------------|:---------|:----------------|
| .outMesh | mesh | the output mesh with the vtxColors |



---
