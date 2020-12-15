/* vim:set ts=2 sw=2 sts=2 et: */
/**
 * \author     Marcus Holland-Moritz (github@mhxnet.de)
 * \copyright  Copyright (c) Marcus Holland-Moritz
 *
 * This file is part of dwarfs.
 *
 * dwarfs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dwarfs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dwarfs.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <boost/program_options.hpp>

#include <folly/Conv.h>
#include <folly/String.h>

#include "dwarfs/filesystem_v2.h"
#include "dwarfs/fstypes.h"
#include "dwarfs/logger.h"
#include "dwarfs/mmap.h"
#include "dwarfs/options.h"
#include "dwarfs/util.h"
#include "dwarfs/worker_group.h"

namespace po = boost::program_options;

using namespace dwarfs;

namespace {

int dwarfsbench(int argc, char** argv) {
  std::string filesystem, cache_size_str, lock_mode_str, decompress_ratio_str,
      log_level;
  size_t num_workers;
  size_t num_readers;

  // clang-format off
  po::options_description opts("Command line options");
  opts.add_options()
    ("filesystem,f",
        po::value<std::string>(&filesystem),
        "path to filesystem")
    ("num-workers,n",
        po::value<size_t>(&num_workers)->default_value(1),
        "number of worker threads")
    ("num-readers,N",
        po::value<size_t>(&num_readers)->default_value(1),
        "number of reader threads")
    ("cache-size,s",
        po::value<std::string>(&cache_size_str)->default_value("256m"),
        "block cache size")
    ("lock-mode,m",
        po::value<std::string>(&lock_mode_str)->default_value("none"),
        "mlock mode (none, try, must)")
    ("decompress-ratio,r",
        po::value<std::string>(&decompress_ratio_str)->default_value("0.8"),
        "block cache size")
    ("log-level,l",
        po::value<std::string>(&log_level)->default_value("info"),
        "log level (error, warn, info, debug, trace)")
    ("help,h",
        "output help message and exit");
  // clang-format on

  po::variables_map vm;

  po::store(po::parse_command_line(argc, argv, opts), vm);
  po::notify(vm);

  if (vm.count("help") or !vm.count("filesystem")) {
    std::cout << "dwarfsbench (" << DWARFS_VERSION << ")\n\n"
              << opts << std::endl;
    return 0;
  }

  stream_logger lgr(std::cerr, logger::parse_level(log_level));
  filesystem_options fsopts;

  fsopts.lock_mode = parse_mlock_mode(lock_mode_str);
  fsopts.block_cache.max_bytes = parse_size_with_unit(cache_size_str);
  fsopts.block_cache.num_workers = num_workers;
  fsopts.block_cache.decompress_ratio = folly::to<double>(decompress_ratio_str);

  dwarfs::filesystem_v2 fs(lgr, std::make_shared<dwarfs::mmap>(filesystem),
                           fsopts);

  worker_group wg("reader", num_readers);

  fs.walk([&](auto entry) {
    if (S_ISREG(entry.mode())) {
      wg.add_job([&fs, entry] {
        struct ::stat stbuf;
        if (fs.getattr(entry, &stbuf) == 0) {
          std::vector<char> buf(stbuf.st_size);
          int fh = fs.open(entry);
          fs.read(fh, buf.data(), buf.size());
        }
      });
    }
  });

  wg.wait();

  return 0;
}
} // namespace

int main(int argc, char** argv) {
  try {
    return dwarfsbench(argc, argv);
  } catch (std::exception const& e) {
    std::cerr << "ERROR: " << folly::exceptionStr(e) << std::endl;
    return 1;
  }
}
