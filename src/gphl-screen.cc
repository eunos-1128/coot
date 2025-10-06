
#include "coot-utils/coot-coord-utils.hh"
#include "cc-interface.hh"
#include "positioned-widgets.h"
#include "c-interface-gtk-widgets.h" // for set_transient_and_position()
#include "c-interface.h"
#include "graphics-info.h"

// dialog callbacks
extern "C" G_MODULE_EXPORT
void on_gphl_screen_close_button_clicked(GtkButton       *button,
                                         gpointer         user_data) {

   std::cout << "close " << button << std::endl;
   GtkWidget *dialog = GTK_WIDGET(g_object_get_data(G_OBJECT(button), "dialog"));
   if (dialog)
      gtk_widget_set_visible(dialog, FALSE);
   else
      std::cout << "boooo null dialog " << std::endl;
}

void go_to_gphl_atoms(int imol, const std::string &atom_ids, const std::string &type) {

   bool debug = false;

   auto gphl_spec_to_coot_spec = [] (const std::string &atom_spec_gphl) {

      coot::atom_spec_t atom_spec;
      std::vector<std::string> parts_3 = coot::util::split_string(atom_spec_gphl, "|");
      if (parts_3.size() == 2) {
         std::string chain_id = parts_3[0];
         std::string rest     = parts_3[1];
         std::vector<std::string> parts_4 = coot::util::split_string(rest, ":");
         if (parts_4.size() == 2) {
            try {
               std::string res_no_str = parts_4[0];
               int res_no = coot::util::string_to_int(res_no_str);
               // now strip out "(PHE)"
               std::vector<std::string> parts_5 = coot::util::split_string(parts_4[1], "(");
               if (parts_5.size() > 0) {
                  std::string ins_code;
                  std::string alt_conf;
                  std::string atom_name_raw = parts_5[0];
                  std::string atom_name = atom_name_raw; // unpadded
                  atom_spec = coot::atom_spec_t(chain_id, res_no, ins_code, atom_name, alt_conf);
               }
            } catch (const std::runtime_error &rte) {
               std::cout << "WARNING::" << rte.what() << std::endl;
            }
         }
      }
      return atom_spec;
   };

   // ------------------ main line --------------------

   if (debug)
      std::cout << "debug:: go_to_gphl_atoms(): decode this: " << atom_ids
                << " type: " << type << std::endl;

   mmdb::Manager *mol = nullptr;
   if (is_valid_model_molecule(imol))
      mol = graphics_info_t::molecules[imol].atom_sel.mol;
   else
      return;

   if (type == "unhappy-atom") {
      std::vector<std::string> parts_1 = coot::util::split_string(atom_ids, "|");
      std::cout << "debug:: " << parts_1.size() << std::endl;
      if (parts_1.size() == 2) {
         std::string chain_id = parts_1[0];
         std::string rest_1   = parts_1[1];
         std::cout << "debug:: chain_id: " << chain_id << " rest_1: " << rest_1 << std::endl;
         std::vector<std::string> parts_2 = coot::util::split_string(rest_1, ":");
         if (parts_2.size() == 2) {
            std::string res_no_str = parts_2[0];
            std::string rest_2     = parts_2[1];
            std::cout << "debug:: res_no_str: " << res_no_str << " rest_2: " << rest_2 << std::endl;
            std::vector<std::string> parts_3 = coot::util::split_string(rest_2, "(");
            if (parts_3.size() == 2) {
               std::string atom_name = parts_3[0];
               std::string rest_3    = parts_3[1];
               if (! atom_name.empty()) {
                  try {
                     int res_no = coot::util::string_to_int(res_no_str);
                     set_go_to_atom_molecule(imol);
                     set_go_to_atom_chain_residue_atom_name(chain_id.c_str(), res_no, atom_name.c_str());
                  }
                  catch (const std::runtime_error &e) {
                     std::cout << "WARNING::" << e.what() << std::endl;
                  }
               }
            }
         }
      }
   }
   if (type == "bond") {
      // e.g.: "A|502:O5'=C5'"
      std::vector<std::string> parts_1 = coot::util::split_string(atom_ids, "|");
      std::cout << "debug:: " << parts_1.size() << std::endl;
      if (parts_1.size() == 2) {
         std::string chain_id = parts_1[0];
         std::string rest_1   = parts_1[1];
         if (debug)
            std::cout << "debug:: bond_block: chain_id: " << chain_id << " rest_1: " << rest_1 << std::endl;
         std::vector<std::string> parts_2 = coot::util::split_string(rest_1, ":");
         if (parts_2.size() == 2) {
            std::string res_no_str = parts_2[0];
            std::string rest_2     = parts_2[1];
            if (debug)
               std::cout << "debug:: bond_block: res_no_str: " << res_no_str << " rest_2: " << rest_2 << std::endl;
            std::vector<std::string> parts_3 = coot::util::split_string(rest_2, "=");
            if (parts_3.size() == 2) {
               std::string atom_name_1_raw = parts_3[0];
               std::string rest_3      = parts_3[1];
               std::string atom_name_2_raw = rest_3;
               std::string atom_name_1 = atom_name_1_raw;
               std::string atom_name_2 = atom_name_2_raw;
               if (debug)
                  std::cout << "debug:: bond_block: atom_name_1 " << atom_name_1
                            << " atom_name_2 " << atom_name_2 << std::endl;
               if (! atom_name_1.empty()) {
                  if (! atom_name_2.empty()) {
                     try {
                        int res_no = coot::util::string_to_int(res_no_str);
                        // it will be a pain to decode these
                        std::string ins_code;
                        std::string alt_conf;
                        coot::atom_spec_t atom_spec_1(chain_id, res_no, ins_code, atom_name_1, alt_conf);
                        coot::atom_spec_t atom_spec_2(chain_id, res_no, ins_code, atom_name_2, alt_conf);
                        mmdb::Atom *at_1 = coot::util::get_atom_using_fuzzy_search(atom_spec_1, mol);
                        mmdb::Atom *at_2 = coot::util::get_atom_using_fuzzy_search(atom_spec_2, mol);
                        if (at_1 && at_2) {
                           if (debug)
                              std::cout << "debug:: bond block - found both" << std::endl;
                           clipper::Coord_orth pt_1(at_1->x, at_1->y, at_1->z);
                           clipper::Coord_orth pt_2(at_2->x, at_2->y, at_2->z);
                           clipper::Coord_orth mid(0.5 * (pt_1 + pt_2));
                           set_rotation_centre(mid.x(), mid.y(), mid.z());
                        } else {
                           std::cout << "WARNING:: failed to lookup an atom " << atom_spec_1 << " " << atom_spec_2
                                     << std::endl;
                        }
                     }
                     catch (const std::runtime_error &e) {
                        std::cout << "WARNING::" << e.what() << std::endl;
                     }
                  } else  {
                     std::cout << "WARNING:: blank atom_name_2" << std::endl;
                  }
               } else {
                  std::cout << "WARNING:: blank atom_name_1" << std::endl;
               }
            }
         }
      }
   }

   if (type == "angle") {
      // two forms: e.g.
      // 124.058     126.800       2.742       0.700       3.917    A|502:C5=C4=N3 (ANP)
      // 125.172     119.300      -5.872       1.500      -3.914    A|231:C(ALA)=A|232:N(PRO)=A|232:CA(PRO)
      std::vector<std::string> parts_1 = coot::util::split_string(atom_ids, "|");
      std::cout << "debug:: " << parts_1.size() << std::endl;
      if (parts_1.size() == 2) {
         // simple case = all atoms in the same residue
         std::string chain_id = parts_1[0];
         std::string rest_1   = parts_1[1];
         if (debug)
            std::cout << "debug:: angle_block: chain_id: " << chain_id << " rest_1: " << rest_1 << std::endl;
         std::vector<std::string> parts_2 = coot::util::split_string(rest_1, ":");
         if (parts_2.size() == 2) {
            std::string res_no_str = parts_2[0];
            std::string rest_2     = parts_2[1];
            if (debug)
               std::cout << "debug:: angle_block: res_no_str: " << res_no_str << " rest_2: " << rest_2 << std::endl;
            std::vector<std::string> parts_3 = coot::util::split_string(rest_2, "=");
            if (parts_3.size() == 3) {
               std::string atom_name_1_raw = parts_3[0];
               std::string rest_3          = parts_3[1];
               std::string atom_name_2_raw = rest_3;
               std::string atom_name_3_raw = parts_3[2];
               // instead of padding here, it would be better to use get_atom_using_fuzzy_search()
               // so that the atom name can be passed without needing to pad it.
               std::string atom_name_1 = atom_name_1_raw;
               std::string atom_name_2 = atom_name_2_raw;
               std::string atom_name_3 = atom_name_3_raw;
               if (debug)
                  std::cout << "debug:: angle_block:"
                            << " atom_name_1 " << atom_name_1
                            << " atom_name_2 " << atom_name_2
                            << " atom_name_3 " << atom_name_3 << std::endl;
               if (! atom_name_1.empty()) {
                  if (! atom_name_2.empty()) {
                     if (! atom_name_3.empty()) {
                        try {
                           int res_no = coot::util::string_to_int(res_no_str);
                           std::string ins_code;
                           std::string alt_conf;
                           coot::atom_spec_t atom_spec_1(chain_id, res_no, ins_code, atom_name_1, alt_conf);
                           coot::atom_spec_t atom_spec_2(chain_id, res_no, ins_code, atom_name_2, alt_conf);
                           coot::atom_spec_t atom_spec_3(chain_id, res_no, ins_code, atom_name_3, alt_conf);
                           mmdb::Atom *at_1 = coot::util::get_atom_using_fuzzy_search(atom_spec_1, mol);
                           mmdb::Atom *at_2 = coot::util::get_atom_using_fuzzy_search(atom_spec_2, mol);
                           mmdb::Atom *at_3 = coot::util::get_atom_using_fuzzy_search(atom_spec_3, mol);
                           if (at_1 && at_2 && at_3) {
                              clipper::Coord_orth pt_1(at_1->x, at_1->y, at_1->z);
                              clipper::Coord_orth pt_2(at_2->x, at_2->y, at_2->z);
                              clipper::Coord_orth pt_3(at_3->x, at_3->y, at_3->z);
                              clipper::Coord_orth mid(0.333333 * (pt_1 + pt_2 + pt_3));
                              set_rotation_centre(mid.x(), mid.y(), mid.z());
                           } else {
                              std::cout << "WARNING:: failed to lookup an atom " << atom_spec_1
                                        << " " << atom_spec_2 << " " << atom_spec_3 << std::endl;
                           }
                        } catch (const std::runtime_error &e) {
                           std::cout << "WARNING::" << e.what() << std::endl;
                        }
                     }
                  }
               }
            }
         }
      }
      // A|231:C(ALA)=A|232:N(PRO)=A|232:CA(PRO)
      if (parts_1.size() == 4) {
         std::vector<std::string> parts_2 = coot::util::split_string(atom_ids, "=");
         // A|231:C(ALA) A|232:N(PRO) A|232:CA(PRO)
         if (parts_2.size() == 3) {
            std::string atom_1 = parts_2[0];
            std::string atom_2 = parts_2[1];
            std::string atom_3 = parts_2[2];
            // for each of the atoms
            coot::atom_spec_t atom_spec_1 = gphl_spec_to_coot_spec(atom_1);
            coot::atom_spec_t atom_spec_2 = gphl_spec_to_coot_spec(atom_2);
            coot::atom_spec_t atom_spec_3 = gphl_spec_to_coot_spec(atom_3);
            if (! atom_spec_1.empty() && ! atom_spec_2.empty() && ! atom_spec_3.empty()) {
               mmdb::Atom *at_1 = coot::util::get_atom_using_fuzzy_search(atom_spec_1, mol);
               mmdb::Atom *at_2 = coot::util::get_atom_using_fuzzy_search(atom_spec_2, mol);
               mmdb::Atom *at_3 = coot::util::get_atom_using_fuzzy_search(atom_spec_3, mol);
               if (at_1 && at_2 && at_3) {
                  clipper::Coord_orth pt_1(at_1->x, at_1->y, at_1->z);
                  clipper::Coord_orth pt_2(at_2->x, at_2->y, at_2->z);
                  clipper::Coord_orth pt_3(at_3->x, at_3->y, at_3->z);
                  clipper::Coord_orth mid(0.333333 * (pt_1 + pt_2 + pt_3));
                  set_rotation_centre(mid.x(), mid.y(), mid.z());
               } else {
                  std::cout << "WARNING:: Failed to find an atom from specs: "
                            << atom_spec_1 << " " << atom_spec_2 << " " << atom_spec_3 << std::endl;
               }
            } else {
               std::cout << "WARNING:: a spec was empty " << atom_spec_1 << " " << atom_spec_2 << " " << atom_spec_3
                         << std::endl;
            }
         }
      }
   }

   if (type == "torsion") {
      // 119.206      60.000     -59.206      15.000      -3.947    A|330:N=CA=CB=OG (SER)
      std::vector<std::string> parts_1 = coot::util::split_string(atom_ids, "|");
      std::cout << "debug:: " << parts_1.size() << std::endl;
      if (parts_1.size() == 2) {
         // simple case = all atoms in the same residue
         std::string chain_id = parts_1[0];
         std::string rest_1   = parts_1[1];
         if (debug)
            std::cout << "debug:: torsion_block: chain_id: " << chain_id << " rest_1: " << rest_1 << std::endl;
         std::vector<std::string> parts_2 = coot::util::split_string(rest_1, ":");
         if (parts_2.size() == 2) {
            std::string res_no_str = parts_2[0];
            std::string rest_2     = parts_2[1];
            if (debug)
               std::cout << "debug:: torsion_block: res_no_str: " << res_no_str << " rest_2: " << rest_2 << std::endl;
            std::vector<std::string> parts_3 = coot::util::split_string(rest_2, "=");
            if (parts_3.size() == 4) {
               std::string atom_name_1_raw = parts_3[0];
               std::string atom_name_2_raw = parts_3[1];
               std::string atom_name_3_raw = parts_3[2];
               std::string atom_name_4_raw = parts_3[3];
               if (debug)
                  std::cout << "debug:: torsion_block: " << atom_name_1_raw << " " << atom_name_2_raw
                            << " " << atom_name_3_raw << " " << atom_name_4_raw
                            << std::endl;
               if (! atom_name_1_raw.empty()) {
                  if (! atom_name_2_raw.empty()) {
                     if (! atom_name_3_raw.empty()) {
                        if (! atom_name_4_raw.empty()) {
                           try {
                              int res_no = coot::util::string_to_int(res_no_str);
                              std::string ins_code;
                              std::string alt_conf;
                              coot::atom_spec_t atom_spec_1(chain_id, res_no, ins_code, atom_name_1_raw, alt_conf);
                              coot::atom_spec_t atom_spec_2(chain_id, res_no, ins_code, atom_name_2_raw, alt_conf);
                              coot::atom_spec_t atom_spec_3(chain_id, res_no, ins_code, atom_name_3_raw, alt_conf);
                              coot::atom_spec_t atom_spec_4(chain_id, res_no, ins_code, atom_name_4_raw, alt_conf);
                              mmdb::Atom *at_1 = coot::util::get_atom_using_fuzzy_search(atom_spec_1, mol);
                              mmdb::Atom *at_2 = coot::util::get_atom_using_fuzzy_search(atom_spec_2, mol);
                              mmdb::Atom *at_3 = coot::util::get_atom_using_fuzzy_search(atom_spec_3, mol);
                              mmdb::Atom *at_4 = coot::util::get_atom_using_fuzzy_search(atom_spec_4, mol);
                              if (at_1 && at_2 && at_3 && at_4) {
                                 clipper::Coord_orth pt_1(at_1->x, at_1->y, at_1->z);
                                 clipper::Coord_orth pt_2(at_2->x, at_2->y, at_2->z);
                                 clipper::Coord_orth pt_3(at_3->x, at_3->y, at_3->z);
                                 clipper::Coord_orth pt_4(at_4->x, at_4->y, at_4->z);
                                 clipper::Coord_orth mid(0.25 * (pt_1 + pt_2 + pt_3 + pt_4));
                                 set_rotation_centre(mid.x(), mid.y(), mid.z());
                              } else {
                                 std::cout << "WARNING:: failed to lookup an atom"
                                           << " " << atom_spec_1 << " " << atom_spec_2
                                           << " " << atom_spec_3 << " " << atom_spec_4 << std::endl;
                                 if (! at_1) std::cout << "WARNING missing at_1" << std::endl;
                                 if (! at_2) std::cout << "WARNING missing at_2" << std::endl;
                                 if (! at_3) std::cout << "WARNING missing at_3" << std::endl;
                                 if (! at_4) std::cout << "WARNING missing at_4" << std::endl;
                              }
                           } catch (const std::runtime_error &e) {
                              std::cout << "WARNING::" << e.what() << " from " << atom_ids << std::endl;
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   if (type == "plane") {
      // two forms
      // A|501:N49=C50=N54=C55 (880)
      // A|221:C=A|221:CA=A|221:O=A|222:N=A|222:CA=A|222:CD
      std::vector<std::string> parts_1 = coot::util::split_string(atom_ids, "|");
      if (parts_1.size() == 2) {
         // simple case = all atoms in the same residue
         std::string chain_id = parts_1[0];
         std::string rest_1   = parts_1[1];
         if (debug)
            std::cout << "debug:: plane_block: chain_id: " << chain_id << " rest_1: " << rest_1 << std::endl;
         std::vector<std::string> parts_2 = coot::util::split_string(rest_1, ":");
         if (parts_2.size() == 2) {
            std::string res_no_str = parts_2[0];
            std::string rest_2     = parts_2[1];
            if (debug)
               std::cout << "debug:: plane_block: res_no_str: " << res_no_str << " rest_2: " << rest_2 << std::endl;
            std::vector<std::string> atom_names = coot::util::split_string(rest_2, "=");
            try {
               int res_no = coot::util::string_to_int(res_no_str);
               std::string ins_code;
               if (atom_names.size() >= 4) {
                  if (debug)
                     for (unsigned int i=0; i<atom_names.size(); i++)
                        std::cout << "part: " << atom_names[i] << std::endl;
                  std::vector<mmdb::Atom *> atoms;
                  for (unsigned int i=0; i<atom_names.size(); i++) {
                     const std::string &atom_name = atom_names[i];
                     std::string alt_conf;
                     coot::atom_spec_t as(chain_id, res_no, ins_code, atom_name, alt_conf);
                     mmdb:: Atom *at = coot::util::get_atom_using_fuzzy_search(as, mol);
                     if (at)
                        atoms.push_back(at);
                  }
                  if (atoms.size() == atom_names.size()) {
                     clipper::Coord_orth sum(0,0,0);
                     for (auto at : atoms) {
                        clipper::Coord_orth pt(at->x, at->y, at->z);
                        sum += pt;
                     }
                     double f = 1.0/static_cast<double>(atom_names.size());
                     clipper::Coord_orth mid(f * sum);
                     set_rotation_centre(mid.x(), mid.y(), mid.z());
                  }
               }
            }
            catch (const std::runtime_error &e) {
               std::cout << "WARNING::" << e.what() << " from " << res_no_str << " from " << rest_1 << std::endl;
            }
         }
      }

      if (parts_1.size() > 3) {
         std::vector<std::string> gphl_atom_specs = coot::util::split_string(atom_ids, "=");
         std::vector<mmdb:: Atom *> atoms;
         for (const auto &gas : gphl_atom_specs) {
            coot::atom_spec_t as = gphl_spec_to_coot_spec(gas);
            mmdb:: Atom *at = coot::util::get_atom_using_fuzzy_search(as, mol);
            if (at)
               atoms.push_back(at);
            else
               std::cout << "WARNING:: failed to get atom from  " << gas << "   " << as << std::endl;
         }
         if (debug)
            std::cout << "debug:: plane block: here with atoms size " << atoms.size() << std::endl;
         if (atoms.size() > 3) {
            clipper::Coord_orth sum(0,0,0);
            for (auto at : atoms) {
               clipper::Coord_orth pt(at->x, at->y, at->z);
               sum += pt;
            }
            double f = 1.0/static_cast<double>(atoms.size());
            clipper::Coord_orth mid(f * sum);
            set_rotation_centre(mid.x(), mid.y(), mid.z());
         } else {
            std::cout << "WARNING:: failed to find 3 or more atoms from " << atom_ids << std::endl;
         }
      }
   }

   if (type == "ideal-contact") {
      // e.g.:
      // A|318:O(PHE)=A|320:N(ALA) [ideal 1-N]
      // A|307:N=CD1 (LEU) [ideal 1-5]
      // A|338:CD(LYS)=A|633:O(HOH) symm: 1555=4455 [ideal 1-N]
      std::vector<std::string> parts_1 = coot::util::split_string(atom_ids, "|");
      if (parts_1.size() == 2) {
         // simple case = all atoms in the same residue
         std::string chain_id = parts_1[0];
         std::string rest_1   = parts_1[1];
         if (debug)
            std::cout << "debug:: ideal_contact_block: chain_id: " << chain_id << " rest_1: " << rest_1 << std::endl;
         std::vector<std::string> parts_2 = coot::util::split_string(rest_1, ":");
         if (parts_2.size() == 2) {
            std::string res_no_str = parts_2[0];
            std::string rest_2     = parts_2[1];
            if (debug)
               std::cout << "debug:: ideal_contact_block: res_no_str: " << res_no_str << " rest_2: " << rest_2 << std::endl;
            // now strip on space to remove the residue type and ideal notes
            std::vector<std::string> parts_3 = coot::util::split_string(rest_2, " ");
            if (parts_3.size() > 0) {
               std::string rest_3 = parts_3[0];
               std::vector<std::string> atom_names = coot::util::split_string(rest_3, "=");
               try {
                  int res_no = coot::util::string_to_int(res_no_str);
                  std::string ins_code;
                  if (atom_names.size() == 2) {
                     if (debug)
                        for (unsigned int i=0; i<atom_names.size(); i++)
                           std::cout << "debug:: ideal_contact_block simple case atom-name: " << atom_names[i] << std::endl;
                     std::vector<mmdb::Atom *> atoms;
                     for (unsigned int i=0; i<atom_names.size(); i++) {
                        const std::string &atom_name = atom_names[i];
                        std::string alt_conf;
                        coot::atom_spec_t as(chain_id, res_no, ins_code, atom_name, alt_conf);
                        mmdb:: Atom *at = coot::util::get_atom_using_fuzzy_search(as, mol);
                        if (at)
                           atoms.push_back(at);
                     }
                     if (atoms.size() == atom_names.size()) {
                        clipper::Coord_orth sum(0,0,0);
                        for (auto at : atoms) {
                           clipper::Coord_orth pt(at->x, at->y, at->z);
                           sum += pt;
                        }
                        double f = 1.0/static_cast<double>(atom_names.size());
                        clipper::Coord_orth mid(f * sum);
                        set_rotation_centre(mid.x(), mid.y(), mid.z());
                     }
                  }
               }
               catch (const std::runtime_error &e) {
                  std::cout << "WARNING::" << e.what() << " from " << res_no_str << " from " << rest_1 << std::endl;
               }
            }
         }
      }
      if (parts_1.size() == 3) {
         // atoms are in different residues - the typical/normal case
         // e.g. A|172:C(GLN)=A|173:C(MET) [ideal 1-(2,3)-4]
         std::vector<std::string> parts_2 = coot::util::split_string(atom_ids, " ");
         if (parts_2.size() > 0) {
            std::string front = parts_2[0];
            std::vector<std::string> gphl_atom_specs = coot::util::split_string(front, "=");
            std::vector<mmdb:: Atom *> atoms;
            for (const auto &gas : gphl_atom_specs) {
               coot::atom_spec_t as = gphl_spec_to_coot_spec(gas);
               mmdb:: Atom *at = coot::util::get_atom_using_fuzzy_search(as, mol);
               if (at)
                  atoms.push_back(at);
               else
                  std::cout << "WARNING:: ideal-contact failed to get atom from  " << gas << "   " << as << std::endl;
            }
            if (debug)
               std::cout << "debug here in ideal-contact with atoms size " << atoms.size() << std::endl;
            if (atoms.size() == 2) {
               clipper::Coord_orth sum(0,0,0);
               for (auto at : atoms) {
                  clipper::Coord_orth pt(at->x, at->y, at->z);
                  sum += pt;
               }
               double f = 1.0/static_cast<double>(atoms.size());
               clipper::Coord_orth mid(f * sum);
               set_rotation_centre(mid.x(), mid.y(), mid.z());
            }
         }
      }
   }

}

#ifdef USE_PYTHON
PyObject *global_phasing_screen(int imol, PyObject *screen_dict) {

   class screen_results_t {
   public:
      screen_results_t(int imol) : imol(imol) {}
      int imol;
      std::vector<std::string> front_page_strings;
      std::vector<std::map<std::string, std::string> > bond_dict_vec;
      std::vector<std::map<std::string, std::string> > angle_dict_vec;
      std::vector<std::map<std::string, std::string> > torsion_dict_vec;
      std::vector<std::map<std::string, std::string> > plane_dict_vec;
      std::vector<std::map<std::string, std::string> > ideal_contact_dict_vec;
      std::vector<std::pair<std::string, std::string> > unhappy_atoms;
   };

   auto widget_from_builder = [] (const std::string &w_name, GtkBuilder *builder) {
      GtkWidget *w = GTK_WIDGET(gtk_builder_get_object(GTK_BUILDER(builder), w_name.c_str()));
      return  w;
   };

   auto connect_callback = +[] (GtkWidget *go_button, int imol, const std::string &atom_ids, const std::string &type) {

      struct button_wrapper_t {
         int imol;
         std::string atom_ids;
         std::string type;
      };

      auto go_button_clicked = +[] (G_GNUC_UNUSED GtkButton *button, gpointer user_data) {

         // 20251005-PE I don't cast like this anywhere else in the code - hmmm.
         button_wrapper_t *bw = static_cast<button_wrapper_t *>(user_data);
         if (bw) {
            if (! bw->atom_ids.empty()) {
               go_to_gphl_atoms(bw->imol, bw->atom_ids, bw->type);
            }
         }
      };

      button_wrapper_t *bw = new button_wrapper_t;
      bw->imol = imol;
      bw->atom_ids =  atom_ids;
      bw->type = type;
      g_signal_connect(G_OBJECT(go_button), "clicked", G_CALLBACK(go_button_clicked), bw);
   };

   auto render_to_widget = [widget_from_builder, connect_callback] (const screen_results_t &screen_results) {

      GtkBuilder *builder = gtk_builder_new();
      GError* error = NULL;
      std::string dir = coot::package_data_dir();
      std::string dir_ui = coot::util::append_dir_dir(dir, "ui");
      std::string ui_file_name = "global-phasing-screen.ui";
      std::string ui_file_full = coot::util::append_dir_file(dir_ui, ui_file_name);
      gboolean status = gtk_builder_add_from_file(builder, ui_file_full.c_str(), &error);
      if (status == FALSE) {
         std::cout << "ERROR:: Failure to read or parse " << ui_file_full << std::endl;
         std::cout << error->message << std::endl;
      } else {

         // first setup the dialog widget for the "Close" dialog callback
         GtkWidget *dialog = widget_from_builder("gphl-screen-dialog", builder);
         set_transient_and_position(COOT_UNDEFINED_WINDOW, dialog);
         GtkWidget *close_button = widget_from_builder("gphl_screen_close_button", builder);
         g_object_set_data(G_OBJECT(close_button), "dialog", dialog);

         // --------------------- Front Page -------------------------------------

         if (! screen_results.front_page_strings.empty()) {
            GtkWidget *label = widget_from_builder("screen-front-page-label", builder);
            std::string s = "<span font_family='monospace'>\n";
            for (unsigned int i=0; i<screen_results.front_page_strings.size(); i++) {
               s += screen_results.front_page_strings[i];
               s += "\n";
            }
            s += "</span>\n";
            gtk_label_set_markup(GTK_LABEL(label), s.c_str());
         }

         // --------------------- Bonds -------------------------------------

         if (! screen_results.bond_dict_vec.empty()) {
            const std::vector<std::map<std::string, std::string> > &v = screen_results.bond_dict_vec;
            GtkWidget *grid = widget_from_builder("gphl-screen-bonds-grid", builder);
            gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
            gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Actual"),       0, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Ideal"),        1, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" Sigma"),       2, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Delta/Sigma"),  3, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("AtomIds"),      4, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ResType"),      5, 0, 1, 1);

            std::vector<std::pair<std::string, unsigned int> > vv = {
               {"actual", 0}, {"ideal", 1}, {"sigma", 2}, {"delta_sigma", 3}, {"atom_ids", 4}, {"res_type", 5} };

            for (unsigned int i=0; i<v.size(); i++) {
               std::string atom_ids;
               unsigned int i_row = i+1;
               const std::map<std::string, std::string> &m = v[i];
               std::map<std::string, std::string>::const_iterator it;
               for (unsigned int j=0; j<vv.size(); j++) {
                  it = m.find(vv[j].first);
                  if (it != m.end()) {
                     GtkWidget *w = gtk_label_new(it->second.c_str());
                     if (vv[j].first == "atom_ids") {
                        gtk_widget_set_halign(w, GTK_ALIGN_START);
                        atom_ids = it->second;
                     }
                     gtk_grid_attach(GTK_GRID(grid), w, vv[j].second, i_row, 1, 1);
                  }
               }
               GtkWidget *go_button = gtk_button_new_with_label("Go");
               connect_callback(go_button, screen_results.imol, atom_ids, "bond");
               gtk_grid_attach(GTK_GRID(grid), go_button, 6, i_row, 1, 1);

               // GtkWidget *fix_button = gtk_button_new_with_label("Try Fix");
               // gtk_grid_attach(GTK_GRID(grid), fix_button, 7, i_row, 1, 1);
               i_row++;
            }
         }

         // --------------------- Angles -------------------------------------

         if (! screen_results.angle_dict_vec.empty()) {
            const std::vector<std::map<std::string, std::string> > &v = screen_results.angle_dict_vec;
            GtkWidget *grid = widget_from_builder("gphl-screen-angles-grid", builder);
            gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
            gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" Actual "),       0, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" Ideal "),        1, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" Sigma "),        2, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" Delta/Sigma "),  3, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" AtomIds "),      4, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" ResType "),      5, 0, 1, 1);

            std::vector<std::pair<std::string, unsigned int> > vv = {
               {"actual", 0}, {"ideal", 1}, {"sigma", 2}, {"delta_sigma", 3}, {"atom_ids", 4}, {"res_type", 5} };

            for (unsigned int i=0; i<v.size(); i++) {
               std::string atom_ids;
               unsigned int i_row = i+1;
               const std::map<std::string, std::string> &m = v[i];
               std::map<std::string, std::string>::const_iterator it;
               for (unsigned int j=0; j<vv.size(); j++) {
                  it = m.find(vv[j].first);
                  if (it != m.end()) {
                     GtkWidget *w = gtk_label_new(it->second.c_str());
                     if (vv[j].first == "atom_ids") {
                        gtk_widget_set_halign(w, GTK_ALIGN_START);
                        atom_ids = it->second;
                     }
                     gtk_grid_attach(GTK_GRID(grid), w, vv[j].second, i_row, 1, 1);
                  }
               }
               GtkWidget *go_button  = gtk_button_new_with_label("Go");
               connect_callback(go_button, screen_results.imol, atom_ids, "angle");
               gtk_grid_attach(GTK_GRID(grid), go_button, 6, i_row, 1, 1);
               // GtkWidget *fix_button = gtk_button_new_with_label("Try Fix");
               // gtk_grid_attach(GTK_GRID(grid), fix_button, 7, i_row, 1, 1);
               i_row++;
            }
         }

         // --------------------- Torsions -------------------------------------

         if (! screen_results.torsion_dict_vec.empty()) {
            const std::vector<std::map<std::string, std::string> > &v = screen_results.torsion_dict_vec;
            GtkWidget *grid = widget_from_builder("gphl-screen-torsions-grid", builder);
            gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
            gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Actual"),       0, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Ideal"),        1, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" Sigma"),       2, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Delta/Sigma"),  3, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("AtomIds"),      4, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ResType"),      5, 0, 1, 1);

            std::vector<std::pair<std::string, unsigned int> > vv = {
               {"actual", 0}, {"ideal", 1}, {"sigma", 2}, {"delta_sigma", 3}, {"atom_ids", 4}, {"res_type", 5} };

            for (unsigned int i=0; i<v.size(); i++) {
               std::string atom_ids;
               unsigned int i_row = i+1;
               const std::map<std::string, std::string> &m = v[i];
               std::map<std::string, std::string>::const_iterator it;
               for (unsigned int j=0; j<vv.size(); j++) {
                  it = m.find(vv[j].first);
                  if (it != m.end()) {
                     GtkWidget *w = gtk_label_new(it->second.c_str());
                     if (vv[j].first == "atom_ids") {
                        gtk_widget_set_halign(w, GTK_ALIGN_START);
                        atom_ids = it->second;
                     }
                     gtk_grid_attach(GTK_GRID(grid), w, vv[j].second, i_row, 1, 1);
                  }
               }
               GtkWidget *go_button = gtk_button_new_with_label("Go");
               connect_callback(go_button, screen_results.imol, atom_ids, "torsion");
               gtk_grid_attach(GTK_GRID(grid), go_button,  6, i_row, 1, 1);
               // GtkWidget *fix_button = gtk_button_new_with_label("Try Fix");
               // gtk_grid_attach(GTK_GRID(grid), fix_button, 7, i_row, 1, 1);
               i_row++;
            }
         }

         // --------------------- Planes -------------------------------------

         if (! screen_results.plane_dict_vec.empty()) {
            const std::vector<std::map<std::string, std::string> > &v = screen_results.plane_dict_vec;
            GtkWidget *grid = widget_from_builder("gphl-screen-planes-grid", builder);
            gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
            gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Actual"),       0, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new(" Sigma"),       1, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Actual/Sigma"), 2, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("AtomIds"),      3, 0, 1, 1);

            std::vector<std::pair<std::string, unsigned int> > vv = {
               {"actual", 0}, {"sigma", 1}, {"actual_sigma", 2}, {"atom_ids", 3} };

            for (unsigned int i=0; i<v.size(); i++) {
               std::string atom_ids;
               unsigned int i_row = i+1;
               const std::map<std::string, std::string> &m = v[i];
               std::map<std::string, std::string>::const_iterator it;
               for (unsigned int j=0; j<vv.size(); j++) {
                  it = m.find(vv[j].first);
                  if (it != m.end()) {
                     GtkWidget *w = gtk_label_new(it->second.c_str());
                     if (vv[j].first == "atom_ids") {
                        gtk_widget_set_halign(w, GTK_ALIGN_START);
                        atom_ids = it->second;
                     }
                     gtk_grid_attach(GTK_GRID(grid), w, vv[j].second, i_row, 1, 1);
                  }
               }
               GtkWidget *go_button = gtk_button_new_with_label("Go");
               connect_callback(go_button, screen_results.imol, atom_ids, "plane");
               gtk_grid_attach(GTK_GRID(grid), go_button,  6, i_row, 1, 1);
               // GtkWidget *fix_button = gtk_button_new_with_label("Try Fix");
               // gtk_grid_attach(GTK_GRID(grid), fix_button, 7, i_row, 1, 1);
               i_row++;
            }
         }

         // --------------------- Ideal-Contact -------------------------------------

         if (! screen_results.bond_dict_vec.empty()) {
            const std::vector<std::map<std::string, std::string> > &v = screen_results.ideal_contact_dict_vec;
            GtkWidget *grid = widget_from_builder("gphl-screen-ideal-contact-grid", builder);
            gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
            gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Actual"),       0, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("ContactD"),     1, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Delta"),        2, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Delta/Sigma"),  3, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("AtomIds"),      4, 0, 1, 1);

            std::vector<std::pair<std::string, unsigned int> > vv = {
               {"actual", 0}, {"contactD", 1}, {"delta", 2}, {"sigma", 3}, {"delta_sigma", 4}, {"atom_ids", 5} };

            for (unsigned int i=0; i<v.size(); i++) {
               std::string atom_ids;
               unsigned int i_row = i+1;
               const std::map<std::string, std::string> &m = v[i];
               std::map<std::string, std::string>::const_iterator it;
               for (unsigned int j=0; j<vv.size(); j++) {
                  it = m.find(vv[j].first);
                  if (it != m.end()) {
                     GtkWidget *w = gtk_label_new(it->second.c_str());
                     if (vv[j].first == "atom_ids") {
                        gtk_widget_set_halign(w, GTK_ALIGN_START);
                        atom_ids = it->second;
                     }
                     gtk_grid_attach(GTK_GRID(grid), w, vv[j].second, i_row, 1, 1);
                  } else {
                     std::cout << "Failed to find " << vv[j].first << std::endl;
                  }
               }
               GtkWidget *go_button = gtk_button_new_with_label("Go");
               connect_callback(go_button, screen_results.imol, atom_ids, "ideal-contact");
               gtk_grid_attach(GTK_GRID(grid), go_button, 6, i_row, 1, 1);
               // GtkWidget *fix_button = gtk_button_new_with_label("Try Fix");
               // gtk_grid_attach(GTK_GRID(grid), fix_button, 7, i_row, 1, 1);
               i_row++;
            }
         }

         // --------------------- Unhappy Atoms -------------------------------------

         if (! screen_results.unhappy_atoms.empty()) {
            const std::vector<std::pair<std::string, std::string> > &v = screen_results.unhappy_atoms;
            GtkWidget *grid = widget_from_builder("gphl-screen-unhappy-atom-grid", builder);
            gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
            gtk_grid_set_row_spacing(GTK_GRID(grid), 2);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Atom"), 0, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), gtk_label_new("F/(medianF)"), 1, 0, 1, 1);

            for (unsigned int i=0; i<v.size(); i++) {
               std::string atom_id = v[i].first;
               std::string f = v[i].second;
               unsigned int i_row = i+1;
               GtkWidget *l0 = gtk_label_new(atom_id.c_str());
               GtkWidget *l1 = gtk_label_new(f.c_str());
               gtk_grid_attach(GTK_GRID(grid), l0, 0, i_row, 1, 1);
               gtk_grid_attach(GTK_GRID(grid), l1, 1, i_row, 1, 1);

               GtkWidget *go_button = gtk_button_new_with_label("Go");
               connect_callback(go_button, screen_results.imol, atom_id, "unhappy-atom");
               gtk_grid_attach(GTK_GRID(grid), go_button, 2, i_row, 1, 1);
               i_row++;
            }
         }

         // --------------------- Finally -------------------------------------

         if (dialog)
            gtk_widget_set_visible(dialog, TRUE);
      }
   };

   PyObject *r = Py_False;
   if (screen_dict) {
      if (PyDict_Check(screen_dict)) {

         screen_results_t screen_results(imol);

         PyObject *fp_data_py = PyDict_GetItemString(screen_dict, "Front page"); // a borrowed reference
         if (fp_data_py) {
            if (PyList_Check(fp_data_py)) {
               long lpi = PyObject_Length(fp_data_py);
               for (long i=0; i<lpi; i++) {
                  PyObject *line_py = PyList_GetItem(fp_data_py, i);
                  if (PyUnicode_Check(line_py)) {
                     std::string l = PyBytes_AS_STRING(PyUnicode_AsUTF8String(line_py));
                     screen_results.front_page_strings.push_back(l);
                  }
               }
            }
         } else {
            std::cout << "fp_data_py was null" << std::endl;
         }

         {
            PyObject *bl_data_py = PyDict_GetItemString(screen_dict, "Bond length"); // a borrowed reference
            if (bl_data_py) {
               if (PyList_Check(bl_data_py)) {
                  long lpi = PyObject_Length(bl_data_py);
                  for (long i=0; i<lpi; i++) {
                     PyObject *bl_dict_py = PyList_GetItem(bl_data_py, i);
                     if (PyDict_Check(bl_dict_py)) {
                        PyObject *actual_py      = PyDict_GetItemString(bl_dict_py, "actual");
                        PyObject *ideal_py       = PyDict_GetItemString(bl_dict_py, "ideal");
                        PyObject *sigma_py       = PyDict_GetItemString(bl_dict_py, "sigma");
                        PyObject *delta_sigma_py = PyDict_GetItemString(bl_dict_py, "delta_sigma");
                        PyObject *atom_ids_py    = PyDict_GetItemString(bl_dict_py, "atom_ids");
                        PyObject *res_type_py    = PyDict_GetItemString(bl_dict_py, "res_type");
                        if (true) {
                           if (PyUnicode_Check(actual_py) &&
                               PyUnicode_Check(ideal_py) &&
                               PyUnicode_Check(sigma_py) &&
                               PyUnicode_Check(delta_sigma_py) &&
                               PyUnicode_Check(atom_ids_py) &&
                               PyUnicode_Check(res_type_py)) {
                              std::string actual = PyBytes_AS_STRING(PyUnicode_AsUTF8String(actual_py));
                              std::string ideal  = PyBytes_AS_STRING(PyUnicode_AsUTF8String(ideal_py));
                              std::string sigma  = PyBytes_AS_STRING(PyUnicode_AsUTF8String(sigma_py));
                              std::string delta_sigma = PyBytes_AS_STRING(PyUnicode_AsUTF8String(delta_sigma_py));
                              std::string atom_ids = PyBytes_AS_STRING(PyUnicode_AsUTF8String(atom_ids_py));
                              std::string res_type = PyBytes_AS_STRING(PyUnicode_AsUTF8String(res_type_py));
                              // std::cout << "bond " << actual << " " << ideal << " " << sigma << " " << atom_ids << std::endl;
                              std::map<std::string, std::string> bond_dict;
                              bond_dict["actual"] = actual;
                              bond_dict["ideal"]  = ideal;
                              bond_dict["sigma"]  = sigma;
                              bond_dict["delta_sigma"]  = delta_sigma;
                              bond_dict["atom_ids"]  = atom_ids;
                              bond_dict["res_type"]  = res_type;
                              screen_results.bond_dict_vec.push_back(bond_dict);
                           }
                        }
                     }
                  }
               }
            }
         }

         {
            PyObject *angle_data_py = PyDict_GetItemString(screen_dict, "Bond angle"); // a borrowed reference
            if (angle_data_py) {
               if (PyList_Check(angle_data_py)) {
                  long lpi = PyObject_Length(angle_data_py);
                  for (long i=0; i<lpi; i++) {
                     PyObject *angle_dict_py = PyList_GetItem(angle_data_py, i);
                     if (PyDict_Check(angle_dict_py)) {
                        PyObject *actual_py      = PyDict_GetItemString(angle_dict_py, "actual");
                        PyObject *ideal_py       = PyDict_GetItemString(angle_dict_py, "ideal");
                        PyObject *sigma_py       = PyDict_GetItemString(angle_dict_py, "sigma");
                        PyObject *delta_sigma_py = PyDict_GetItemString(angle_dict_py, "delta_sigma");
                        PyObject *atom_ids_py    = PyDict_GetItemString(angle_dict_py, "atom_ids");
                        PyObject *res_type_py    = PyDict_GetItemString(angle_dict_py, "res_type");
                        if (true) { // check the pointers
                           if (PyUnicode_Check(actual_py) &&
                               PyUnicode_Check(ideal_py) &&
                               PyUnicode_Check(sigma_py) &&
                               PyUnicode_Check(delta_sigma_py) &&
                               PyUnicode_Check(atom_ids_py) &&
                               PyUnicode_Check(res_type_py)) {
                              std::string actual = PyBytes_AS_STRING(PyUnicode_AsUTF8String(actual_py));
                              std::string ideal  = PyBytes_AS_STRING(PyUnicode_AsUTF8String(ideal_py));
                              std::string sigma  = PyBytes_AS_STRING(PyUnicode_AsUTF8String(sigma_py));
                              std::string delta_sigma = PyBytes_AS_STRING(PyUnicode_AsUTF8String(delta_sigma_py));
                              std::string atom_ids = PyBytes_AS_STRING(PyUnicode_AsUTF8String(atom_ids_py));
                              std::string res_type = PyBytes_AS_STRING(PyUnicode_AsUTF8String(res_type_py));
                              // std::cout << "angle " << actual << " " << ideal << " " << sigma << " " << atom_ids << std::endl;
                              std::map<std::string, std::string> angle_dict;
                              angle_dict["actual"] = actual;
                              angle_dict["ideal"]  = ideal;
                              angle_dict["sigma"]  = sigma;
                              angle_dict["delta_sigma"]  = delta_sigma;
                              angle_dict["atom_ids"]  = atom_ids;
                              angle_dict["res_type"]  = res_type;
                              screen_results.angle_dict_vec.push_back(angle_dict);
                           }
                        }
                     }
                  }
               }
            }
         }

         {
            PyObject *torsion_data_py = PyDict_GetItemString(screen_dict, "Torsion"); // a borrowed reference
            if (torsion_data_py) {
               if (PyList_Check(torsion_data_py)) {
                  long lpi = PyObject_Length(torsion_data_py);
                  for (long i=0; i<lpi; i++) {
                     PyObject *torsion_dict_py = PyList_GetItem(torsion_data_py, i);
                     if (PyDict_Check(torsion_dict_py)) {
                        PyObject *actual_py      = PyDict_GetItemString(torsion_dict_py, "actual");
                        PyObject *ideal_py       = PyDict_GetItemString(torsion_dict_py, "ideal");
                        PyObject *sigma_py       = PyDict_GetItemString(torsion_dict_py, "sigma");
                        PyObject *delta_sigma_py = PyDict_GetItemString(torsion_dict_py, "delta_sigma");
                        PyObject *atom_ids_py    = PyDict_GetItemString(torsion_dict_py, "atom_ids");
                        PyObject *res_type_py    = PyDict_GetItemString(torsion_dict_py, "res_type");
                        if (true) { // check the pointers
                           if (PyUnicode_Check(actual_py) &&
                               PyUnicode_Check(ideal_py) &&
                               PyUnicode_Check(sigma_py) &&
                               PyUnicode_Check(delta_sigma_py) &&
                               PyUnicode_Check(atom_ids_py) &&
                               PyUnicode_Check(res_type_py)) {
                              std::string actual      = PyBytes_AS_STRING(PyUnicode_AsUTF8String(actual_py));
                              std::string ideal       = PyBytes_AS_STRING(PyUnicode_AsUTF8String(ideal_py));
                              std::string sigma       = PyBytes_AS_STRING(PyUnicode_AsUTF8String(sigma_py));
                              std::string delta_sigma = PyBytes_AS_STRING(PyUnicode_AsUTF8String(delta_sigma_py));
                              std::string atom_ids    = PyBytes_AS_STRING(PyUnicode_AsUTF8String(atom_ids_py));
                              std::string res_type    = PyBytes_AS_STRING(PyUnicode_AsUTF8String(res_type_py));
                              // std::cout << "torsion " << actual << " " << ideal << " " << sigma << " " << atom_ids << std::endl;
                              std::map<std::string, std::string> torsion_dict;
                              torsion_dict["actual"] = actual;
                              torsion_dict["ideal"]  = ideal;
                              torsion_dict["sigma"]  = sigma;
                              torsion_dict["delta_sigma"] = delta_sigma;
                              torsion_dict["atom_ids"]    = atom_ids;
                              torsion_dict["res_type"]    = res_type;
                              screen_results.torsion_dict_vec.push_back(torsion_dict);
                           }
                        }
                     }
                  }
               }
            }
         }

         {
            PyObject *plane_data_py = PyDict_GetItemString(screen_dict, "Plane"); // a borrowed reference
            if (plane_data_py) {
               if (PyList_Check(plane_data_py)) {
                  long lpi = PyObject_Length(plane_data_py);
                  for (long i=0; i<lpi; i++) {
                     PyObject *plane_dict_py = PyList_GetItem(plane_data_py, i);
                     if (PyDict_Check(plane_dict_py)) {
                        PyObject *actual_py       = PyDict_GetItemString(plane_dict_py, "actual");
                        PyObject *sigma_py        = PyDict_GetItemString(plane_dict_py, "sigma");
                        PyObject *actual_sigma_py = PyDict_GetItemString(plane_dict_py, "actual_sigma");
                        PyObject *atom_ids_py     = PyDict_GetItemString(plane_dict_py, "atom_ids");
                        if (actual_py && sigma_py && actual_sigma_py && atom_ids_py) {
                           if (PyUnicode_Check(actual_py) &&
                               PyUnicode_Check(sigma_py) &&
                               PyUnicode_Check(actual_sigma_py) &&
                               PyUnicode_Check(atom_ids_py)) {
                              std::string actual       = PyBytes_AS_STRING(PyUnicode_AsUTF8String(actual_py));
                              std::string sigma        = PyBytes_AS_STRING(PyUnicode_AsUTF8String(sigma_py));
                              std::string actual_sigma = PyBytes_AS_STRING(PyUnicode_AsUTF8String(actual_sigma_py));
                              std::string atom_ids     = PyBytes_AS_STRING(PyUnicode_AsUTF8String(atom_ids_py));
                              // std::cout << "plane " << actual << " " << sigma << " " << atom_ids << std::endl;
                              std::map<std::string, std::string> plane_dict;
                              plane_dict["actual"] = actual;
                              plane_dict["sigma"]  = sigma;
                              plane_dict["actual_sigma"] = actual_sigma;
                              plane_dict["atom_ids"]    = atom_ids;
                              screen_results.plane_dict_vec.push_back(plane_dict);
                           }
                        }
                     }
                  }
               }
            }
         }

         {
            PyObject *ideal_contact_data_py = PyDict_GetItemString(screen_dict, "Ideal Contact"); // a borrowed reference
            if (ideal_contact_data_py) {
               if (PyList_Check(ideal_contact_data_py)) {
                  long lpi = PyObject_Length(ideal_contact_data_py);
                  for (long i=0; i<lpi; i++) {
                     PyObject *ideal_contact_dict_py = PyList_GetItem(ideal_contact_data_py, i);
                     if (PyDict_Check(ideal_contact_dict_py)) {
                        PyObject *actual_py      = PyDict_GetItemString(ideal_contact_dict_py, "actual");
                        PyObject *contactD_py    = PyDict_GetItemString(ideal_contact_dict_py, "contactD");
                        PyObject *sigma_py       = PyDict_GetItemString(ideal_contact_dict_py, "sigma");
                        PyObject *delta_py       = PyDict_GetItemString(ideal_contact_dict_py, "delta");
                        PyObject *delta_sigma_py = PyDict_GetItemString(ideal_contact_dict_py, "delta_sigma");
                        PyObject *atom_ids_py    = PyDict_GetItemString(ideal_contact_dict_py, "atom_ids");
                        if (actual_py && contactD_py  && sigma_py && delta_sigma_py && atom_ids_py) {
                           if (PyUnicode_Check(actual_py) &&
                               PyUnicode_Check(contactD_py) &&
                               PyUnicode_Check(sigma_py) &&
                               PyUnicode_Check(delta_py) &&
                               PyUnicode_Check(delta_sigma_py) &&
                               PyUnicode_Check(atom_ids_py)) {
                              std::string actual      = PyBytes_AS_STRING(PyUnicode_AsUTF8String(actual_py));
                              std::string contactD    = PyBytes_AS_STRING(PyUnicode_AsUTF8String(contactD_py));
                              std::string sigma       = PyBytes_AS_STRING(PyUnicode_AsUTF8String(sigma_py));
                              std::string delta       = PyBytes_AS_STRING(PyUnicode_AsUTF8String(delta_py));
                              std::string delta_sigma = PyBytes_AS_STRING(PyUnicode_AsUTF8String(delta_sigma_py));
                              std::string atom_ids    = PyBytes_AS_STRING(PyUnicode_AsUTF8String(atom_ids_py));
                              std::map<std::string, std::string> ideal_contact_dict;
                              ideal_contact_dict["actual"]   = actual;
                              ideal_contact_dict["contactD"] = contactD;
                              ideal_contact_dict["delta"]    = delta;
                              ideal_contact_dict["sigma"]    = sigma;
                              ideal_contact_dict["delta_sigma"] = delta_sigma;
                              ideal_contact_dict["atom_ids"]    = atom_ids;
                              screen_results.ideal_contact_dict_vec.push_back(ideal_contact_dict);
                           }
                        }
                     }
                  }
               }
            }
         }

         {
            PyObject *unhappy_atom_data_py = PyDict_GetItemString(screen_dict, "Unhappy Atom"); // a borrowed reference
            if (unhappy_atom_data_py) {
               if (PyList_Check(unhappy_atom_data_py)) {
                  long lpi = PyObject_Length(unhappy_atom_data_py);
                  for (long i=0; i<lpi; i++) {
                     PyObject *unhappy_atom_dict_py = PyList_GetItem(unhappy_atom_data_py, i);
                     if (PyDict_Check(unhappy_atom_dict_py)) {
                        PyObject *atom_id_py   = PyDict_GetItemString(unhappy_atom_dict_py, "atom_id");
                        PyObject *f_medianf_py = PyDict_GetItemString(unhappy_atom_dict_py, "f_over_median");
                        if (atom_id_py && f_medianf_py) {
                           std::string atom_id   = PyBytes_AS_STRING(PyUnicode_AsUTF8String(atom_id_py));
                           std::string f_medianf = PyBytes_AS_STRING(PyUnicode_AsUTF8String(f_medianf_py));
                           screen_results.unhappy_atoms.push_back(std::make_pair(atom_id, f_medianf));
                        }
                     }
                  }
               }
            }
         }

         render_to_widget(screen_results);

      }
   }
   if (PyBool_Check(r))
     Py_INCREF(r);
   return r;
}
#endif
