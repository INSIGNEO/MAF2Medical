/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medGizmoCrossRotate.cpp,v $
Language:  C++
Date:      $Date: 2010-11-10 16:53:03 $
Version:   $Revision: 1.1.2.4 $
Authors:   Stefano Perticoni
==========================================================================
Copyright (c) 2002/2004 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "medGizmoCrossRotate.h"
#include "mafDecl.h"
#include "mafGUIGizmoRotate.h"
#include "mafSmartPointer.h"

#include "mafInteractorGenericMouse.h"

#include "mafMatrix.h"
#include "mafTransform.h"
#include "mafVME.h"
#include "mafVMEOutput.h"

#include "vtkTransform.h"
#include "medGizmoCrossRotateAxis.h"
#include "medGizmoCrossRotateFan.h"

//----------------------------------------------------------------------------
medGizmoCrossRotate::medGizmoCrossRotate(mafVME* input, mafObserver *listener, bool buildGUI , int axis)
//----------------------------------------------------------------------------
{
	axis = Z;
	assert(input);
	m_BuildGUI = buildGUI;
	m_InputVME    = input;
	m_Listener  = listener;

	m_GRFan = NULL;
	m_GRCircle = NULL;
	m_GuiGizmoRotate = NULL;
	m_CircleFanRadius = -1;

	// create the fan and send events to this
	m_GRFan = new medGizmoCrossRotateFan(input, this);
	m_GRFan->SetAxis(axis);

	// Create mafGizmoRotateCircle and send events to the corresponding fan
	m_GRCircle = new medGizmoCrossRotateAxis(input, m_GRFan);
	m_GRCircle->SetAxis(axis);
	m_GRCircle->SetMediator(this);

	if (m_BuildGUI)
	{
		// create the gizmo gui
		// gui is sending events to this
		m_GuiGizmoRotate = new mafGUIGizmoRotate(this);

		// initialize gizmo gui
		m_GuiGizmoRotate->SetAbsOrientation(m_InputVME->GetOutput()->GetAbsMatrix());
	}
}
//----------------------------------------------------------------------------
medGizmoCrossRotate::~medGizmoCrossRotate() 
//----------------------------------------------------------------------------
{
	//Destroy:
	//1 mafGizmoRotateCircle 
	//1 mafGizmoRotateFan

	cppDEL(m_GRCircle);
	cppDEL(m_GRFan);
	cppDEL(m_GuiGizmoRotate);
}

//----------------------------------------------------------------------------
void medGizmoCrossRotate::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	// get the sender
	void *sender = maf_event->GetSender();

	if (sender == m_GRFan)
	{
		OnEventGizmoComponents(maf_event); // process events from fans
	}
	else if (sender == m_GuiGizmoRotate)
	{
		OnEventGizmoGui(maf_event); // process events from gui
	}
	else
	{
		// forward to the listener
		mafEventMacro(*maf_event);
	}
}

//----------------------------------------------------------------------------
void medGizmoCrossRotate::OnEventGizmoComponents(mafEventBase *maf_event) 
//----------------------------------------------------------------------------
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		switch (e->GetId())
		{
			//receiving pose matrices from the fan
		case ID_TRANSFORM:
			{
				void *sender = e->GetSender();
				long arg = e->GetArg();

				// if a gizmo circle has been picked
				if (arg == mafInteractorGenericMouse::MOUSE_DOWN)
				{
					if (sender == m_GRFan)
					{
						this->Highlight(true);
					}
				}  
				else if (arg == mafInteractorGenericMouse::MOUSE_MOVE)
				{
					// gizmo mode == local; gizmo is rotating during mouse move events
					if (m_Modality == G_LOCAL)
					{
						// get the old abs pose
						vtkTransform *currTr = vtkTransform::New();
						currTr->PostMultiply();
						currTr->SetMatrix(GetAbsPose()->GetVTKMatrix());
						currTr->Concatenate(e->GetMatrix()->GetVTKMatrix());
						currTr->Update();

						mafMatrix newAbsMatr;
						newAbsMatr.DeepCopy(currTr->GetMatrix());
						newAbsMatr.SetTimeStamp(GetAbsPose()->GetTimeStamp());

						// set the new pose to the gizmo
						SetAbsPose(&newAbsMatr, false);
						currTr->Delete();
					}
				}
				else if (arg == mafInteractorGenericMouse::MOUSE_UP)
				{
					this->Highlight(false);

					// gizmo mode == local
					if (m_Modality == G_LOCAL)
					{
						SetAbsPose(GetAbsPose());
					}
				}

				// forward event to the listener ie the operation
				// instanciating the gizmo; the sender is changed to "this" so that the operation can check for
				// the gizmo sending events
				e->SetSender(this);
				mafEventMacro(*e);
			}
			break;
		default:
			{
				mafEventMacro(*e);
			}
		}
	}
}

