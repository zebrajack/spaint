/**
 * spaint: SemanticVisualiser_CPU.cpp
 */

#include "visualisers/cpu/SemanticVisualiser_CPU.h"

#include "visualisers/shared/SemanticVisualiser_Shared.h"

namespace spaint {

//#################### PUBLIC MEMBER FUNCTIONS ####################

void SemanticVisualiser_CPU::render(const ITMLib::Objects::ITMScene<SpaintVoxel,ITMVoxelIndex> *scene, const ITMLib::Objects::ITMPose *pose,
                                    const ITMLib::Objects::ITMIntrinsics *intrinsics, const ITMLib::Objects::ITMRenderState *renderState,
                                    ITMUChar4Image *outputImage) const
{
  // Set up the label colours.
  // FIXME: These should ultimately be passed in from elsewhere.
  Vector3u labelColours[] =
  {
    Vector3u(255, 255, 255),
    Vector3u(255, 0, 0),
    Vector3u(0, 255, 0),
    Vector3u(0, 0, 255)
  };

  // Shade all of the pixels in the image.
  int imgSize = outputImage->noDims.x * outputImage->noDims.y;
  Vector3f lightSource = -Vector3f(pose->GetInvM().getColumn(2));
  Vector4u *outRendering = outputImage->GetData(MEMORYDEVICE_CPU);
  const Vector4f *pointsRay = renderState->raycastResult->GetData(MEMORYDEVICE_CPU);
  const SpaintVoxel *voxelData = scene->localVBA.GetVoxelBlocks();
  const ITMVoxelIndex::IndexData *voxelIndex = scene->index.getIndexData();

#ifdef WITH_OPENMP
  #pragma omp parallel for
#endif
  for (int locId = 0; locId < imgSize; ++locId)
  {
    Vector4f ptRay = pointsRay[locId];
    shade_pixel_semantic(outRendering[locId], ptRay.toVector3(), ptRay.w > 0, voxelData, voxelIndex, lightSource, labelColours);
  }
}

}