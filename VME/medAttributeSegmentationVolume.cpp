/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medAttributeSegmentationVolume.cpp,v $
Language:  C++
Date:      $Date: 2010-04-19 14:19:18 $
Version:   $Revision: 1.1.2.1 $
Authors:   Matteo Giacomoni
==========================================================================
Copyright (c) 2010
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "medDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "medAttributeSegmentationVolume.h"
#include "medDecl.h"
#include "mafEvent.h"

#include "mafStorageElement.h"
#include "mafIndent.h"
#include "medVMESegmentationVolume.h"

using namespace std;

//----------------------------------------------------------------------------
mafCxxTypeMacro(medAttributeSegmentationVolume)
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
medAttributeSegmentationVolume::medAttributeSegmentationVolume()
//----------------------------------------------------------------------------
{  
  m_Name = "SegmentationVolumeData";

  m_AutomaticSegmentationThresholdModality = medVMESegmentationVolume::GLOBAL;
  m_AutomaticSegmentationGlobalThreshold = 0.0;
}
//----------------------------------------------------------------------------
medAttributeSegmentationVolume::~medAttributeSegmentationVolume()
//----------------------------------------------------------------------------
{
  for (int i=0;i<m_AutomaticSegmentationRanges.size();i++)
  {
    delete []m_AutomaticSegmentationRanges[i];
  }
  m_AutomaticSegmentationRanges.clear();
}
//-------------------------------------------------------------------------
void medAttributeSegmentationVolume::DeepCopy(const mafAttribute *a)
//-------------------------------------------------------------------------
{ 
  if (!a->IsA("medAttributeSegmentationVolume"))
  {
  	return;
  }

  Superclass::DeepCopy(a);

  //////////////////////////////////////////////////////////////////////////
 	m_AutomaticSegmentationThresholdModality = ((medAttributeSegmentationVolume*)a)->GetAutomaticSegmentationThresholdModality();
  m_AutomaticSegmentationGlobalThreshold = ((medAttributeSegmentationVolume*)a)->GetAutomaticSegmentationGlobalThreshold();
  for (int i=0;i<((medAttributeSegmentationVolume*)a)->GetNumberOfRanges();i++)
  {
    int startSlice,endSlice;
    double threshold;
    ((medAttributeSegmentationVolume*)a)->GetRange(i,startSlice,endSlice,threshold);
    this->AddRange(startSlice,endSlice,threshold);
  }
}
//----------------------------------------------------------------------------
bool medAttributeSegmentationVolume::Equals(const mafAttribute *a)
//----------------------------------------------------------------------------
{
  if (Superclass::Equals(a))
  {
    if (m_AutomaticSegmentationThresholdModality != ((medAttributeSegmentationVolume*)a)->GetAutomaticSegmentationThresholdModality())
    {
      return false;
    }

    if (m_AutomaticSegmentationGlobalThreshold != ((medAttributeSegmentationVolume*)a)->GetAutomaticSegmentationGlobalThreshold())
    {
      return false;
    }

    if (m_AutomaticSegmentationRanges.size() != ((medAttributeSegmentationVolume*)a)->GetNumberOfRanges())
    {
      return false;
    }

    for (int i=0;i<((medAttributeSegmentationVolume*)a)->GetNumberOfRanges();i++)
    {
      int startSlice,endSlice;
      double threshold;
      ((medAttributeSegmentationVolume*)a)->GetRange(i,startSlice,endSlice,threshold);
      
      if (startSlice != m_AutomaticSegmentationRanges[i][0] || endSlice!= m_AutomaticSegmentationRanges[i][1] || threshold != m_AutomaticSegmentationThresholds[i])
      {
        return false;
      }
    }

    return true;
  }
  return false;
}
//-----------------------------------------------------------------------
int medAttributeSegmentationVolume::InternalStore(mafStorageElement *parent)
//-----------------------------------------------------------------------
{  
  if (Superclass::InternalStore(parent)==MAF_OK)
  {
    //////////////////////////////////////////////////////////////////////////
    mafString value = "AUTOMATIC_SEGMENTATION_THRESHOLD_MODALITY";
    int valueInt = m_AutomaticSegmentationThresholdModality;
    parent->StoreInteger(value,valueInt);
    //////////////////////////////////////////////////////////////////////////
    value = "AUTOMATIC_SEGMENTATION_GLOBAL_THRESHOLD";
    double valueDouble = m_AutomaticSegmentationGlobalThreshold;
    parent->StoreDouble(value,valueDouble);
    //////////////////////////////////////////////////////////////////////////
    value = "NUM_OF_RANGES";
    parent->StoreInteger(value,m_AutomaticSegmentationRanges.size());
    for (int i=0;i<m_AutomaticSegmentationRanges.size();i++)
    {
      value = "RANGE_";
      value<<i;
      parent->StoreVectorN(value,m_AutomaticSegmentationRanges[i],2);
      value = "THRESHOLD_";
      value<<i;
      parent->StoreDouble(value,m_AutomaticSegmentationThresholds[i]);
    }

    return MAF_OK;
  }
  return MAF_ERROR;
}
//----------------------------------------------------------------------------
int medAttributeSegmentationVolume::InternalRestore(mafStorageElement *node)
//----------------------------------------------------------------------------
{
  if (Superclass::InternalRestore(node) == MAF_OK)
  {
    //////////////////////////////////////////////////////////////////////////
    mafString value = "AUTOMATIC_SEGMENTATION_THRESHOLD_MODALITY";
    node->RestoreInteger(value,m_AutomaticSegmentationThresholdModality);
    //////////////////////////////////////////////////////////////////////////
    value = "AUTOMATIC_SEGMENTATION_GLOBAL_THRESHOLD";
    node->RestoreDouble(value,m_AutomaticSegmentationGlobalThreshold);
    //////////////////////////////////////////////////////////////////////////
    int numOfRanges;
    value = "NUM_OF_RANGES";
    node->RestoreInteger(value,numOfRanges);
    for (int i=0;i<numOfRanges;i++)
    {
      int *range = new int[2];
      double threshold;
      value = "RANGE_";
      value<<i;
      node->RestoreVectorN(value,range,2);
      m_AutomaticSegmentationRanges.push_back(range);
      value = "THRESHOLD_";
      value<<i;
      node->RestoreDouble(value,threshold);
      m_AutomaticSegmentationThresholds.push_back(threshold);
    }

    return MAF_OK;
  }
  return MAF_ERROR;
}
//----------------------------------------------------------------------------
int medAttributeSegmentationVolume::AddRange(int startSlice,int endSlice,double threshold)
//----------------------------------------------------------------------------
{
  int *range = new int[2];
  range[0] = startSlice;
  range[1] = endSlice;
  m_AutomaticSegmentationRanges.push_back(range);
  m_AutomaticSegmentationThresholds.push_back(threshold);

  return MAF_OK;
}
//----------------------------------------------------------------------------
int medAttributeSegmentationVolume::GetRange(int index,int &startSlice, int &endSlice, double &threshold)
//----------------------------------------------------------------------------
{
  if (index<0 || index>m_AutomaticSegmentationRanges.size()-1)
  {
    return MAF_ERROR;
  }

  startSlice = m_AutomaticSegmentationRanges[index][0];
  endSlice = m_AutomaticSegmentationRanges[index][1];
  threshold = m_AutomaticSegmentationThresholds[index];

  return MAF_OK;
}
//----------------------------------------------------------------------------
int medAttributeSegmentationVolume::UpdateRange(int index,int &startSlice, int &endSlice, double &threshold)
//----------------------------------------------------------------------------
{
  if (index<0 || index>m_AutomaticSegmentationRanges.size()-1)
  {
    return MAF_ERROR;
  }
  m_AutomaticSegmentationRanges[index][0] = startSlice;
  m_AutomaticSegmentationRanges[index][1] = endSlice;
  m_AutomaticSegmentationThresholds[index] = threshold;

  return MAF_OK;
}
//----------------------------------------------------------------------------
int medAttributeSegmentationVolume::DeleteRange(int index)
//----------------------------------------------------------------------------
{
  if (index<0 || index>m_AutomaticSegmentationRanges.size()-1)
  {
    return MAF_ERROR;
  }

  for (int i=0,j=0;i<m_AutomaticSegmentationRanges.size();i++)
  {
    if (i != index)
    {
      j++;
      m_AutomaticSegmentationRanges[j][0] = m_AutomaticSegmentationRanges[i][0];
      m_AutomaticSegmentationRanges[j][1] = m_AutomaticSegmentationRanges[i][1];
      m_AutomaticSegmentationThresholds[j] = m_AutomaticSegmentationThresholds[i];
    }
  }

  delete []m_AutomaticSegmentationRanges[m_AutomaticSegmentationRanges.size()-1];
  m_AutomaticSegmentationRanges.pop_back();
  m_AutomaticSegmentationThresholds.pop_back();

  return MAF_OK;
}
//-----------------------------------------------------------------------
void medAttributeSegmentationVolume::Print(std::ostream& os, const int tabs) const
//-----------------------------------------------------------------------
{
  Superclass::Print(os,tabs);
  mafIndent indent(tabs);
  
  //////////////////////////////////////////////////////////////////////////
  os << indent << "SegmentationVolumeData:"<<std::endl;
  if (m_AutomaticSegmentationThresholdModality == medVMESegmentationVolume::GLOBAL)
  {
    os << indent << indent << "AutomaticSegmentationThresholdModality: GLOBAL"<<std::endl;
  }
  else
  {
    os << indent << indent << "AutomaticSegmentationThresholdModality: RANGE"<<std::endl;
  }
  //////////////////////////////////////////////////////////////////////////
  os << indent << indent << "Global Threshold: "<<m_AutomaticSegmentationGlobalThreshold<<std::endl;
  //////////////////////////////////////////////////////////////////////////
  for (int i=0;i<m_AutomaticSegmentationRanges.size();i++)
  {
    os << indent << indent << "Range " << i+1 << "�:";
    os << indent << indent << indent << "Start Slice " << m_AutomaticSegmentationRanges[i][0]<<std::endl;
    os << indent << indent << indent << "End Slice " << m_AutomaticSegmentationRanges[i][1]<<std::endl;
    os << indent << indent << indent << "Threshold " << m_AutomaticSegmentationThresholds[i]<<std::endl;
  }
  //////////////////////////////////////////////////////////////////////////
}