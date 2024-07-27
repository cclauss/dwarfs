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

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include <dwarfs/metadata_types.h>

namespace dwarfs {

class logger;

struct metadata_options;
struct filesystem_info;
struct file_stat;
struct vfs_stat;

class performance_monitor;

namespace thrift::metadata {
class metadata;
}

namespace internal {

class metadata_v2 {
 public:
  metadata_v2() = default;
  metadata_v2(metadata_v2&&) = default;
  metadata_v2& operator=(metadata_v2&&) = default;

  metadata_v2(logger& lgr, std::span<uint8_t const> schema,
              std::span<uint8_t const> data, metadata_options const& options,
              int inode_offset = 0, bool force_consistency_check = false,
              std::shared_ptr<performance_monitor const> perfmon = nullptr);

  void check_consistency() const { impl_->check_consistency(); }

  void
  dump(std::ostream& os, int detail_level, filesystem_info const& fsinfo,
       std::function<void(const std::string&, uint32_t)> const& icb) const {
    impl_->dump(os, detail_level, fsinfo, icb);
  }

  nlohmann::json
  info_as_json(int detail_level, filesystem_info const& fsinfo) const {
    return impl_->info_as_json(detail_level, fsinfo);
  }

  nlohmann::json as_json() const { return impl_->as_json(); }

  std::string serialize_as_json(bool simple) const {
    return impl_->serialize_as_json(simple);
  }

  size_t size() const { return impl_->size(); }

  void walk(std::function<void(dir_entry_view)> const& func) const {
    impl_->walk(func);
  }

  void walk_data_order(std::function<void(dir_entry_view)> const& func) const {
    impl_->walk_data_order(func);
  }

  std::optional<inode_view> find(const char* path) const {
    return impl_->find(path);
  }

  std::optional<inode_view> find(int inode) const { return impl_->find(inode); }

  std::optional<inode_view> find(int inode, const char* name) const {
    return impl_->find(inode, name);
  }

  int getattr(inode_view iv, file_stat* stbuf) const {
    return impl_->getattr(iv, stbuf);
  }

  std::optional<directory_view> opendir(inode_view iv) const {
    return impl_->opendir(iv);
  }

  std::optional<std::pair<inode_view, std::string>>
  readdir(directory_view dir, size_t offset) const {
    return impl_->readdir(dir, offset);
  }

  size_t dirsize(directory_view dir) const { return impl_->dirsize(dir); }

  int access(inode_view iv, int mode, uid_t uid, gid_t gid) const {
    return impl_->access(iv, mode, uid, gid);
  }

  int open(inode_view iv) const { return impl_->open(iv); }

  int readlink(inode_view iv, std::string* buf, readlink_mode mode) const {
    return impl_->readlink(iv, buf, mode);
  }

  int statvfs(vfs_stat* stbuf) const { return impl_->statvfs(stbuf); }

  std::optional<chunk_range> get_chunks(int inode) const {
    return impl_->get_chunks(inode);
  }

  size_t block_size() const { return impl_->block_size(); }

  bool has_symlinks() const { return impl_->has_symlinks(); }

  nlohmann::json get_inode_info(inode_view iv) const {
    return impl_->get_inode_info(iv);
  }

  std::optional<std::string> get_block_category(size_t block_number) const {
    return impl_->get_block_category(block_number);
  }

  std::vector<std::string> get_all_block_categories() const {
    return impl_->get_all_block_categories();
  }

  std::vector<file_stat::uid_type> get_all_uids() const {
    return impl_->get_all_uids();
  }

  std::vector<file_stat::gid_type> get_all_gids() const {
    return impl_->get_all_gids();
  }

  class impl {
   public:
    virtual ~impl() = default;

    virtual void check_consistency() const = 0;

    virtual void dump(
        std::ostream& os, int detail_level, filesystem_info const& fsinfo,
        std::function<void(const std::string&, uint32_t)> const& icb) const = 0;

    virtual nlohmann::json
    info_as_json(int detail_level, filesystem_info const& fsinfo) const = 0;

    virtual nlohmann::json as_json() const = 0;
    virtual std::string serialize_as_json(bool simple) const = 0;

    virtual size_t size() const = 0;

    virtual void
    walk(std::function<void(dir_entry_view)> const& func) const = 0;

    virtual void
    walk_data_order(std::function<void(dir_entry_view)> const& func) const = 0;

    virtual std::optional<inode_view> find(const char* path) const = 0;
    virtual std::optional<inode_view> find(int inode) const = 0;
    virtual std::optional<inode_view>
    find(int inode, const char* name) const = 0;

    virtual int getattr(inode_view iv, file_stat* stbuf) const = 0;

    virtual std::optional<directory_view> opendir(inode_view iv) const = 0;

    virtual std::optional<std::pair<inode_view, std::string>>
    readdir(directory_view dir, size_t offset) const = 0;

    virtual size_t dirsize(directory_view dir) const = 0;

    virtual int access(inode_view iv, int mode, uid_t uid, gid_t gid) const = 0;

    virtual int open(inode_view iv) const = 0;

    virtual int
    readlink(inode_view iv, std::string* buf, readlink_mode mode) const = 0;

    virtual int statvfs(vfs_stat* stbuf) const = 0;

    virtual std::optional<chunk_range> get_chunks(int inode) const = 0;

    virtual size_t block_size() const = 0;

    virtual bool has_symlinks() const = 0;

    virtual nlohmann::json get_inode_info(inode_view iv) const = 0;

    virtual std::optional<std::string>
    get_block_category(size_t block_number) const = 0;

    virtual std::vector<std::string> get_all_block_categories() const = 0;

    virtual std::vector<file_stat::uid_type> get_all_uids() const = 0;

    virtual std::vector<file_stat::gid_type> get_all_gids() const = 0;
  };

 private:
  std::unique_ptr<impl> impl_;
};

} // namespace internal
} // namespace dwarfs