//----------------------------------------------------------------------------
void medGizmoCrossRotate::OnEventGizmoGui(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	// process events from the gui
	switch (maf_event->GetId())
	{
	case (mafGUIGizmoRotate::ID_ROTATE_X):
		{
			// receiving abs orientation from gui
			SendTransformMatrixFromGui(maf_event);
		}
		break;
	case (mafGUIGizmoRotate::ID_ROTATE_Y):
		{
			SendTransformMatrixFromGui(maf_event);     
		}
		break;
	case (mafGUIGizmoRotate::ID_ROTATE_Z):
		{
			SendTransformMatrixFromGui(maf_event);     
		}
		break;
	default:
		{
			mafEventMacro(*maf_event);
		}
		break;
	}
}

//----------------------------------------------------------------------------
void medGizmoCrossRotate::Highlight (bool highlight) 
//----------------------------------------------------------------------------
{
	m_GRCircle->Highlight(highlight);
	m_GRFan->Show(highlight);
}

//----------------------------------------------------------------------------  
void medGizmoCrossRotate::Show(bool show)
//----------------------------------------------------------------------------
{
	// set visibility ivar
	m_Visibility = show;

	m_GRCircle->Show(show);
	m_GRFan->Show(show);

	// if auxiliary ref sys is different from vme its orientation cannot be changed
	// so gui must not be keyable. Otherwise set gui keyability to show.
	if (m_BuildGUI)
	{
		if (m_RefSysVME == m_InputVME)
		{
			m_GuiGizmoRotate->EnableWidgets(show);
		}
		else
		{
			m_GuiGizmoRotate->EnableWidgets(false);
		}
	}
}

//----------------------------------------------------------------------------  
void medGizmoCrossRotate::SetAbsPose(mafMatrix *absPose, bool applyPoseToFans)
//----------------------------------------------------------------------------
{
	// remove scaling part from gizmo abs pose; gizmo not scale
	double pos[3] = {0,0,0};
	double orient[3] = {0,0,0};

	mafTransform::GetPosition(*absPose, pos);
	mafTransform::GetOrientation(*absPose, orient);

	mafSmartPointer<mafMatrix> tmpMatr;
	tmpMatr->SetTimeStamp(absPose->GetTimeStamp());
	mafTransform::SetPosition(*tmpMatr.GetPointer(), pos);
	mafTransform::SetOrientation(*tmpMatr.GetPointer(), orient);

	for (int i = 0; i < 3; i++)
	{
		m_GRCircle->SetAbsPose(tmpMatr);
		if (applyPoseToFans == true)
		{
			m_GRFan->SetAbsPose(tmpMatr);
		}
	}
	if (m_BuildGUI) m_GuiGizmoRotate->SetAbsOrientation(tmpMatr);
}

//----------------------------------------------------------------------------
mafMatrix *medGizmoCrossRotate::GetAbsPose()
//----------------------------------------------------------------------------
{
	return m_GRCircle->GetAbsPose();
}

//----------------------------------------------------------------------------  
void medGizmoCrossRotate::SetInput(mafVME *input)
//----------------------------------------------------------------------------
{
	m_InputVME = input;

	m_GRCircle->SetInput(input);
	m_GRFan->SetInput(input);

}

