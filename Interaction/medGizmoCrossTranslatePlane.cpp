/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: medGizmoCrossTranslatePlane.cpp,v $
  Language:  C++
  Date:      $Date: 2010-11-10 16:53:03 $
  Version:   $Revision: 1.1.2.3 $
  Authors:   Stefano Perticoni
==========================================================================
  Copyright (c) 2002/2004 
  CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/


//======================== WORK IN PROGRESS !!!!! ======================== 
//======================== WORK IN PROGRESS !!!!! ======================== 
//======================== WORK IN PROGRESS !!!!! ======================== 
//======================== WORK IN PROGRESS !!!!! ======================== 
//======================== WORK IN PROGRESS !!!!! ======================== 
//======================== WORK IN PROGRESS !!!!! ======================== 
//======================== WORK IN PROGRESS !!!!! ======================== 
//======================== WORK IN PROGRESS !!!!! ======================== 
//======================== WORK IN PROGRESS !!!!! ======================== 

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "medGizmoCrossTranslatePlane.h"
#include "mafDecl.h"

// isa stuff
#include "mafInteractorCompositorMouse.h"
#include "mafInteractorGenericMouse.h"

// vme stuff
#include "mmaMaterial.h"
#include "mafVME.h"
#include "mafVMEGizmo.h"
#include "mafVMEOutput.h"

// vtk stuff
#include "vtkLineSource.h"
#include "vtkPlaneSource.h"
#include "vtkTubeFilter.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkPolyData.h"
#include "vtkMath.h"
#include "vtkProperty.h"
#include "mafGizmoPath.h"

//----------------------------------------------------------------------------
medGizmoCrossTranslatePlane::medGizmoCrossTranslatePlane(mafVME *input, mafObserver *listener)
//----------------------------------------------------------------------------
{
  this->SetIsActive(false);
  
  m_LastColor[0] = 0;
  m_LastColor[1] = 0;
  m_LastColor[2] = 1;

  m_IsaComp[0]  = m_IsaComp[1] =  NULL;
  m_Listener  = listener;
  m_InputVme    = input;
  m_Length = 1;
  
  // default plane is YZ
  m_ActivePlane = YZ;
  
  //-----------------
  // pivot stuff
  //-----------------
  // pivotTransform is useless for this operation but required by isa generic
  m_PivotTransform = vtkTransform::New();

  // create pipeline stuff
  CreatePipeline();

  // create isa stuff
  CreateISA();

  //-----------------
  // create vme gizmos stuff
  //-----------------
  mafString vmeName;
  int i;
  for (i = 0; i < 3; i++)
  {
    // the ith gizmo
    m_Gizmo[i] = mafVMEGizmo::New();
    vmeName = "part";
    vmeName << i;
    m_Gizmo[i]->SetName(vmeName.GetCStr());
    m_Gizmo[i]->SetData(m_RotatePDF[i]->GetOutput());
	m_Gizmo[i]->SetMediator(m_Listener);
  }
  // assign isa to S1 and S2;
  m_Gizmo[S1]->SetBehavior(m_IsaComp[S1]);
  m_Gizmo[S2]->SetBehavior(m_IsaComp[S2]);

  mafMatrix *absInputMatrix = m_InputVme->GetOutput()->GetAbsMatrix();
  SetAbsPose(absInputMatrix);
  SetConstrainRefSys(absInputMatrix);

  // set come gizmo material property and initial color 
  this->SetColor(S1, 0, 1, 0);
  this->SetColor(S2, 0, 0, 1);
  this->SetColor(SQ, 1, 1, 0);
  
  // add the gizmo to the tree, this should increase reference count 
  for (i = 0; i < 3; i++)
  {
    m_Gizmo[i]->ReparentTo(mafVME::SafeDownCast(m_InputVme->GetRoot()));
  }
}
//----------------------------------------------------------------------------
medGizmoCrossTranslatePlane::~medGizmoCrossTranslatePlane() 
//----------------------------------------------------------------------------
{
  m_Gizmo[S1]->SetBehavior(NULL);
  m_Gizmo[S2]->SetBehavior(NULL);
  m_Gizmo[SQ]->SetBehavior(NULL);
  
  vtkDEL(m_Line[S1]);
  vtkDEL(m_Line[S2]);
  vtkDEL(m_Plane);
  vtkDEL(m_RotationTr);

  // clean up
  int i;
  for (i = 0; i < SQ; i++)
  {
    vtkDEL(m_LineTF[i]);
    vtkDEL(m_IsaComp[i]); 
  }

  m_PivotTransform->Delete();

  for (i = 0; i < 3; i++)
  {
    vtkDEL(m_RotatePDF[i]);
		m_Gizmo[i]->ReparentTo(NULL);
  }
}

