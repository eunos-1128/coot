/* coot-utils/slurp-map.hh
 *
 * Copyright 2019 by Medical Research Council
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

#ifndef COOT_UTILS_SLURP_MAP_HH
#define COOT_UTILS_SLURP_MAP_HH

#include <sys/stat.h>

#include <string>

#include <clipper/ccp4/ccp4_map_io.h>
#include "utils/ctpl.h"

namespace coot {
   namespace util {

      // 20250531-PE this is how to do enum class variables

      enum class slurp_map_result_t { OK=1,
                                      IS_SLURPABLE_EM_MAP=100,
                                      IS_NON_SLURPABLE_EM_MAP=600,
                                      NOT_AN_EM_MAP=200,
                                      FAIL=300,
                                      FILE_NOT_FOUND=400,
                                      UNRESOLVED=500 };
      std::string to_string(slurp_map_result_t r);
      std::ostream& operator<<(std::ostream& os, slurp_map_result_t smr);

      std::vector<std::string> get_map_labels(const std::string &file_name);

      // PANDDA::, or that is.
      bool map_labels_contain_PANDDA(const std::string &file_name);

      slurp_map_result_t is_basic_em_map_file(const std::string &file_name);

      // inf check_only is true, then just read the header, check that it is sane
      // and return that status (don't touch the xmap). Otherwise, fill the xmap.
      //
      slurp_map_result_t slurp_fill_xmap_from_map_file(const std::string &file_name,
                                                       clipper::Xmap<float> *xmap_p,
                                                       bool check_only=false);

      slurp_map_result_t slurp_parse_xmap_data(char *data, clipper::Xmap<float> *xmap_p,
                                               bool check_only=false);

   }

}

#endif // COOT_UTILS_SLURP_MAP_HH

