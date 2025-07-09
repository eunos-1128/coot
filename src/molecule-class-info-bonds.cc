/* src/molecule-class-info-bonds.cc
 *
 * Copyright 2016 by Medical Research Council
 * Author: Paul Emsley
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License and
 * the GNU Lesser General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA, 02110-1301, USA.
 */

#include "coot-utils/atom-selection-container.hh"
#include "molecule-class-info.h"

#include "graphics-info.h" // for attach_buffers()


void
molecule_class_info_t::set_user_defined_colour_indices_by_selections(const std::vector<std::pair<std::string, unsigned int> > &cis) {

   // two functions doing the same thing
   bool apply_colour_to_non_carbon_atoms_also = true;
   set_user_defined_atom_colour_by_selection(cis, apply_colour_to_non_carbon_atoms_also);
}


void
molecule_class_info_t::set_user_defined_colour_indices(const std::vector<std::pair<coot::atom_spec_t, int> > &cis) {

   if (atom_sel.mol) {
      int udd_handle = atom_sel.mol->GetUDDHandle(mmdb::UDR_ATOM, "user-defined-atom-colour-index");

      if (udd_handle == 0)
	 udd_handle = atom_sel.mol->RegisterUDInteger(mmdb::UDR_ATOM, "user-defined-atom-colour-index");

      for (unsigned int i=0; i<cis.size(); i++) {
	 const coot::atom_spec_t &spec = cis[i].first;
	 mmdb::Atom *at = get_atom(spec);
	 if (at) {
	    int ierr = at->PutUDData(udd_handle, cis[i].second);
	    if (ierr != mmdb::UDDATA_Ok) {
	       std::cout << "ERROR:: problem setting udd on atom " << coot::atom_spec_t(at) << std::endl;
	    }
	 } else {
            std::cout << "WARNING:: in set_user_defined_colour_indices() failed to get atom "
                      << spec << std::endl;
         }
      }
   }
}

void
molecule_class_info_t::user_defined_colours_representation(coot::protein_geometry *geom_p,
                                                           bool all_atoms_mode,
                                                           bool draw_missing_loops_flag) {

   bonds_box.clear_up();
   if (all_atoms_mode) {

      Bond_lines_container bonds(atom_sel, imol_no, geom_p, Bond_lines_container::COLOUR_BY_USER_DEFINED_COLOURS);
      bonds_box = bonds.make_graphical_bonds_no_thinning();
      bonds_box_type = coot::COLOUR_BY_USER_DEFINED_COLOURS____BONDS;

   } else {

      // CA mode then

      // std::cout << "::::::::::::::::::::::::::::: in user_defined_colours_representation() Path B " << std::endl;
      std::set<int> no_bonds_to_these_atom_indices;
      Bond_lines_container bonds(geom_p, "dummy-CA-mode", no_bonds_to_these_atom_indices, false);
      bonds.do_Ca_plus_ligands_bonds(atom_sel, imol_no, geom_p, 2.4, 4.7,
                                     draw_missing_loops_flag,
                                     coot::COLOUR_BY_USER_DEFINED_COLOURS, false);
      bool add_residue_indices = true;
      bonds_box = bonds.make_graphical_bonds_no_thinning();
      bonds_box_type = coot::COLOUR_BY_USER_DEFINED_COLOURS_CA_BONDS;
   }
   graphics_info_t::attach_buffers();
   // make_mesh_from_bonds_box();
   make_meshes_from_bonds_box_instanced_version();

}


void
molecule_class_info_t::add_to_non_drawn_bonds(const std::string &atom_selection_cid) {

   // I have used an *atom* selection here, but see notes in add_to_non_drawn_bonds()
   // in coot-molecule-bonds.cc
  //
   if (atom_sel.mol) {
      int atom_index_udd_handle = atom_sel.UDDAtomIndexHandle;
      int handle = atom_sel.mol->NewSelection();
      int n_atoms = 0;
      mmdb::Atom **atom_selection = nullptr;
      std::vector<std::string> v = coot::util::split_string(atom_selection_cid, "||");
      if (! v.empty()) {
         for (const auto &cid : v) {
            atom_sel.mol->Select(handle, mmdb::STYPE_ATOM, cid.c_str(), mmdb::SKEY_NEW);
            atom_sel.mol->GetSelIndex(handle, atom_selection, n_atoms);
            if (n_atoms > 0) {
               for (int i = 0; i < n_atoms; i++) {
                  mmdb::Atom *at = atom_selection[i];
                  int idx;
                  at->GetUDData(atom_index_udd_handle, idx);
                  no_bonds_to_these_atom_indices.insert(idx);
               }
            }
         }
      }
      // we are passing a class variable to this function - that's not a great idea
      make_bonds_type_checked(no_bonds_to_these_atom_indices, __FUNCTION__);
   }
}