//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::CreatePipeline() 
//----------------------------------------------------------------------------
{
  // calculate diagonal of InputVme space bounds 
  double b[6],p1[3],p2[3],d;
	if(m_InputVme->IsA("mafVMEGizmo"))
		m_InputVme->GetOutput()->GetVTKData()->GetBounds(b);
	else
		m_InputVme->GetOutput()->GetBounds(b);
  p1[0] = b[0];
  p1[1] = b[2];
  p1[2] = b[4];
  p2[0] = b[1];
  p2[1] = b[3];
  p2[2] = b[5];
  d = sqrt(vtkMath::Distance2BetweenPoints(p1,p2));

  /*
        z
        ^  S2
(0,0,1) |----- (0,1,1)
        |     |         
        | SQ  |S1          
        |     |         
        x--------> y    

            (0,1,0)
  
  */

  // create pipeline for cone-cylinder gizmo along global X axis
  
  // create S1
  m_Line[S1] = vtkLineSource::New();  
  m_Line[S1]->SetPoint1(0, 1, -1);
  m_Line[S1]->SetPoint2(0, 1, 1);

  // create S2
  m_Line[S2] = vtkLineSource::New();
  m_Line[S2]->SetPoint1(0, -1, 1);
  m_Line[S2]->SetPoint2(0, 1, 1);

  // create SQ
  m_Plane = vtkPlaneSource::New();
  m_Plane->SetOrigin(0, 0, 0);
  m_Plane->SetPoint1(0, 1, 0);
  m_Plane->SetPoint2(0, 0, 1);

  // create tube filter for the segments
  int i;
  for (i = 0; i < SQ; i++)
  {
    m_LineTF[i] = vtkTubeFilter::New();
    m_LineTF[i]->SetInput(m_Line[i]->GetOutput());
    m_LineTF[i]->SetRadius(d / 190);
    m_LineTF[i]->SetNumberOfSides(20);
  }

  //-----------------
  // update segments and square dimension based on vme bb diagonal
  //-----------------
  this->SetSizeLength(d / 4);

  //-----------------
  m_RotationTr = vtkTransform::New();
  m_RotationTr->Identity();

  // create rotation transform and rotation TPDF 
  for (i = 0; i < SQ; i++)
  {
    m_RotatePDF[i] = vtkTransformPolyDataFilter::New();
    m_RotatePDF[i]->SetTransform(m_RotationTr);
    m_RotatePDF[i]->SetInput(m_LineTF[i]->GetOutput());
  }
  m_RotatePDF[SQ] = vtkTransformPolyDataFilter::New();
  m_RotatePDF[SQ]->SetTransform(m_RotationTr);
  m_RotatePDF[SQ]->SetInput(m_Plane->GetOutput());
}

//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::CreateISA()
//----------------------------------------------------------------------------
{
  // Create isa compositor and assign behaviors to IsaGen ivar.
  // Default isa is constrained to plane XZ.
  for (int i = 0; i < SQ; i++)
  {
    m_IsaComp[i] = mafInteractorCompositorMouse::New();

    // default behavior is activated by mouse left and is constrained to X axis,
    // default ref sys is input vme abs matrix
    m_IsaGen[i] = m_IsaComp[i]->CreateBehavior(MOUSE_LEFT);
    m_IsaGen[i]->SetVME(m_InputVme);
    m_IsaGen[i]->GetTranslationConstraint()->SetConstraintModality(mafInteractorConstraint::FREE, mafInteractorConstraint::LOCK, mafInteractorConstraint::FREE);     
    
    //isa will send events to this
    m_IsaGen[i]->SetListener(this);
  }
}

