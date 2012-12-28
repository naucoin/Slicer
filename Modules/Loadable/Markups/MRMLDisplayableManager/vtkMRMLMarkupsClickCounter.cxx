
// MarkupsModule/MRMLDisplayableManager includes
#include "vtkMRMLMarkupsClickCounter.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLMarkupsClickCounter);
vtkCxxRevisionMacro (vtkMRMLMarkupsClickCounter, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
void vtkMRMLMarkupsClickCounter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsClickCounter::vtkMRMLMarkupsClickCounter()
{
  this->m_Clicks = 0;
}

//---------------------------------------------------------------------------
vtkMRMLMarkupsClickCounter::~vtkMRMLMarkupsClickCounter()
{
  // TODO Auto-generated destructor stub
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsClickCounter::Reset()
{
  this->m_Clicks = 0;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsClickCounter::HasEnoughClicks(int clicks)
{
  this->m_Clicks++;

  if (this->m_Clicks==clicks)
    {
      this->Reset();
      return true;
    }

  return false;
}

