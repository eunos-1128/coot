/*
 * src/cc-interface-graphics.cc
 *
 * Copyright 2021 by Medical Research Council
 * Author: Paul Emsley
 *
 * This file is part of Coot
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
 * You should have received a copies of the GNU General Public License and
 * the GNU Lesser General Public License along with this program; if not,
 * write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA, 02110-1301, USA.
 * See http://www.gnu.org/licenses/
 *
 */

#include "cc-interface.hh"

#include "widget-from-builder.hh"

#include "graphics-info.h"

// 20211019-PE these have moved into graphics_info_t now because I want to add a
// key-binding "Esc" to do an unfullscreen()

void fullscreen() {
   graphics_info_t::fullscreen();
}

void unfullscreen() {
   graphics_info_t::unfullscreen();

}

void set_use_trackpad(short int state) {
   set_use_primary_mouse_button_for_view_rotation(state);
}

//! this is an alias for the above (at the moment).
void set_use_primary_mouse_button_for_view_rotation(short int state) {
   graphics_info_t::use_primary_mouse_for_view_rotation_flag = state;
}


// maybe this function should have its own file?
//
//! \brief display the SMILES entry. This is the simple version - no dictionary
//! is generated.
void do_smiles_to_simple_3d_overlay_frame() {

   GtkWidget *frame = widget_from_builder("smiles_to_simple_3d_frame");
   if (frame)
      gtk_widget_set_visible(frame, TRUE);

}

void show_coot_points_frame() {

   auto coot_points_frame_callback = +[] (gpointer user_data) {
      GtkWidget *frame = widget_from_builder("coot-points-frame");
      if (frame) {
         gtk_widget_set_visible(frame, FALSE);
      }
      return FALSE;
   };

   GtkWidget *frame = widget_from_builder("coot-points-frame");
   if (frame) {
      gtk_widget_set_visible(frame, TRUE);
      GSourceFunc cb = G_SOURCE_FUNC(coot_points_frame_callback);
      g_timeout_add(4000, cb, nullptr);
   }

}

#include "coot-utils/positron.hh"

void
read_positron_metadata(const std::string &z_data, const std::string &s_data) {

   coot::read_positron_metadata(&graphics_info_t::positron_metadata, z_data, s_data);
}

//! \brief add to non-drawn bonds - using atom selection cid
void add_to_non_drawn_bonds(int imol, const std::string &cid) {

   graphics_info_t g;
   if (g.is_valid_model_molecule(imol)) {
      graphics_info_t::molecules[imol].add_to_non_drawn_bonds(cid);
   }
   g.graphics_draw();
}

void set_new_non_drawn_bonds(int imol, const std::string &cid) {

   graphics_info_t g;
   if (g.is_valid_model_molecule(imol)) {
      graphics_info_t::molecules[imol].set_new_non_drawn_bonds(cid);
   }
   g.graphics_draw();
}

//! \brief clear the non-drawn bonds
void clear_non_drawn_bonds(int imol) {

    graphics_info_t g;
    if (g.is_valid_model_molecule(imol))
       g.molecules[imol].clear_non_drawn_bonds(true);
    g.graphics_draw();
}

