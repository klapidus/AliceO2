// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file GPUChainTracking.h
/// \author David Rohr

#ifndef GPUCHAINTRACKING_H
#define GPUCHAINTRACKING_H

#include "GPUChain.h"
#include "GPUReconstructionHelpers.h"
#include <atomic>
#include <array>
class AliHLTTPCClusterMCLabel;
struct AliHLTTPCRawCluster;

namespace o2
{
namespace trd
{
class TRDGeometryFlat;
}
} // namespace o2::trd

namespace o2
{
namespace TPC
{
struct ClusterNativeAccessFullTPC;
struct ClusterNative;
}
} // namespace o2::TPC

namespace GPUCA_NAMESPACE
{
namespace gpu
{
class GPUTPCSliceOutput;
class GPUTPCSliceOutTrack;
class GPUTPCSliceOutCluster;
class GPUTPCGMMergedTrack;
struct GPUTPCGMMergedTrackHit;
class GPUTRDTrackletWord;
class GPUTPCMCInfo;
class GPUTRDTracker;
class GPUTPCGPUTracker;
struct GPUTPCClusterData;
struct ClusterNativeAccessExt;
struct GPUTRDTrackletLabels;
class GPUDisplay;
class GPUQA;
class GPUTRDGeometry;
class TPCFastTransform;

class GPUChainTracking : public GPUChain, GPUReconstructionHelpers::helperDelegateBase
{
  friend class GPUReconstruction;

 public:
  virtual ~GPUChainTracking() override = default;
  virtual void RegisterPermanentMemoryAndProcessors() override;
  virtual void RegisterGPUProcessors() override;
  virtual int Init() override;
  virtual int Finalize() override;
  virtual int RunStandalone() override;

  // Structures for input and output data
  struct InOutPointers {
    InOutPointers() = default;
    InOutPointers(const InOutPointers&) = default;

    const GPUTPCClusterData* clusterData[NSLICES] = { nullptr };
    unsigned int nClusterData[NSLICES] = { 0 };
    const AliHLTTPCRawCluster* rawClusters[NSLICES] = { nullptr };
    unsigned int nRawClusters[NSLICES] = { 0 };
    const o2::TPC::ClusterNativeAccessFullTPC* clustersNative = nullptr;
    const GPUTPCSliceOutTrack* sliceOutTracks[NSLICES] = { nullptr };
    unsigned int nSliceOutTracks[NSLICES] = { 0 };
    const GPUTPCSliceOutCluster* sliceOutClusters[NSLICES] = { nullptr };
    unsigned int nSliceOutClusters[NSLICES] = { 0 };
    const AliHLTTPCClusterMCLabel* mcLabelsTPC = nullptr;
    unsigned int nMCLabelsTPC = 0;
    const GPUTPCMCInfo* mcInfosTPC = nullptr;
    unsigned int nMCInfosTPC = 0;
    const GPUTPCGMMergedTrack* mergedTracks = nullptr;
    unsigned int nMergedTracks = 0;
    const GPUTPCGMMergedTrackHit* mergedTrackHits = nullptr;
    unsigned int nMergedTrackHits = 0;
    const GPUTRDTrack* trdTracks = nullptr;
    unsigned int nTRDTracks = 0;
    const GPUTRDTrackletWord* trdTracklets = nullptr;
    unsigned int nTRDTracklets = 0;
    const GPUTRDTrackletLabels* trdTrackletsMC = nullptr;
    unsigned int nTRDTrackletsMC = 0;
    friend class GPUReconstruction;
  } mIOPtrs;

  struct InOutMemory {
    InOutMemory();
    ~InOutMemory();
    InOutMemory(InOutMemory&&);
    InOutMemory& operator=(InOutMemory&&);

    std::unique_ptr<GPUTPCClusterData[]> clusterData[NSLICES];
    std::unique_ptr<AliHLTTPCRawCluster[]> rawClusters[NSLICES];
    std::unique_ptr<o2::TPC::ClusterNative[]> clustersNative[NSLICES * GPUCA_ROW_COUNT];
    std::unique_ptr<GPUTPCSliceOutTrack[]> sliceOutTracks[NSLICES];
    std::unique_ptr<GPUTPCSliceOutCluster[]> sliceOutClusters[NSLICES];
    std::unique_ptr<AliHLTTPCClusterMCLabel[]> mcLabelsTPC;
    std::unique_ptr<GPUTPCMCInfo[]> mcInfosTPC;
    std::unique_ptr<GPUTPCGMMergedTrack[]> mergedTracks;
    std::unique_ptr<GPUTPCGMMergedTrackHit[]> mergedTrackHits;
    std::unique_ptr<GPUTRDTrack[]> trdTracks;
    std::unique_ptr<GPUTRDTrackletWord[]> trdTracklets;
    std::unique_ptr<GPUTRDTrackletLabels[]> trdTrackletsMC;
  } mIOMem;

