/* pli/flev.hh
 * 
 * Copyright 2012 by The University of Oxford
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#ifndef COOT_PLI_FLEV_HH
#define COOT_PLI_FLEV_HH

#include "geometry/protein-geometry.hh"
#include "lidia-core/svg-container.hh"
#include "lidia-core/lig-build.hh"
#include "lidia-core/svg-molecule.hh"
#include "residue-circle.hh"
#include "flev-annotations.hh"
#include "solvent-accessible-atom.hh"
#include "solvent-exposure-difference-helper.hh"
#include "pi-stacking.hh"
#include "optimise-residue-circles.hh"

// Draw all the things in SVG so that it can be used by Moorhen
//
class flev_t {

// public: ??

   class grid_index_t {

      int ii_;
      int jj_;
   public:
      enum { INVALID_INDEX = -1 };
      grid_index_t(int i, int j) {
         ii_ = i;
         jj_ = j;
      }
      // hhmmmm!  needed to compile contour_fragment
      // constructor, which takes a const reference
      // to a grid_index_t as an argument. I don't
      // understand why this is needed - or if it
      // works.
      grid_index_t() {
         ii_ = INVALID_INDEX;
         jj_ = INVALID_INDEX;
      }
      bool is_valid_p() const {
         return (ii_ != INVALID_INDEX);
      }
      int i() const { return ii_;}
      int j() const { return jj_;}
      bool operator==(const grid_index_t &grid_in) const {
         if (grid_in.i() != ii_) {
            return 0;
         } else {
            if (grid_in.j() != jj_) {
               return 0;
            } else {
               return 1; // they match
            }
         }
      }
   };

   class ligand_grid {
      double scale_fac;
      double LIGAND_TO_CANVAS_SCALE_FACTOR;
      double ligand_atoms_min_x;
      double ligand_atoms_min_y;
      double ligand_atoms_max_x;
      double ligand_atoms_max_y;
      double extra_extents; // added to both front and end, top and bottom.
      double n_grid_per_angstrom;
      // useful for coordinate transformation
      double mol_space_grid_min_x;
      double mol_space_grid_min_y;
      std::vector<std::vector<double> > grid_;
      int x_size_;
      int y_size_;
      void normalize(); // scale peak value to 1.0
      int square_type(int ii, int jj, float contour_level) const;
      std::vector<std::vector<lig_build::pos_t> > make_contour_lines(const std::vector<std::pair<lig_build::pos_t, lig_build::pos_t> > &line_fragments) const;
      double substitution_value(double r_squared, double bash_dist) const;
      // can throw a std::runtime_error (if result is out of grid)
      grid_index_t grid_pos_nearest(const lig_build::pos_t &pos) const;

      std::pair<int, int> mol_space_pos_to_grid_pos(const lig_build::pos_t &pos) const;
      lig_build::pos_t grid_pos_to_mol_space_pos(int ix, int iy) const;
      lig_build::pos_t grid_pos_as_double_to_mol_space_pos(double x, double y) const;


   public:
      //
      ligand_grid(const lig_build::pos_t &low_x_and_y,
                  const lig_build::pos_t &high_x_and_y);

      // REPLACE-ME-WITH-SVG
      void plot_contour_lines(const std::vector<std::vector<lig_build::pos_t> > &contour_lines);

      enum { MS_NO_CROSSING = -2,
             MS_NO_SQUARE = -1,
             MS_UP_0_0,
             MS_UP_0_1,
             MS_UP_1_0,
             MS_UP_1_1,
             MS_UP_0_0_and_0_1,
             MS_UP_0_0_and_1_0,
             MS_UP_0_0_and_1_1, // hideous valley
             MS_UP_0_1_and_1_0, // hideous valley
             MS_UP_0_1_and_1_1,
             MS_UP_1_0_and_1_1,
             MS_UP_0_0_and_0_1_and_1_0,
             MS_UP_0_0_and_0_1_and_1_1,
             MS_UP_0_0_and_1_0_and_1_1,
             MS_UP_0_1_and_1_0_and_1_1,
             };

      // Actually, not exactly zero but something small.
      // Don't return a grid-point/position that matches anything in
      // already_positioned.
      //
      std::pair<grid_index_t, lig_build::pos_t>
      find_nearest_zero(const lig_build::pos_t &pos, const std::vector<grid_index_t> &already_positioned) const;

      // arg is not const reference because get_ring_centres() caches
      // the return value inside mol.
      //
      void fill(svg_molecule_t mol);

      void print(int primary_index) const;

      double get(int i, int j) const {
         return grid_[i][j];
      }
      int x_size() const {
         return x_size_;
      }
      int y_size() const {
         return y_size_;
      }
      void add_quadratic(const std::vector<std::pair<lig_build::pos_t, double> > &attachment_points);
      lig_build::pos_t find_minimum_position() const;

      void avoid_ring_centres(const std::vector<std::vector<std::string> > &ring_atoms_list,
                              const lig_build::molecule_t<svg_atom_t, svg_bond_t> &mol);

      void add_for_accessibility(double bash_dist,
                                 double exp_fac, // 0.03 say
                                 const lig_build::pos_t &atom_pos);

      // fudge in a smoothly varing function (so that the contouring
      // behaves smoothly, rather that the jaggies that we'd get if we
      // added a top hat function values to 1.0A.
      //
      void add_for_accessibility_no_bash_dist_atom(double scale, const lig_build::pos_t &atom_pos);

      // REPLACE-ME-WITH-SVG
      void show_contour(float contour_level);

      svg_container_t show_contour(float contour_level,
                                   bool is_dashed,
                                   const std::string &col,
                                   const std::vector<lig_build::atom_ring_centre_info_t> &unlimited_atoms,
                                   const std::vector<std::vector<std::string> > &ring_atoms_list);

      // the "cutting" of the contour behaves differently if the
      // unlimited atom is a member of a ring (compared to if it is
      // not).
      // void show_contour_old(float contour_level,
      //                           const std::vector<lig_build::atom_ring_centre_info_t> &unlimited_atoms,
      //                           const std::vector<std::vector<std::string> > &ring_atoms_list);

   };

   // The lines constituting the fragment
   //
   class contour_fragment {

   public:
      enum { X_AXIS_LOW, X_AXIS_HIGH, Y_AXIS_LOW, Y_AXIS_HIGH };
      class coordinates {
         double frac_x;
         double frac_y;
         int i_ax;
         bool x_y_axis;

      public:
         coordinates() { frac_x = 0; frac_y = 0; i_ax = 0; x_y_axis = true; }
         coordinates(double f, int i) {
            x_y_axis = true;
#ifdef HAVE_IOSTREAM_HEADER
            if (f>1.0)
               std::cout << "-----> Bad frac " << f << std::endl;
            if (f<0.0)
               std::cout << "-----> Bad frac " << f << std::endl;
#endif
            frac_x = f;
            i_ax = i;
            if (i == X_AXIS_LOW)
               frac_y = 0.0;
            else
               frac_y = 1.0;
#ifdef HAVE_IOSTREAM_HEADER
            if (i_ax != X_AXIS_LOW)
               if (i_ax != X_AXIS_HIGH)
                  std::cout << "Bad axis to coordinates(f, i) f: "
                            << f << "  i: " << i << std::endl;
#endif
         }
         coordinates(int i, double f) {
            x_y_axis = true;
#ifdef HAVE_IOSTREAM_HEADER
            if (f>1.0)
               std::cout << "----->  Bad frac " << f << std::endl;
            if (f<0.0)
               std::cout << "----->  Bad frac " << f << std::endl;
#endif
            frac_y = f;
            i_ax = i;
            if (i == Y_AXIS_LOW)
               frac_x = 0.0;
            else
               frac_x = 1.0;
#ifdef HAVE_IOSTREAM_HEADER
            if (i_ax != Y_AXIS_LOW)
               if (i_ax != Y_AXIS_HIGH)
                  std::cout << "Bad axis to coordinates(i, f) i: "
                            << i << "  f: " << f << std::endl;
#endif
         }
         double get_frac_x() { return frac_x; }
         double get_frac_y() { return frac_y; }
      };

      typedef std::pair<coordinates, coordinates> cp_t;

   private:

      std::vector<cp_t> coords;

   public:

      grid_index_t grid_index_next;
      lig_build::pos_t start_point; // on either the x or y axis
      lig_build::pos_t end_point;
      contour_fragment(int ms_type,
                       const float &contour_level,
                       double v00, double v01, double v10, double v11);

      std::pair<double, double> get_coords(int ii, int jj, int coord_indx) const;
      unsigned int coords_size() const { return coords.size(); }

   };

   std::vector<residue_circle_t> residue_circles;

   // a set of handles (returned from
   // additional_representation_by_attributes()) that correspond to
   // the residues in residue_circles.  If there are no additional
   // representations, then an empty vector is handled.
   std::vector<int> additional_representation_handles;
   svg_container_t draw_stacking_interactions(const std::vector<residue_circle_t> &v);
   // click_pos is where we recentre in 3D graphics when the annotation
   // (line) is clicked.
   svg_container_t
   draw_annotated_stacking_line(const lig_build::pos_t &ligand_ring_centre,
                                const lig_build::pos_t &residue_pos,
                                int stacking_type,
                                const clipper::Coord_orth &click_pos);
   svg_container_t draw_all_flev_residue_attribs();
   svg_container_t draw_all_flev_ligand_annotations();
   svg_container_t draw_residue_circles(const std::vector<residue_circle_t> &v,
                                        const std::vector<int> &add_rep_handles);

   svg_container_t draw_residue_circle_top_layer(const residue_circle_t &residue_circle,
                                                 const lig_build::pos_t &ligand_centre,
                                                 int add_rep_handle);

   svg_container_t draw_solvent_accessibility_of_atom(const lig_build::pos_t &pos, double sa);
   svg_container_t draw_solvent_accessibility_of_atoms();

   svg_container_t draw_substitution_contour();
   svg_container_t draw_bonds_to_ligand();
   svg_container_t draw_solvent_exposure_circle(const residue_circle_t &residue_circle,
                                                const lig_build::pos_t &ligand_centre);
   std::string get_residue_solvent_exposure_fill_colour(double radius_extra) const;
   // top left and bottom right corners.
   //
   std::pair<lig_build::pos_t, lig_build::pos_t> flev_residues_extents() const;

   std::vector<std::vector<std::string> > ring_atoms_list;

   std::vector<pli::solvent_accessible_atom_t>
   convert(const std::vector<std::pair<coot::atom_spec_t, float> > &s_a_v,
           const pli::flev_attached_hydrogens_t &ah) const;

   std::pair<std::string, std::string> get_residue_circle_colour(const std::string &residue_type) const;

   std::string svg;
   void render(); // add stuff to the svg
   double standard_residue_circle_radius;

   void refine_residue_circle_positions(); // changes the positions of residue_circles
   std::vector<int> get_primary_indices() const;
   void initial_residues_circles_layout();

   // fiddle with the position of the residue_circles[primary_index].
   //
   void initial_primary_residue_circles_layout(const ligand_grid &grid,
                                               int primary_index,
                                               const std::vector<std::pair<lig_build::pos_t, double> > &attachment_points);

   // untrap residues as needed.
   void position_non_primaries(const ligand_grid &grid,
                               const std::vector<int> &primary_indices);

   void numerical_gradients(gsl_vector *x, gsl_vector *df, void *params) const;

   // return first true on solution having problems, false for no problems, also
   // return a list of the residue circles with problems.
   //
   std::pair<bool, std::vector<int> > solution_has_problems_p() const;

   // check that the residues with bonds to the ligand have sane distances to the
   // bonding ligand atom
   std::pair<bool, std::vector<int> > solution_has_bonded_problems_p() const;

   void reposition_bonded_problematics_and_reoptimise(const std::vector<int> &problematics,
                                                      const std::vector<int> &primary_indices);

   // the other residues, not bonded to the ligand.
   //
   // This is (currently) not a clever repositioning!
   void reposition_problematics_and_reoptimise(const std::vector<int> &problematics,
                                               const std::vector<int> &primary_indices);

   // minimise layout energy
   std::pair<int, std::vector<residue_circle_t> >
   optimise_residue_circle_positions(const std::vector<residue_circle_t> &start,
                                     const std::vector<residue_circle_t> &current,
                                     const std::vector<int> &primary_indices) const;

   std::pair<bool,lig_build::pos_t> get_residue_circles_top_left() const;
   std::pair<int, std::vector<residue_circle_t> > solution() const;

public:
   flev_t() { init(); }
   svg_molecule_t mol; // does it help to make this private?
   double scale_factor;
   float fle_water_dist_max;
   float fle_h_bond_dist_max;
   void init() {
      fle_water_dist_max = 3.25;
      fle_h_bond_dist_max = 3.9;
      standard_residue_circle_radius = 1.2; // 20250118-PE was 15.45; c.f. r in draw_residue_circle_top_layer()
      scale_factor = 400.0; // used in svg_molecule
   }


   svg_container_t draw_all_flev_annotations();
   void write_png(const std::string &file_name) const;
   void write_svg(const std::string &file_name) const;

   bool annotate(const std::vector<std::pair<coot::atom_spec_t, float> > &s_a_v,
                 const std::vector<pli::fle_residues_helper_t> &centres,
                 const std::vector<int> &additional_representation_handles_in,
                 const std::vector<pli::fle_ligand_bond_t> &bonds_to_ligand,
                 const std::vector<pli::solvent_exposure_difference_helper_t> &sed,
                 const pli::flev_attached_hydrogens_t &ah,
                 const pli::pi_stacking_container_t &pi_stack_info,
                 const coot::dictionary_residue_restraints_t &restraints);
};

// maybe pli functions need their own header - otherwise some confusion?
// pli-utils.hh

namespace pli {

   // the src directory already has a function of this name. So here we will put it in the pli namespace

   svg_container_t fle_view_with_rdkit_internal(mmdb::Manager *mol,
                                                int imol, // for looking up ligands in the dictionary
                                                coot::protein_geometry *geom_p,
                                                const std::string &chain_id, int res_no, const std::string &ins_code,
                                                float residues_near_radius,
                                                bool add_key);

   svg_container_t make_key(const lig_build::pos_t &pos);
   std::string make_circle(const lig_build::pos_t &pos, double radius, double stroke_width,
                           const std::string &fill_color, const std::string &stroke_color);

   svg_container_t draw_solvent_accessibility_of_atom(const lig_build::pos_t &pos, double sa);

   std::vector<fle_residues_helper_t>
   get_flev_residue_centres(mmdb::Residue *residue_ligand_3d,
                            mmdb::Manager *mol_containing_residue_ligand,
                            std::vector<mmdb::Residue *> residues,
                            mmdb::Manager *flat_mol);

   std::map<std::string, std::string> make_flat_ligand_name_map(mmdb::Residue *flat_res);

}


#endif // COOT_PLI_FLEV_HH
