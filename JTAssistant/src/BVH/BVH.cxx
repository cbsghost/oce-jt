// JT format reading and visualization tools
// Copyright (C) 2013-2015 OPEN CASCADE SAS
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

#include <BVH_Geometry.hxx>
#include <BVH_Triangulation.hxx>
#include <BVH_LinearBuilder.hxx>
#include <BVH_BinnedBuilder.hxx>
#include <BVH_SweepPlaneBuilder.hxx>
#include <BVH_SpatialMedianBuilder.hxx>

// Specific instantiations of struct templates to avoid compilation warnings

template class BVH_Box<Standard_Real, 2>;
template class BVH_Box<Standard_Real, 3>;
template class BVH_Box<Standard_Real, 4>;

template class BVH_Box<Standard_ShortReal, 2>;
template class BVH_Box<Standard_ShortReal, 3>;
template class BVH_Box<Standard_ShortReal, 4>;

template class BVH_Set<Standard_Real, 2>;
template class BVH_Set<Standard_Real, 3>;
template class BVH_Set<Standard_Real, 4>;

template class BVH_Set<Standard_ShortReal, 2>;
template class BVH_Set<Standard_ShortReal, 3>;
template class BVH_Set<Standard_ShortReal, 4>;

template class BVH_Object<Standard_Real, 2>;
template class BVH_Object<Standard_Real, 3>;
template class BVH_Object<Standard_Real, 4>;

template class BVH_Object<Standard_ShortReal, 2>;
template class BVH_Object<Standard_ShortReal, 3>;
template class BVH_Object<Standard_ShortReal, 4>;

template class BVH_ObjectSet<Standard_Real, 2>;
template class BVH_ObjectSet<Standard_Real, 3>;
template class BVH_ObjectSet<Standard_Real, 4>;

template class BVH_ObjectSet<Standard_ShortReal, 2>;
template class BVH_ObjectSet<Standard_ShortReal, 3>;
template class BVH_ObjectSet<Standard_ShortReal, 4>;

template class BVH_Geometry<Standard_Real, 2>;
template class BVH_Geometry<Standard_Real, 3>;
template class BVH_Geometry<Standard_Real, 4>;

template class BVH_Geometry<Standard_ShortReal, 2>;
template class BVH_Geometry<Standard_ShortReal, 3>;
template class BVH_Geometry<Standard_ShortReal, 4>;

template class BVH_Tree<Standard_Real, 2>;
template class BVH_Tree<Standard_Real, 3>;
template class BVH_Tree<Standard_Real, 4>;

template class BVH_Tree<Standard_ShortReal, 2>;
template class BVH_Tree<Standard_ShortReal, 3>;
template class BVH_Tree<Standard_ShortReal, 4>;

template class BVH_Builder<Standard_Real, 2>;
template class BVH_Builder<Standard_Real, 3>;
template class BVH_Builder<Standard_Real, 4>;

template class BVH_Builder<Standard_ShortReal, 2>;
template class BVH_Builder<Standard_ShortReal, 3>;
template class BVH_Builder<Standard_ShortReal, 4>;

template class BVH_BinnedBuilder<Standard_Real, 2>;
template class BVH_BinnedBuilder<Standard_Real, 3>;
template class BVH_BinnedBuilder<Standard_Real, 4>;

template class BVH_BinnedBuilder<Standard_ShortReal, 2>;
template class BVH_BinnedBuilder<Standard_ShortReal, 3>;
template class BVH_BinnedBuilder<Standard_ShortReal, 4>;

template class BVH_LinearBuilder<Standard_Real, 3>;
template class BVH_LinearBuilder<Standard_Real, 4>;

template class BVH_LinearBuilder<Standard_ShortReal, 3>;
template class BVH_LinearBuilder<Standard_ShortReal, 4>;

template class BVH_SweepPlaneBuilder<Standard_Real, 2>;
template class BVH_SweepPlaneBuilder<Standard_Real, 3>;
template class BVH_SweepPlaneBuilder<Standard_Real, 4>;

template class BVH_SweepPlaneBuilder<Standard_ShortReal, 2>;
template class BVH_SweepPlaneBuilder<Standard_ShortReal, 3>;
template class BVH_SweepPlaneBuilder<Standard_ShortReal, 4>;

template class BVH_SpatialMedianBuilder<Standard_Real, 2>;
template class BVH_SpatialMedianBuilder<Standard_Real, 3>;
template class BVH_SpatialMedianBuilder<Standard_Real, 4>;

template class BVH_SpatialMedianBuilder<Standard_ShortReal, 2>;
template class BVH_SpatialMedianBuilder<Standard_ShortReal, 3>;
template class BVH_SpatialMedianBuilder<Standard_ShortReal, 4>;

template class BVH_PrimitiveSet<Standard_Real, 2>;
template class BVH_PrimitiveSet<Standard_Real, 3>;
template class BVH_PrimitiveSet<Standard_Real, 4>;

template class BVH_PrimitiveSet<Standard_ShortReal, 2>;
template class BVH_PrimitiveSet<Standard_ShortReal, 3>;
template class BVH_PrimitiveSet<Standard_ShortReal, 4>;

template class BVH_Triangulation<Standard_Real, 2>;
template class BVH_Triangulation<Standard_Real, 3>;
template class BVH_Triangulation<Standard_Real, 4>;

template class BVH_Triangulation<Standard_ShortReal, 2>;
template class BVH_Triangulation<Standard_ShortReal, 3>;
template class BVH_Triangulation<Standard_ShortReal, 4>;

template class BVH_Transform<Standard_Real, 4>;
template class BVH_Transform<Standard_ShortReal, 4>;
