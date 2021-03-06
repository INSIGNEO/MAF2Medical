/*=========================================================================

 Program: MAF2Medical
 Module: medVMEPolylineEditor
 Authors: Matteo Giacomoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __medVMEPolylineEditor_h
#define __medVMEPolylineEditor_h

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "medVMEDefines.h"
#include "mafVMEGeneric.h"

//----------------------------------------------------------------------------
// forward declarations :
//----------------------------------------------------------------------------

class MED_VME_EXPORT medVMEPolylineEditor : public mafVMEGeneric
{
public:
	mafTypeMacro(medVMEPolylineEditor, mafVMEGeneric);

	/** 
	Set data for the give timestamp. This function automatically creates a
	a VMEItem for the data to be stored. The optional parameter specify if
	data must be copied, referenced, or detached from original source. Default
	is to copy data which creates a new data set copying the original one.
	Referencing means the data is kept connected to its original source, and
	detaching means disconnect is like referencing but also disconnect the data
	from the source.
	Return MAF_OK if succeeded, MAF_ERROR if they kind of data is not accepted by
	this type of VME. */
	virtual int SetData(vtkDataSet *data, mafTimeStamp t, int mode=MAF_VME_COPY_DATA);

	/** Return the suggested pipe-typename for the visualization of this vme */
	virtual mafString GetVisualPipe() {return m_VisualPipeName;};

	/** return the right type of output */  
	virtual mafVMEOutput *GetOutput();

	/** Set the suggested pipe-typename for the visualization of this vme */
	void SetVisualPipe(mafString vpipe){m_VisualPipeName = vpipe;};

protected:
	medVMEPolylineEditor();
	virtual ~medVMEPolylineEditor(); 

	mafString m_VisualPipeName;

private:
	medVMEPolylineEditor(const medVMEPolylineEditor&); // Not implemented
	void operator=(const medVMEPolylineEditor&); // Not implemented
};
#endif