//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::SetPlane(int plane) 
//----------------------------------------------------------------------------
{
  // this should be called when the translation gizmo
  // is created; gizmos are not highlighted
  
  // register the plane
  m_ActivePlane = plane;
  
  // rotate the gizmo components to match the specified plane
  if (m_ActivePlane == YZ)
  {
    // reset cyl and cone rotation
    m_RotationTr->Identity();
  
    // set S1 and S2 color
    this->SetColor(S1, 0, 1, 0);
    this->SetColor(S2, 0, 0, 1);

    // change the axis constrain
    for (int i = 0; i < 2; i++)
    {
      m_IsaGen[i]->GetTranslationConstraint()->SetConstraintModality(mafInteractorConstraint::LOCK, mafInteractorConstraint::FREE, mafInteractorConstraint::FREE);
    }
  }
  else if (m_ActivePlane == XZ)
  {
    // set rotation to move con and cyl on Y 
    m_RotationTr->Identity();
    m_RotationTr->RotateZ(-90);
   
    // set S1 and S2 color
    this->SetColor(S1, 1, 0, 0);
    this->SetColor(S2, 0, 0, 1);

    // change the axis constrain
    for (int i = 0; i < 2; i++)
    {
      m_IsaGen[i]->GetTranslationConstraint()->SetConstraintModality(mafInteractorConstraint::FREE, mafInteractorConstraint::LOCK, mafInteractorConstraint::FREE);
    }
  }  
  else if (m_ActivePlane == XY)
  {
    // set rotation to move con and cyl on Z
    m_RotationTr->Identity();
    m_RotationTr->RotateY(90);
    
    // set S1 and S2 color
    this->SetColor(S1, 0, 1, 0);
    this->SetColor(S2, 1, 0, 0);

     // change the axis constrain
    for (int i = 0; i < 2; i++)
    {
      m_IsaGen[i]->GetTranslationConstraint()->SetConstraintModality(mafInteractorConstraint::FREE, mafInteractorConstraint::FREE, mafInteractorConstraint::LOCK);
    }
  }  
}

//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::Highlight(bool highlight)
//----------------------------------------------------------------------------
{
  if (highlight == true)
  {
   // Highlight the S1 and S2  by setting its color to yellow 

   m_LastColor[0] = m_Color[0];
   m_LastColor[1] = m_Color[1];
   m_LastColor[2] = m_Color[2];

   this->SetColor(S1, 1, 1, 0);
   this->SetColor(S2, 1, 1, 0);

   // Show the square
   //ShowSquare(true);

  } 
  else
  {
   // restore original color 
   if (m_ActivePlane == YZ)
   {
    // set S1 and S2 color
    this->SetColor(S1, 0, 1, 0);
    this->SetColor(S2, 0, 0, 1);
   } 
   else if (m_ActivePlane == XZ)
   {
     // set S1 and S2 color
    this->SetColor(S1, 1, 0, 0);
    this->SetColor(S2, 0, 0, 1);
   }
   else if (m_ActivePlane == XY)
   {     
    // set S1 and S2 color
    this->SetColor(S1, m_LastColor);
    this->SetColor(S2, m_LastColor);
   } 

   // Hide the square
   ShowSquare(false);
  }
}

