/*
 *
 * Copyright 2024 The RMG Project Developers. See the COPYRIGHT file 
 * at the top-level directory of this distribution or in the current
 * directory.
 * 
 * This file is part of RMG. 
 * RMG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * any later version.
 *
 * RMG is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/foreach.hpp>

#if !(defined(_WIN32) || defined(_WIN64))
    #include <unistd.h>
#else
    #include <io.h>
#endif
#include <complex>
#include "const.h"
#include "params.h"
#include "rmgtypedefs.h"
#include "typedefs.h"
#include "rmg_error.h"
#include "State.h"
#include "Kpoint.h"
#include "transition.h"


/*
   This function writes an xml file that closely follows the format of the
   vasprun.xml files. The format is not exactly the same but is intended to
   make it easy for an external parser to generate classical/ML potentials
   using the DFT results generated by RMG.
*/

namespace pt = boost::property_tree;

/* Writes a single easy to parse file for force field fitting codes */
void write_ffield (std::string &filename)
{
    // Create an empty property tree object.
    pt::ptree tree;
    pt::ptree modeling;
    pt::ptree structure;
    pt::ptree atominfo;
    pt::ptree atoms, atom_field1, atom_field2;
    pt::ptree crystal;
    pt::ptree basis;
    pt::ptree rec_basis;
    pt::ptree positions;
    pt::ptree forces;

//    tree.put("energy.<xmlattr>.units","Ha");
//    tree.put("energy.total",ct.TOTAL);

   modeling.put("modeling","");
   atom_field1.add("<xmlattr>.type", "string");
   atom_field1.put_value("element");
   atom_field2.add("<xmlattr>.type", "int");
   atom_field2.put_value("atomtype");
   atoms.add_child("field", atom_field1);
   atoms.add_child("field", atom_field2);

   atominfo.put("atoms", Atoms.size());
   atominfo.put("types", Species.size());
   atominfo.add_child("array", atoms);
   atominfo.put("array.<xmlattr>.name", "atoms");
   atominfo.put("array.dimension", "ion");
   atominfo.put("array.dimension.<xmlattr>.dim", "1");
   atominfo.put("array.set", "");
   for(auto Atom : Atoms)
   {
       pt::ptree rc, cols1, cols2;
       cols1.put_value(std::string(Atom.Type->atomic_symbol));
       cols2.put_value(std::to_string(Atom.Type->index + 1));
       rc.add_child("c", cols1);
       rc.add_child("c", cols2);
       atominfo.add_child("array.set.rc", rc);
   }

   modeling.add_child("atominfo", atominfo);

   structure.put("<xmlattr>.name","finalpos");
   basis.put("<xmlattr>.name","basis");
       std::string a1 = std::to_string(Rmg_L.a0[0]) + "  " + 
                     std::to_string(Rmg_L.a0[1]) + "  " +
                     std::to_string(Rmg_L.a0[2]) + " ";
       basis.add("v",a1);
       std::string a2 = std::to_string(Rmg_L.a1[0]) + "  " + 
                     std::to_string(Rmg_L.a1[1]) + "  " +
                     std::to_string(Rmg_L.a1[2]) + " ";
       basis.add("v",a2);
       std::string a3 = std::to_string(Rmg_L.a2[0]) + "  " + 
                     std::to_string(Rmg_L.a2[1]) + "  " +
                     std::to_string(Rmg_L.a2[2]) + " ";
       basis.add("v",a3);
   crystal.add_child("varray", basis);
   rec_basis.put("<xmlattr>.name","rec_basis");
       std::string b1 = std::to_string(Rmg_L.b0[0]) + "      " + 
                     std::to_string(Rmg_L.b0[1]) + "      " +
                     std::to_string(Rmg_L.b0[2]) + " ";
       rec_basis.add("v",b1);
       std::string b2 = std::to_string(Rmg_L.b1[0]) + "      " + 
                     std::to_string(Rmg_L.b1[1]) + "      " +
                     std::to_string(Rmg_L.b1[2]) + " ";
       rec_basis.add("v",b2);
       std::string b3 = std::to_string(Rmg_L.b2[0]) + "      " + 
                     std::to_string(Rmg_L.b2[1]) + "      " +
                     std::to_string(Rmg_L.b2[2]) + " ";
       rec_basis.add("v",b3);
   crystal.add_child("varray", rec_basis);

   positions.put("<xmlattr>.name","positions");
   forces.put("<xmlattr>.name","forces");
   for(auto Atom : Atoms)
   {
       std::string p = "      " + 
                       std::to_string(Atom.xtal[0]) + "      " +
                       std::to_string(Atom.xtal[1]) + "      " +
                       std::to_string(Atom.xtal[2]) + " ";
       positions.add("v", p);
       std::string f = "      " + 
                       std::to_string(Atom.force[0][0]) + "      " +
                       std::to_string(Atom.force[0][1]) + "      " +
                       std::to_string(Atom.force[0][2]) + " ";
       forces.add("v", f);
   }

   structure.add_child("crystal", crystal);
   structure.add_child("varray", positions);
   structure.add_child("varray", forces);
   modeling.add_child("structure", structure);
   

    // Write the xml file
    pt::xml_writer_settings<std::string> settings('\t', 1);
    pt::write_xml( filename,
                modeling,  
                std::locale(),
                pt::xml_writer_make_settings< std::string >( ' ', 1) );
}
