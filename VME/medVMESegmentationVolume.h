/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medVMESegmentationVolume.h,v $
Language:  C++
Date:      $Date: 2010-04-19 14:19:18 $
Version:   $Revision: 1.1.2.1 $
Authors:   Matteo Giacomoni
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/
#ifndef __medVMESegmentationVolume_h
#define __medVMESegmentationVolume_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafVME.h"
#include "mafVMEVolumeGray.h"

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------
class mafTransform;
class mmaVolumeMaterial;
class medDataPipeCustomSegmentationVolume;
class medAttributeSegmentationVolume;

class MAF_EXPORT medVMESegmentationVolume : public mafVME
{
public:
  mafTypeMacro(medVMESegmentationVolume, mafVME);

  enum THRESHOLD_TYPE
  {
    GLOBAL = 0,
    RANGE,
  };

  enum BRUSH_TYPE
  {
    CIRCLE = 0,
    SQUARE,
  };

  enum PLANE_TYPE
  {
    XY = 0,
    XZ,
    YZ,
  };
  
  enum SELECTION_TYPE
  {
    SELECT = 0,
    UNSELECT,
  };

  enum SEGMENTATION_VOLUME_WIDGET_ID
  {
    ID_VOLUME_LINK = Superclass::ID_LAST,
    ID_LAST
  };

  /** Return the suggested pipe-typename for the visualization of this vme */
  /*virtual*/ mafString GetVisualPipe() {return mafString("mafPipeBox");};

  /** Precess events coming from other objects */ 
  /*virtual*/ void OnEvent(mafEventBase *maf_event);

  /** Return pointer to material attribute. */
  mmaVolumeMaterial *GetMaterial();

  /** Copy the contents of another medVMESegmentationVolume into this one. */
  /*virtual*/ int DeepCopy(mafNode *a);

  /** Compare with another medVMESegmentationVolume. */
  /*virtual*/ bool Equals(mafVME *vme);

  /** Set the pose matrix for the Prober. */
  void SetMatrix(const mafMatrix &mat);

  /** Clear the parameter 'kframes' because medVMESegmentationVolume has no timestamp. */
  void GetLocalTimeStamps(std::vector<mafTimeStamp> &kframes);

  /** return always false since (currently) the slicer is not an animated VME (position 
  is the same for all timestamps). */
  /*virtual*/ bool IsAnimated();

  /** Return true if the data associated with the VME is present and updated at the current time.*/
  /*virtual*/ bool IsDataAvailable();

  /** return icon */
  static char** GetIcon();

  /** Set the link to the volume.*/
  void SetVolumeLink(mafNode *volume);

  /** Get the link to the volume.*/
  mafNode *GetVolumeLink();

  /** Return the vtkDataSet of the automatic segmentation */
  vtkDataSet *GetAutomaticOutput();

  /** Set the volume mask for the manual segmentation */
  void SetManualVolumeMask(mafNode *volume);

  /** Get the volume mask for the manual segmentation */
  mafNode *GetManualVolumeMask();

  /** Add a new range with a particular threshold */
  int AddRange(int startSlice,int endSlice,double threshold);

  /** Return the value of the range of the position index */
  int GetRange(int index,int &startSlice, int &endSlice, double &threshold);

  /** Update the value of the range of the position index - return MAF_ERROR if the index isn't correct*/
  int UpdateRange(int index,int startSlice, int endSlice, double threshold);

  /** Delete the range of the position index */
  int DeleteRange(int index);

  /** Return the number of ranges */
  int GetNumberOfRanges();

  /** Set the threshold modality for automatic segmentation: GLOBAL or RANGE */
  void SetAutomaticSegmentationThresholdModality(int modality);

  /** Get the threshold modality for automatic segmentation: GLOBAL or RANGE */
  int GetAutomaticSegmentationThresholdModality();

  /** Set the value to use during a global threshold */
  void SetAutomaticSegmentationGlobalThreshold(double threshold);

  /** Return the value to use during a global threshold */
  double GetAutomaticSegmentationGlobalThreshold();

  /** Check if all thresholds exist */
  bool CheckNumberOfThresholds();

  /** return the right type of output */  
  /*virtual*/ mafVMEOutput *GetOutput();

  static bool VolumeAccept(mafNode *node) {return(node != NULL && node->IsA("mafVMEVolume"));}

protected:
  medVMESegmentationVolume();
  virtual ~medVMESegmentationVolume(); 

  /** Internally used to create a new instance of the GUI.*/
  virtual mafGUI *CreateGui();

  /** used to initialize and create the material attribute if not yet present */
  /*virtual*/ int InternalInitialize();

  /*virtual*/ int InternalStore(mafStorageElement *parent);
  /*virtual*/ int InternalRestore(mafStorageElement *node);

  /** called to prepare the update of the output */
  /*virtual*/ void InternalPreUpdate();

  mafString m_VolumeName;

  mafTransform *m_Transform;

  medDataPipeCustomSegmentationVolume *m_SegmentingDataPipe;

  medAttributeSegmentationVolume *m_VolumeAttribute;

private:
  medVMESegmentationVolume(const medVMESegmentationVolume&); // Not implemented
  void operator=(const medVMESegmentationVolume&); // Not implemented
};
#endif