  // Read / Dump / Clear Data
  void ClearIOPointers();
  void AllocateIOMemory();
  using GPUChain::DumpData;
  void DumpData(const char* filename);
  using GPUChain::ReadData;
  int ReadData(const char* filename);
  virtual void DumpSettings(const char* dir = "") override;
  virtual void ReadSettings(const char* dir = "") override;

  // Converter functions
  void ConvertNativeToClusterData();

  // Getters for external usage of tracker classes
  GPUTRDTracker* GetTRDTracker() { return &workers()->trdTracker; }
  GPUTPCTracker* GetTPCSliceTrackers() { return workers()->tpcTrackers; }
  const GPUTPCTracker* GetTPCSliceTrackers() const { return workers()->tpcTrackers; }
  const GPUTPCGMMerger& GetTPCMerger() const { return workers()->tpcMerger; }
  GPUTPCGMMerger& GetTPCMerger() { return workers()->tpcMerger; }
  GPUDisplay* GetEventDisplay() { return mEventDisplay.get(); }
  const GPUQA* GetQA() const { return mQA.get(); }
  GPUQA* GetQA() { return mQA.get(); }

  // Processing functions
  int RunTPCTrackingSlices();
  virtual int RunTPCTrackingMerger();
  virtual int RunTRDTracking();
  int DoTRDGPUTracking();

  // Getters / setters for parameters
  const TPCFastTransform* GetTPCTransform() const { return mTPCFastTransform.get(); }
  const GPUTRDGeometry* GetTRDGeometry() const { return (GPUTRDGeometry*)mTRDGeometry.get(); }
  const ClusterNativeAccessExt* GetClusterNativeAccessExt() const { return mClusterNativeAccess.get(); }
  void SetTPCFastTransform(std::unique_ptr<TPCFastTransform> tpcFastTransform);
  void SetTRDGeometry(const o2::trd::TRDGeometryFlat& geo);
  void LoadClusterErrors();

  const void* mConfigDisplay = nullptr; // Abstract pointer to Standalone Display Configuration Structure
  const void* mConfigQA = nullptr;      // Abstract pointer to Standalone QA Configuration Structure

 protected:
  struct GPUTrackingFlatObjects : public GPUProcessor {
    GPUChainTracking* mChainTracking = nullptr;
    TPCFastTransform* mTpcTransform = nullptr;
    char* mTpcTransformBuffer = nullptr;
    o2::trd::TRDGeometryFlat* mTrdGeometry = nullptr;
    void* SetPointersFlatObjects(void* mem);
    short mMemoryResFlat = -1;
  };

  struct eventStruct // Must consist only of void* ptr that will hold the GPU event ptrs!
  {
    void* selector[NSLICES];
    void* stream[GPUCA_MAX_STREAMS];
    void* init;
    void* constructor;
  };

  GPUChainTracking(GPUReconstruction* rec);

  int ReadEvent(int iSlice, int threadId);
  void WriteOutput(int iSlice, int threadId);
  int GlobalTracking(int iSlice, int threadId);

  int PrepareProfile();
  int DoProfile();

  // Pointers to tracker classes
  GPUTrackingFlatObjects mFlatObjectsShadow; // Host copy of flat objects that will be used on the GPU
  GPUTrackingFlatObjects mFlatObjectsDevice; // flat objects that will be used on the GPU

  // Display / QA
  std::unique_ptr<GPUDisplay> mEventDisplay;
  bool mDisplayRunning = false;
  std::unique_ptr<GPUQA> mQA;
  bool mQAInitialized = false;

  // Ptr to reconstruction detecto objects
  std::unique_ptr<ClusterNativeAccessExt> mClusterNativeAccess; // Internal memory for clusterNativeAccess
  std::unique_ptr<TPCFastTransform> mTPCFastTransform;          // Global TPC fast transformation object
  std::unique_ptr<o2::trd::TRDGeometryFlat> mTRDGeometry;       // TRD Geometry

  HighResTimer timerTPCtracking[NSLICES][10];
  eventStruct mEvents;
  std::ofstream mDebugFile;

#ifdef __ROOT__ // ROOT5 BUG: cint doesn't do volatile
#define volatile
#endif
  volatile int mSliceOutputReady = 0;
  volatile char mSliceLeftGlobalReady[NSLICES] = { 0 };
  volatile char mSliceRightGlobalReady[NSLICES] = { 0 };
#ifdef __ROOT__
#undef volatile
#endif
  std::array<char, NSLICES> mGlobalTrackingDone;
  std::array<char, NSLICES> mWriteOutputDone;

 private:
  int RunTPCTrackingSlices_internal();
  std::atomic_flag mLockAtomic = ATOMIC_FLAG_INIT;

  int HelperReadEvent(int iSlice, int threadId, GPUReconstructionHelpers::helperParam* par);
  int HelperOutput(int iSlice, int threadId, GPUReconstructionHelpers::helperParam* par);
};
}
} // namespace GPUCA_NAMESPACE::gpu

#endif
