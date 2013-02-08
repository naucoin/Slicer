/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

// Colors
#include "vtkSlicerScalarBarActor.h"

// VTK includes
#include "vtkObjectFactory.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkScalarsToColors.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkViewport.h"
#include "vtkLookupTable.h"

vtkCxxRevisionMacro(vtkSlicerScalarBarActor, "$Revision$");
vtkStandardNewMacro(vtkSlicerScalarBarActor);

//---------------------------------------------------------------------------
vtkSlicerScalarBarActor::vtkSlicerScalarBarActor()
{
  this->Names.clear();
  this->UseColorNameAsLabel = 0;
}

//----------------------------------------------------------------------------
vtkSlicerScalarBarActor::~vtkSlicerScalarBarActor()
{
  this->Names.clear();
}

//----------------------------------------------------------------------------
void vtkSlicerScalarBarActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "UseColorNameAsLabel: " << this->UseColorNameAsLabel << "\n";
}

//---------------------------------------------------------------------------
int vtkSlicerScalarBarActor::SetColorName(int ind, const char *name)
{
  if (!this->LookupTable)
    {
    vtkWarningMacro(<<"Need a lookup table to render a scalar bar");
    return 0;
    }
  vtkLookupTable* lookupTable = vtkLookupTable::SafeDownCast(this->LookupTable);
  if (lookupTable)
    {
    if (lookupTable->GetNumberOfColors() != this->Names.size())
      {
      this->Names.resize(lookupTable->GetNumberOfColors());
      }
    
    std::string newName(name);
    if (this->Names[ind] != newName)
      {
      this->Names[ind] = newName;
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkSlicerScalarBarActor::AllocateAndSizeLabels(int *labelSize,
                                                    int *size,
                                                    vtkViewport *viewport,
                                                    double *range)
{
  labelSize[0] = labelSize[1] = 0;

  if (this->GetUseColorNameAsLabel() == 1)
    {
    // if number of labels are too many, the label will be too small to see 
    // change number to 64
    if (this->NumberOfLabels > 64)
      {
      this->NumberOfLabels = 64;
      }
    // if number of labels is larger than actual number of color names 
    if (this->NumberOfLabels > this->Names.size())
      {
      this->NumberOfLabels = this->Names.size();
      }
    }

  this->TextMappers = new vtkTextMapper * [this->NumberOfLabels];
  this->TextActors = new vtkActor2D * [this->NumberOfLabels];

  char string[512];

  double val;
  int i;

  // TODO: this should be optimized, maybe by keeping a list of
  // allocated mappers, in order to avoid creation/destruction of
  // their underlying text properties (i.e. each time a mapper is
  // created, text properties are created and shallow-assigned a font size
  // which value might be "far" from the target font size).

  // is this a vtkLookupTable or a subclass of vtkLookupTable
  // with its scale set to log
  int isLogTable = this->LookupTable->UsingLogScale();

  for (i=0; i < this->NumberOfLabels; i++)
    {
    this->TextMappers[i] = vtkTextMapper::New();

    if ( isLogTable )
      {
      double lval;
      if (this->NumberOfLabels > 1)
        {
        lval = log10(range[0]) +
          static_cast<double>(i)/(this->NumberOfLabels-1) *
          (log10(range[1])-log10(range[0]));
        }
      else
        {
        lval = log10(range[0]) + 0.5*(log10(range[1])-log10(range[0]));
        }
      val = pow(10.0,lval);
      }
    else
      {
      if (this->NumberOfLabels > 1)
        {
        val = range[0] +
          static_cast<double>(i)/(this->NumberOfLabels-1)
          * (range[1]-range[0]);
        }
      else
        {
        val = range[0] + 0.5*(range[1]-range[0]);
        }
      }
    //
    if (this->GetUseColorNameAsLabel() == 1)
      {
      double indx = 0.0;
      int index  = 0;
      if (this->NumberOfLabels > 1)
        {
        indx = static_cast<double>(i)/(this->NumberOfLabels-1)*(this->Names.size()-1);
        }
      else
        {
        indx = 0.5*this->Names.size();
        }
      index = static_cast<int>(indx+0.5);
      sprintf(string, this->LabelFormat, this->Names[index].c_str());
      }
    else
      {
      sprintf(string, this->LabelFormat, val);
      }
    this->TextMappers[i]->SetInput(string);

    // Shallow copy here so that the size of the label prop is not affected
    // by the automatic adjustment of its text mapper's size (i.e. its
    // mapper's text property is identical except for the font size
    // which will be modified later). This allows text actors to
    // share the same text property, and in that case specifically allows
    // the title and label text prop to be the same.
    this->TextMappers[i]->GetTextProperty()->ShallowCopy(
      this->LabelTextProperty);

    this->TextActors[i] = vtkActor2D::New();
    this->TextActors[i]->SetMapper(this->TextMappers[i]);
    this->TextActors[i]->SetProperty(this->GetProperty());
    this->TextActors[i]->GetPositionCoordinate()->
      SetReferenceCoordinate(this->PositionCoordinate);
    }

  if (this->NumberOfLabels)
    {
    int targetWidth, targetHeight;

    if ( this->Orientation == VTK_ORIENT_VERTICAL )
      {
      targetWidth = static_cast<int>(0.6*size[0]);
      targetHeight = static_cast<int>(0.86*size[1]/this->NumberOfLabels);
      }
    else
      {
      targetWidth = static_cast<int>(size[0]*0.8/this->NumberOfLabels);
      targetHeight = static_cast<int>(0.25*size[1]);
      }

    vtkTextMapper::SetMultipleConstrainedFontSize(viewport,
                                                  targetWidth,
                                                  targetHeight,
                                                  this->TextMappers,
                                                  this->NumberOfLabels,
                                                  labelSize);
    }
}