//----------------------------------------------------------------------------  
mafInteractorGenericInterface *medGizmoCrossRotate::GetInteractor(int axis)
//----------------------------------------------------------------------------  
{
	return m_GRCircle->GetInteractor();
}

//----------------------------------------------------------------------------
void medGizmoCrossRotate::SendTransformMatrixFromGui(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
	{
		// send matrix to be postmultiplied to listener
		//                                                                  -1    
		// [NewAbsPose] = [M]*[OldAbsPose] => [M] = [NewAbsPose][OldAbsPose]

		// build objects
		mafSmartPointer<mafMatrix> M;
		mafMatrix invOldAbsPose;
		mafSmartPointer<mafMatrix> newAbsPose;

		// incoming matrix is a rotation matrix
		newAbsPose->DeepCopy(GetAbsPose());
		// copy rotation from incoming matrix
		mafTransform::CopyRotation(*e->GetMatrix(), *newAbsPose.GetPointer());

		invOldAbsPose.DeepCopy(this->GetAbsPose());
		invOldAbsPose.Invert();

		mafMatrix::Multiply4x4(*newAbsPose.GetPointer(),invOldAbsPose,*M.GetPointer());

		// update gizmo abs pose
		this->SetAbsPose(newAbsPose, true);

		// send transfrom to postmultiply to the listener. Events is sent as a transform event
		SendTransformMatrix(M, ID_TRANSFORM, mafInteractorGenericMouse::MOUSE_MOVE);
	}
}

//----------------------------------------------------------------------------
void medGizmoCrossRotate::SetRefSys(mafVME *refSys)
//----------------------------------------------------------------------------
{
	assert(m_InputVME);  

	m_RefSysVME = refSys;
	SetAbsPose(m_RefSysVME->GetOutput()->GetAbsMatrix());

	if (m_RefSysVME == m_InputVME)
	{
		SetModalityToLocal();

		// if the gizmo is visible set the widgets visibility to true
		// if the refsys is local
		if (m_Visibility == true && m_BuildGUI == true)
		{
			m_GuiGizmoRotate->EnableWidgets(true);
		}
	}
	else
	{
		SetModalityToGlobal();    

		// if the gizmo is visible set the widgets visibility to false
		// if the refsys is global since this refsys cannot be changed
		if (m_Visibility == true && m_BuildGUI == true)
		{
			m_GuiGizmoRotate->EnableWidgets(false);
		}
	}
}
//----------------------------------------------------------------------------
void medGizmoCrossRotate::SetCircleFanRadius(double radius)
//----------------------------------------------------------------------------
{
	short circleNumber;

	//  if(m_GRCircle) m_GRCircle->SetRadius(radius);
	if(m_GRFan) m_GRFan->SetRadius(radius);

	m_CircleFanRadius = radius;
}

mafVME* medGizmoCrossRotate::GetRefSys()
{
	return m_RefSysVME;
}

double medGizmoCrossRotate::GetCircleFanRadius()
{
	return m_CircleFanRadius;
}

void medGizmoCrossRotate::SetAutoscale( bool autoscale )
{
	mafGizmoInterface::SetAutoscale(autoscale);

	m_GRFan->SetAutoscale(autoscale);
	m_GRCircle->SetAutoscale(autoscale);
}

void medGizmoCrossRotate::SetAlwaysVisible( bool alwaysVisible )
{
	mafGizmoInterface::SetAlwaysVisible(alwaysVisible);

	m_GRFan->SetAlwaysVisible(alwaysVisible);
	m_GRCircle->SetAlwaysVisible(alwaysVisible);
}

void medGizmoCrossRotate::SetRenderWindowHeightPercentage(double percentage)
{
	mafGizmoInterface::SetRenderWindowHeightPercentage(percentage);

	m_GRFan->SetRenderWindowHeightPercentage(percentage);
	m_GRCircle->SetRenderWindowHeightPercentage(percentage);
}