// this clears the old no_bonds_to_these_atom_indices set and replaces it with a new one - and regens bonds.
void
molecule_class_info_t::set_new_non_drawn_bonds(const std::string &cid) {

   no_bonds_to_these_atom_indices.clear();
   add_to_non_drawn_bonds(cid);
}

void
molecule_class_info_t::clear_non_drawn_bonds(bool regen_bonds) {

  no_bonds_to_these_atom_indices.clear();
  if (regen_bonds)
     make_bonds_type_checked(no_bonds_to_these_atom_indices, __FUNCTION__);

}

void
molecule_class_info_t::clear_user_defined_atom_colours() {

   if (atom_sel.mol) {
      int udd_handle = atom_sel.mol->GetUDDHandle(mmdb::UDR_ATOM, "user-defined-atom-colour-index");
      if (udd_handle != 0) {
	       udd_handle = 0; // reset
      }
   }
}

// This is used in getting atom specs from a bonds box. Not a sensible thing to do
//
// instead the bonds box should contain atom specs
//
mmdb::Atom *
molecule_class_info_t::get_atom_at_pos(const coot::Cartesian &pt) const {

   mmdb::Atom *at = NULL;

   int imod = 1;
   mmdb::Model *model_p = atom_sel.mol->GetModel(imod);
   if (model_p) {
      int n_chains = model_p->GetNumberOfChains();
      for (int ichain=0; ichain<n_chains; ichain++) {
	 mmdb::Chain *chain_p = model_p->GetChain(ichain);
	 int nres = chain_p->GetNumberOfResidues();
	 for (int ires=0; ires<nres; ires++) {
	    mmdb::Residue *residue_p = chain_p->GetResidue(ires);
	    int n_atoms = residue_p->GetNumberOfAtoms();
	    for (int iat=0; iat<n_atoms; iat++) {
	       mmdb::Atom *atl = residue_p->GetAtom(iat);
	       coot::Cartesian at_pos(atl->x, atl->y, atl->z);
	       at_pos -= pt;
	       float d = at_pos.amplitude_squared();
	       if (d < 0.001) {
		  at = atl;
		  break;
	       }
	    }
	    if (at) break;
	 }
	 if (at) break;
      }
   }

   return at;


}


void
molecule_class_info_t::alt_conf_view_next_alt_conf(const std::string &current_alt_conf) {

   std::vector<std::string> alt_confs;
   int n = alt_confs.size();
   if (n > 1) {
      int idx_this = -1; // unset
      int next_index = 0;
      for (int i=0; i<n; i++) {
         const std::string a = alt_confs[i];
         if (current_alt_conf == a) {
            idx_this = i;
         }
      }
      if (idx_this != -1) {
         next_index = idx_this + 1;
         if (next_index == n)
            next_index = 0;
      }
      std::string alt_conf = alt_confs[next_index];
      std::string atom_selection = "//";
      if (! alt_conf.empty()) {
         atom_selection = "//*/*/*:" + alt_conf;
      }

      mmdb::Manager *mol = atom_sel.mol;
      int SelectionHandle = mol->NewSelection(); // d
      mol->Select(SelectionHandle, mmdb::STYPE_ATOM, atom_selection.c_str(), mmdb::SKEY_NEW);
      mmdb::Manager *new_mol = coot::util::create_mmdbmanager_from_atom_selection(mol, SelectionHandle); // deleted?

      atom_selection_container_t asc_selection = make_asc(new_mol);
      bool draw_hydrogens_flag = true;
      bool draw_missing_loops_flag = true;
      bool do_goodsell_colour_mode = false;
      bool do_rama_markup = false;
      std::set<int> no_bonds_to_these_atoms;
      bool include_disulphides = 1;
      coot::protein_geometry *geom_in = graphics_info_t::Geom_p();
      int model_number = 0;
      Bond_lines_container blc(asc_selection, imol_no, no_bonds_to_these_atoms, geom_in, include_disulphides, draw_hydrogens_flag,
                               draw_missing_loops_flag, model_number, "", false, false, false, nullptr);

      mol->DeleteSelection(SelectionHandle);
   }
}
