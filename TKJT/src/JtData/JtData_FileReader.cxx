// JT format reading and visualization tools
// Copyright (C) 2014-2015 OPEN CASCADE SAS
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2 of the License, or any later
// version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// Copy of the GNU General Public License is in LICENSE.txt and  
// on <http://www.gnu.org/licenses/>.

#include <JtData_FileReader.hxx>

//=======================================================================
//function : JtData_FileReader
//purpose  : Constructor
//=======================================================================

JtData_FileReader::JtData_FileReader (std::ifstream&                theFile,
                                      const Handle(JtData_Model)&   theModel,
                                      const std::ifstream::pos_type theOffset)
  : JtData_Reader (theModel)
  , myFile        (&theFile)
{
  if (theOffset != std::ifstream::pos_type (-1))
    myFile->seekg (theOffset);
}

//=======================================================================
//function : ReadBytes
//purpose  : Read raw bytes from the stream
//=======================================================================

Standard_Boolean JtData_FileReader::ReadBytes (void* theBuffer, Standard_Size theLength)
{
  myFile->read (reinterpret_cast<char*> (theBuffer), theLength);
  return !myFile->eof();
}

//=======================================================================
//function : SkipBytes
//purpose  : Skip some bytes
//=======================================================================

Standard_Boolean JtData_FileReader::SkipBytes (Standard_Size theLength)
{
  myFile->seekg (theLength, std::ios_base::cur);
  return !myFile->eof();
}

//=======================================================================
//function : GetPosition
//purpose  : Get absolute reading position
//=======================================================================

Standard_Size JtData_FileReader::GetPosition() const
{
  return static_cast <Standard_Size> (myFile->tellg());
}