//----------------------------------------------------------------------------
void  medGizmoCrossTranslatePlane::SetSizeLength(double length)
//----------------------------------------------------------------------------
{
  /*
          z
          ^  S2
  (0,0,L) |----- (0,L,L)
          |     |         
          | SQ  |S1          
          |     |         
          x--------> y    

                (0,L,0)
  */

  // register the gizmo length
  m_Length = length;
  double L = length;
  // update S1
  m_Line[S1]->SetPoint1(0, 0, -L);
  m_Line[S1]->SetPoint2(0, 0, L);

  // update S2
  m_Line[S2]->SetPoint1(0, -L, 0);
  m_Line[S2]->SetPoint2(0, L, 0);

  // update SQ
  m_Plane->SetOrigin(0, 0, 0);
  m_Plane->SetPoint1(0, L, 0);
  m_Plane->SetPoint2(0, 0, L);
}
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    // mouse down change gizmo selection status
    if (e->GetId() == ID_TRANSFORM)
    {
      if (e->GetArg() == mafInteractorGenericMouse::MOUSE_DOWN)
      {
        this->SetIsActive(true);
      }
    }
    // forward events to the listener
    e->SetSender(this);
    mafEventMacro(*e);
  }
}
/** Gizmo color */
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::SetColor(int part, double col[3])
//----------------------------------------------------------------------------
{

  m_Color[0] = col[0];
  m_Color[1] = col[1];
  m_Color[2] = col[2];

  if (part == S1 || part == S2) //|| part == SQ)
  {
    m_Gizmo[part]->GetMaterial()->m_Prop->SetColor(col);
	  m_Gizmo[part]->GetMaterial()->m_Prop->SetAmbient(0);
	  m_Gizmo[part]->GetMaterial()->m_Prop->SetDiffuse(1);
	  m_Gizmo[part]->GetMaterial()->m_Prop->SetSpecular(0);
  }
}
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::SetColor(int part, double colR, double colG, double colB)
//----------------------------------------------------------------------------
{
  double col[3] = {colR, colG, colB};
  this->SetColor(part, col);
}
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::Show(bool show)
//----------------------------------------------------------------------------
{
  for (int i = 0; i < 2; i++)
		mafEventMacro(mafEvent(this,VME_SHOW,m_Gizmo[i],show));
}
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::ShowSquare(bool show)
//----------------------------------------------------------------------------
{
  double opacity = ((show == TRUE) ? 0.5 : 0);
  
  double invisibleOpacity = 0;
  m_Gizmo[SQ]->GetMaterial()->m_Prop->SetOpacity(invisibleOpacity);
}
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::SetAbsPose(mafMatrix *absPose)
//----------------------------------------------------------------------------
{
  for (int i = 0; i < 3; i++)
  {  
    m_Gizmo[i]->SetAbsMatrix(*absPose);
  }
  
  SetConstrainRefSys(absPose);
}
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::SetConstrainRefSys(mafMatrix *constrain)
//----------------------------------------------------------------------------
{  
  for (int i = 0; i < SQ; i++)
  {
    m_IsaGen[i]->GetTranslationConstraint()->GetRefSys()->SetTypeToCustom(constrain);
  }
}
//----------------------------------------------------------------------------
mafMatrix *medGizmoCrossTranslatePlane::GetAbsPose()
//----------------------------------------------------------------------------
{
  return m_Gizmo[S1]->GetOutput()->GetAbsMatrix();
}
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::SetInput(mafVME *vme)
//----------------------------------------------------------------------------
{
  this->m_InputVme = vme; 
  SetAbsPose(vme->GetOutput()->GetAbsMatrix()); 
  SetConstrainRefSys(vme->GetOutput()->GetAbsMatrix());
}
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::SetConstraintModality(int axis, int constrainModality)
//----------------------------------------------------------------------------
{
  for (int i = 0; i < SQ; i++)
  {
    m_IsaGen[i]->GetTranslationConstraint()->SetConstraintModality(axis,constrainModality);
  }
}
//----------------------------------------------------------------------------
void medGizmoCrossTranslatePlane::SetStep(int axis, double step)
//----------------------------------------------------------------------------
{
  for (int i = 0; i < SQ; i++)
  {
    m_IsaGen[i]->GetTranslationConstraint()->SetStep(axis,step);
  }
}
