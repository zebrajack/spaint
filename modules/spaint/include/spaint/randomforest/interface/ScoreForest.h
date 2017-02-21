/**
 * spaint: ScoreForest.h
 * Copyright (c) Torr Vision Group, University of Oxford, 2016. All rights reserved.
 */

#ifndef H_SPAINT_SCOREFOREST
#define H_SPAINT_SCOREFOREST

#include <vector>

#include <grove/features/interface/RGBDPatchFeatureCalculator.h>
using namespace grove;

#include "../../util/ITMImagePtrTypes.h"

#include "ScoreClusterer.h"
#include "ExampleReservoirs.h"
#include "../ScoreForestTypes.h"

#include "ORUtils/Vector.h"

#ifdef WITH_SCOREFORESTS
// Forward declare stuff to avoid cluttering the global namespace with the entirety of ScoreForests classes.
class EnsembleLearner;
class Learner;
class PredictionGaussianMean;
#endif

namespace spaint
{

class ScoreForest
{
  // Typedefs
public:
  struct NodeEntry
  {
    int leftChildIdx; // No need to store the right child, it's left + 1
    int leafIdx;  // Index of the associated leaf (-1 if the node is not a leaf)
    int featureIdx;   // Index of the feature to evaluate;
    float featureThreshold; // Feature threshold
  };

  typedef ORUtils::Image<NodeEntry> NodeImage;
  typedef boost::shared_ptr<ORUtils::Image<NodeEntry> > NodeImage_Ptr;
  typedef boost::shared_ptr<const ORUtils::Image<NodeEntry> > NodeImage_CPtr;

  typedef spaint::LeafIndices LeafIndices; // TODO: remove
  typedef ORUtils::Image<LeafIndices> LeafIndicesImage;
  typedef boost::shared_ptr<LeafIndicesImage> LeafIndicesImage_Ptr;
  typedef boost::shared_ptr<const LeafIndicesImage> LeafIndicesImage_CPtr;

public:
  explicit ScoreForest(const std::string &fileName);
  virtual ~ScoreForest();

  void reset_predictions();
  void evaluate_forest(const RGBDPatchDescriptorImage_CPtr &descriptors,
      ScorePredictionsImage_Ptr &predictions);
  void add_features_to_forest(const Keypoint3DColourImage_CPtr &keypoints,
      const RGBDPatchDescriptorImage_CPtr &descriptors);
  void update_forest();

  void load_structure_from_file(const std::string &fileName);
  void save_structure_to_file(const std::string &fileName) const;

  size_t get_nb_trees() const;
  size_t get_nb_nodes_in_tree(size_t treeIdx) const;
  size_t get_nb_leaves_in_tree(size_t treeIdx) const;
  virtual ScorePrediction get_prediction(size_t treeIdx,
      size_t leafIdx) const = 0;

protected:
  std::vector<int> m_nbNodesPerTree;
  std::vector<int> m_nbLeavesPerTree;

  NodeImage_Ptr m_nodeImage;
  ScorePredictionsBlock_Ptr m_predictionsBlock;
  PositionReservoir_Ptr m_leafReservoirs;
  ScoreClusterer_Ptr m_gpuClusterer;

  size_t m_reservoirCapacity;
  size_t m_maxReservoirsToUpdate;
  size_t m_lastFeaturesAddedStartIdx;
  size_t m_reservoirUpdateStartIdx;

  virtual void find_leaves(const RGBDPatchDescriptorImage_CPtr &descriptors,
      LeafIndicesImage_Ptr &leaf_indices) const = 0;
  virtual void get_predictions(const LeafIndicesImage_Ptr &leaf_indices,
      ScorePredictionsImage_Ptr &predictions) const = 0;

private:
  ScoreForest();

  LeafIndicesImage_Ptr m_leafImage;

  //#################### SCOREFOREST INTEROP FUNCTIONS ####################
#ifdef WITH_SCOREFORESTS
public:
  explicit ScoreForest(const EnsembleLearner &pretrained_forest);

private:
  int convert_node(const Learner *learner, int node_idx, int tree_idx,
      int n_trees, int output_idx, int first_free_idx, NodeEntry *gpu_nodes,
      std::vector<PredictionGaussianMean> &leafPredictions);
  void convert_predictions(const std::vector<PredictionGaussianMean> &leafPredictions);
#endif
};

typedef boost::shared_ptr<ScoreForest> ScoreForest_Ptr;
typedef boost::shared_ptr<const ScoreForest> ScoreForest_CPtr;

}

#endif