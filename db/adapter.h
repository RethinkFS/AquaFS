//
// Created by chiro on 23-5-27.
//

#ifndef AQUAFS_ADAPTER_H
#define AQUAFS_ADAPTER_H

#include "plugin/AquaFS/AquaZFS/base/file_system.h"
#include "rocksdb/file_system.h"
#include "rocksdb/rocksdb_namespace.h"

namespace aquafs {

class AquaFSFileSystemWrapper : public ROCKSDB_NAMESPACE::FileSystemWrapper {
 public:
  std::unique_ptr<aquafs::FileSystem> inner_;
  explicit AquaFSFileSystemWrapper(std::unique_ptr<aquafs::FileSystem> inner,
                                   std::shared_ptr<FileSystem> aux_fs);
  ~AquaFSFileSystemWrapper() override;
  const char *Name() const override;
  rocksdb::IOStatus NewSequentialFile(
      const std::string &f, const rocksdb::FileOptions &file_opts,
      std::unique_ptr<rocksdb::FSSequentialFile> *r,
      rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus NewRandomAccessFile(
      const std::string &f, const rocksdb::FileOptions &file_opts,
      std::unique_ptr<rocksdb::FSRandomAccessFile> *r,
      rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus NewWritableFile(const std::string &f,
                                    const rocksdb::FileOptions &file_opts,
                                    std::unique_ptr<rocksdb::FSWritableFile> *r,
                                    rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus ReopenWritableFile(
      const std::string &fname, const rocksdb::FileOptions &file_opts,
      std::unique_ptr<rocksdb::FSWritableFile> *result,
      rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus ReuseWritableFile(
      const std::string &fname, const std::string &old_fname,
      const rocksdb::FileOptions &file_opts,
      std::unique_ptr<rocksdb::FSWritableFile> *r,
      rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus NewRandomRWFile(
      const std::string &fname, const rocksdb::FileOptions &file_opts,
      std::unique_ptr<rocksdb::FSRandomRWFile> *result,
      rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus NewMemoryMappedFileBuffer(
      const std::string &fname,
      std::unique_ptr<rocksdb::MemoryMappedFileBuffer> *result) override;
  rocksdb::IOStatus NewDirectory(const std::string &name,
                                 const rocksdb::IOOptions &io_opts,
                                 std::unique_ptr<rocksdb::FSDirectory> *result,
                                 rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus FileExists(const std::string &f,
                               const rocksdb::IOOptions &io_opts,
                               rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus GetChildren(const std::string &dir,
                                const rocksdb::IOOptions &io_opts,
                                std::vector<std::string> *r,
                                rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus GetChildrenFileAttributes(
      const std::string &dir, const rocksdb::IOOptions &options,
      std::vector<rocksdb::FileAttributes> *result,
      rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus DeleteFile(const std::string &f,
                               const rocksdb::IOOptions &options,
                               rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus Truncate(const std::string &fname, size_t size,
                             const rocksdb::IOOptions &options,
                             rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus CreateDir(const std::string &d,
                              const rocksdb::IOOptions &options,
                              rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus CreateDirIfMissing(const std::string &d,
                                       const rocksdb::IOOptions &options,
                                       rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus DeleteDir(const std::string &d,
                              const rocksdb::IOOptions &options,
                              rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus GetFileSize(const std::string &f,
                                const rocksdb::IOOptions &options, uint64_t *s,
                                rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus GetFileModificationTime(
      const std::string &fname, const rocksdb::IOOptions &options,
      uint64_t *file_mtime, rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus GetAbsolutePath(const std::string &db_path,
                                    const rocksdb::IOOptions &options,
                                    std::string *output_path,
                                    rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus RenameFile(const std::string &s, const std::string &t,
                               const rocksdb::IOOptions &options,
                               rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus LinkFile(const std::string &s, const std::string &t,
                             const rocksdb::IOOptions &options,
                             rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus NumFileLinks(const std::string &fname,
                                 const rocksdb::IOOptions &options,
                                 uint64_t *count,
                                 rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus AreFilesSame(const std::string &first,
                                 const std::string &second,
                                 const rocksdb::IOOptions &options, bool *res,
                                 rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus LockFile(const std::string &f,
                             const rocksdb::IOOptions &options,
                             rocksdb::FileLock **l,
                             rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus UnlockFile(rocksdb::FileLock *l,
                               const rocksdb::IOOptions &options,
                               rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus GetTestDirectory(const rocksdb::IOOptions &options,
                                     std::string *path,
                                     rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus NewLogger(const std::string &fname,
                              const rocksdb::IOOptions &options,
                              std::shared_ptr<rocksdb::Logger> *result,
                              rocksdb::IODebugContext *dbg) override;
  void SanitizeFileOptions(rocksdb::FileOptions *opts) const override;
  rocksdb::FileOptions OptimizeForLogRead(
      const rocksdb::FileOptions &file_options) const override;
  rocksdb::FileOptions OptimizeForManifestRead(
      const rocksdb::FileOptions &file_options) const override;
  rocksdb::FileOptions OptimizeForLogWrite(
      const rocksdb::FileOptions &file_options,
      const rocksdb::DBOptions &db_options) const override;
  rocksdb::FileOptions OptimizeForManifestWrite(
      const rocksdb::FileOptions &file_options) const override;
  rocksdb::FileOptions OptimizeForCompactionTableWrite(
      const rocksdb::FileOptions &file_options,
      const rocksdb::ImmutableDBOptions &immutable_ops) const override;
  rocksdb::FileOptions OptimizeForCompactionTableRead(
      const rocksdb::FileOptions &file_options,
      const rocksdb::ImmutableDBOptions &db_options) const override;
  rocksdb::FileOptions OptimizeForBlobFileRead(
      const rocksdb::FileOptions &file_options,
      const rocksdb::ImmutableDBOptions &db_options) const override;
  rocksdb::IOStatus GetFreeSpace(const std::string &path,
                                 const rocksdb::IOOptions &options,
                                 uint64_t *diskfree,
                                 rocksdb::IODebugContext *dbg) override;
  rocksdb::IOStatus IsDirectory(const std::string &path,
                                const rocksdb::IOOptions &options, bool *is_dir,
                                rocksdb::IODebugContext *dbg) override;
};

}  // namespace aquafs

#endif  // AQUAFS_ADAPTER_H
