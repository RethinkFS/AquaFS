//
// Created by chiro on 23-5-27.
//

#include "adapter.h"

#include <memory>

#include "plugin/AquaFS/AquaZFS/fs/fs_aquafs.h"
#include "plugin/AquaFS/AquaZFS/fs/zbd_aquafs.h"
#include "rocksdb/utilities/object_registry.h"

namespace aquafs {
#define CAST_FROM_AQUA(x)                      \
  inline rocksdb::x fromAqua(aquafs::x from) { \
    return static_cast<rocksdb::x>(from);      \
  }
#define CAST_FROM_AQUA_CLONE(x)                       \
  inline rocksdb::x fromAqua(const aquafs::x &from) { \
    return static_cast<rocksdb::x>(from);             \
  }
#define CAST_FROM_AQUA_MOVE(x)                         \
  inline rocksdb::x fromAqua(const aquafs::x &&from) { \
    return static_cast<rocksdb::x>(from);              \
  }

CAST_FROM_AQUA_MOVE(IOStatus::Code)
CAST_FROM_AQUA_MOVE(IOStatus::SubCode)
CAST_FROM_AQUA_MOVE(Status::Severity)
CAST_FROM_AQUA_CLONE(Temperature)

inline rocksdb::Slice fromAqua(aquafs::Slice &&from) {
  return rocksdb::Slice(from.data(), from.size());
}
inline rocksdb::IOStatus fromAqua(aquafs::IOStatus &&from) {
  return {fromAqua(from.code()),     fromAqua(from.subcode()),
          fromAqua(from.severity()), from.retryable_,
          from.data_loss_,           from.scope_,
          std::move(from.state_)};
}
inline aquafs::IOOptions toAqua(const rocksdb::IOOptions &from) {
  return aquafs::IOOptions(from.force_dir_fsync);
}
#define CAST_TO_AQUA(x)                            \
  inline aquafs::x toAqua(const rocksdb::x from) { \
    return static_cast<aquafs::x>(from);           \
  }
#define CAST_TO_AQUA_CLONE(x)                       \
  inline aquafs::x toAqua(const rocksdb::x &from) { \
    return static_cast<aquafs::x>(from);            \
  }
#define CAST_TO_AQUA_MOVE(x)                         \
  inline aquafs::x toAqua(const rocksdb::x &&from) { \
    return static_cast<aquafs::x>(from);             \
  }

CAST_TO_AQUA_CLONE(Temperature)
CAST_TO_AQUA_CLONE(ChecksumType)

inline aquafs::FileOptions toAqua(const rocksdb::FileOptions &from) {
  aquafs::FileOptions r;
  r.io_options = toAqua(from.io_options);
  r.temperature = toAqua(from.temperature);
  r.handoff_checksum_type = toAqua(from.handoff_checksum_type);
  return r;
}

AquaFSFileSystemWrapper::AquaFSFileSystemWrapper(
    std::unique_ptr<aquafs::FileSystem> inner,
    std::shared_ptr<rocksdb::FileSystem> aux_fs)
    : FileSystemWrapper(aux_fs), inner_(std::move(inner)) {}

AquaFSFileSystemWrapper::~AquaFSFileSystemWrapper() {}

class FSSequentialFileAdapter : public rocksdb::FSSequentialFile {
  std::unique_ptr<aquafs::FSSequentialFile> inner;

 public:
  explicit FSSequentialFileAdapter(
      std::unique_ptr<aquafs::FSSequentialFile> &&inner_)
      : inner(std::move(inner_)) {}
  rocksdb::IOStatus Read(size_t n, const rocksdb::IOOptions &options,
                         rocksdb::Slice *result, char *scratch,
                         rocksdb::IODebugContext *dbg) override {
    aquafs::Slice data;
    auto ret =
        fromAqua(inner->Read(n, toAqua(options), &data, scratch, nullptr));
    *result = fromAqua(std::move(data));
    return ret;
  }
  rocksdb::IOStatus Skip(uint64_t n) override {
    return fromAqua(inner->Skip(n));
  }
  bool use_direct_io() const override { return inner->use_direct_io(); }
  size_t GetRequiredBufferAlignment() const override {
    return inner->GetRequiredBufferAlignment();
  }
  rocksdb::IOStatus InvalidateCache(size_t size, size_t size1) override {
    return fromAqua(inner->InvalidateCache(size, size1));
  }
  rocksdb::IOStatus PositionedRead(uint64_t uint64, size_t size,
                                   const rocksdb::IOOptions &options,
                                   rocksdb::Slice *slice, char *string,
                                   rocksdb::IODebugContext *context) override {
    aquafs::IOOptions options_ = toAqua(options);
    aquafs::Slice data;
    auto ret = fromAqua(
        inner->PositionedRead(uint64, size, options_, &data, string, nullptr));
    *slice = fromAqua(std::move(data));
    return ret;
  }
  rocksdb::Temperature GetTemperature() const override {
    return fromAqua(inner->GetTemperature());
  }
};

rocksdb::IOStatus AquaFSFileSystemWrapper::NewSequentialFile(
    const std::string &f, const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSSequentialFile> *r,
    rocksdb::IODebugContext *dbg) {
  // return FileSystemWrapper::NewSequentialFile(f, file_opts, r, dbg);
  std::unique_ptr<aquafs::FSSequentialFile> result;
  auto ret = fromAqua(
      inner_->NewSequentialFile(f, toAqua(file_opts), &result, nullptr));
  *r = std::make_unique<FSSequentialFileAdapter>(std::move(result));
  return ret;
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NewRandomAccessFile(
    const std::string &f, const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSRandomAccessFile> *r,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::NewRandomAccessFile(f, file_opts, r, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NewWritableFile(
    const std::string &f, const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSWritableFile> *r, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::NewWritableFile(f, file_opts, r, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::ReopenWritableFile(
    const std::string &fname, const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSWritableFile> *result,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::ReopenWritableFile(fname, file_opts, result, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::ReuseWritableFile(
    const std::string &fname, const std::string &old_fname,
    const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSWritableFile> *r, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::ReuseWritableFile(fname, old_fname, file_opts, r,
                                              dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NewRandomRWFile(
    const std::string &fname, const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSRandomRWFile> *result,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::NewRandomRWFile(fname, file_opts, result, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NewMemoryMappedFileBuffer(
    const std::string &fname,
    std::unique_ptr<rocksdb::MemoryMappedFileBuffer> *result) {
  return FileSystemWrapper::NewMemoryMappedFileBuffer(fname, result);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NewDirectory(
    const std::string &name, const rocksdb::IOOptions &io_opts,
    std::unique_ptr<rocksdb::FSDirectory> *result,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::NewDirectory(name, io_opts, result, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::FileExists(
    const std::string &f, const rocksdb::IOOptions &io_opts,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::FileExists(f, io_opts, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetChildren(
    const std::string &dir, const rocksdb::IOOptions &io_opts,
    std::vector<std::string> *r, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::GetChildren(dir, io_opts, r, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetChildrenFileAttributes(
    const std::string &dir, const rocksdb::IOOptions &options,
    std::vector<rocksdb::FileAttributes> *result,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::GetChildrenFileAttributes(dir, options, result,
                                                      dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::DeleteFile(
    const std::string &f, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::DeleteFile(f, options, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::Truncate(
    const std::string &fname, size_t size, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::Truncate(fname, size, options, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::CreateDir(
    const std::string &d, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::CreateDir(d, options, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::CreateDirIfMissing(
    const std::string &d, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::CreateDirIfMissing(d, options, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::DeleteDir(
    const std::string &d, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::DeleteDir(d, options, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetFileSize(
    const std::string &f, const rocksdb::IOOptions &options, uint64_t *s,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::GetFileSize(f, options, s, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetFileModificationTime(
    const std::string &fname, const rocksdb::IOOptions &options,
    uint64_t *file_mtime, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::GetFileModificationTime(fname, options, file_mtime,
                                                    dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetAbsolutePath(
    const std::string &db_path, const rocksdb::IOOptions &options,
    std::string *output_path, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::GetAbsolutePath(db_path, options, output_path, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::RenameFile(
    const std::string &s, const std::string &t,
    const rocksdb::IOOptions &options, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::RenameFile(s, t, options, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::LinkFile(
    const std::string &s, const std::string &t,
    const rocksdb::IOOptions &options, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::LinkFile(s, t, options, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NumFileLinks(
    const std::string &fname, const rocksdb::IOOptions &options,
    uint64_t *count, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::NumFileLinks(fname, options, count, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::AreFilesSame(
    const std::string &first, const std::string &second,
    const rocksdb::IOOptions &options, bool *res,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::AreFilesSame(first, second, options, res, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::LockFile(
    const std::string &f, const rocksdb::IOOptions &options,
    rocksdb::FileLock **l, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::LockFile(f, options, l, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::UnlockFile(
    rocksdb::FileLock *l, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::UnlockFile(l, options, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetTestDirectory(
    const rocksdb::IOOptions &options, std::string *path,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::GetTestDirectory(options, path, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NewLogger(
    const std::string &fname, const rocksdb::IOOptions &options,
    std::shared_ptr<rocksdb::Logger> *result, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::NewLogger(fname, options, result, dbg);
}
void AquaFSFileSystemWrapper::SanitizeFileOptions(
    rocksdb::FileOptions *opts) const {
  FileSystemWrapper::SanitizeFileOptions(opts);
}
rocksdb::FileOptions AquaFSFileSystemWrapper::OptimizeForLogRead(
    const rocksdb::FileOptions &file_options) const {
  return FileSystemWrapper::OptimizeForLogRead(file_options);
}
rocksdb::FileOptions AquaFSFileSystemWrapper::OptimizeForManifestRead(
    const rocksdb::FileOptions &file_options) const {
  return FileSystemWrapper::OptimizeForManifestRead(file_options);
}
rocksdb::FileOptions AquaFSFileSystemWrapper::OptimizeForLogWrite(
    const rocksdb::FileOptions &file_options,
    const rocksdb::DBOptions &db_options) const {
  return FileSystemWrapper::OptimizeForLogWrite(file_options, db_options);
}
rocksdb::FileOptions AquaFSFileSystemWrapper::OptimizeForManifestWrite(
    const rocksdb::FileOptions &file_options) const {
  return FileSystemWrapper::OptimizeForManifestWrite(file_options);
}
rocksdb::FileOptions AquaFSFileSystemWrapper::OptimizeForCompactionTableWrite(
    const rocksdb::FileOptions &file_options,
    const rocksdb::ImmutableDBOptions &immutable_ops) const {
  return FileSystemWrapper::OptimizeForCompactionTableWrite(file_options,
                                                            immutable_ops);
}
rocksdb::FileOptions AquaFSFileSystemWrapper::OptimizeForCompactionTableRead(
    const rocksdb::FileOptions &file_options,
    const rocksdb::ImmutableDBOptions &db_options) const {
  return FileSystemWrapper::OptimizeForCompactionTableRead(file_options,
                                                           db_options);
}
rocksdb::FileOptions AquaFSFileSystemWrapper::OptimizeForBlobFileRead(
    const rocksdb::FileOptions &file_options,
    const rocksdb::ImmutableDBOptions &db_options) const {
  return FileSystemWrapper::OptimizeForBlobFileRead(file_options, db_options);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetFreeSpace(
    const std::string &path, const rocksdb::IOOptions &options,
    uint64_t *diskfree, rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::GetFreeSpace(path, options, diskfree, dbg);
}
rocksdb::IOStatus AquaFSFileSystemWrapper::IsDirectory(
    const std::string &path, const rocksdb::IOOptions &options, bool *is_dir,
    rocksdb::IODebugContext *dbg) {
  return FileSystemWrapper::IsDirectory(path, options, is_dir, dbg);
}
const char *AquaFSFileSystemWrapper::Name() const {
  return "AquaFSFileSystemWrapper";
}

extern "C" rocksdb::FactoryFunc<rocksdb::FileSystem> aquafs_filesystem_reg;

rocksdb::FactoryFunc<rocksdb::FileSystem> aquafs_filesystem_reg =
    rocksdb::ObjectLibrary::Default()->AddFactory<rocksdb::FileSystem>(
        rocksdb::ObjectLibrary::PatternEntry("aquafs", false)
            .AddSeparator("://"), /* "aquafs://.+" */
        [](const std::string &uri, std::unique_ptr<rocksdb::FileSystem> *f,
           std::string *errmsg) {
          std::string devID = uri;
          FileSystem *fs = nullptr;
          Status s;

          devID.replace(0, strlen("aquafs://"), "");
          if (devID.rfind("dev:") == 0) {
            devID.replace(0, strlen("dev:"), "");
#ifdef AQUAFS_EXPORT_PROMETHEUS
            s = NewAquaFS(&fs, ZbdBackendType::kBlockDev, devID,
                          std::make_shared<AquaFSPrometheusMetrics>());
#else
            s = NewAquaFS(&fs, ZbdBackendType::kBlockDev, devID);
#endif
            if (!s.ok()) {
              *errmsg = s.ToString();
            }
          } else if (devID.find("raid") == 0) {
#ifdef AQUAFS_EXPORT_PROMETHEUS
            s = NewAquaFS(&fs, ZbdBackendType::kRaid, devID,
                          std::make_shared<AquaFSPrometheusMetrics>());
#else
            s = NewAquaFS(&fs, ZbdBackendType::kRaid, devID);
#endif
            if (!s.ok()) {
              *errmsg = s.ToString();
            }
          } else if (devID.rfind("uuid:") == 0) {
            std::map<std::string, std::pair<std::string, ZbdBackendType>>
                aquaFileSystems;
            s = ListAquaFileSystems(aquaFileSystems);
            if (!s.ok()) {
              *errmsg = s.ToString();
            } else {
              devID.replace(0, strlen("uuid:"), "");

              if (aquaFileSystems.find(devID) == aquaFileSystems.end()) {
                *errmsg = "UUID not found";
              } else {

#ifdef AQUAFS_EXPORT_PROMETHEUS
                s = NewAquaFS(&fs, aquaFileSystems[devID].second,
                              aquaFileSystems[devID].first,
                              std::make_shared<AquaFSPrometheusMetrics>());
#else
                s = NewAquaFS(&fs, aquaFileSystems[devID].second,
                              aquaFileSystems[devID].first);
#endif
                if (!s.ok()) {
                  *errmsg = s.ToString();
                }
              }
            }
          } else if (devID.rfind("zonefs:") == 0) {
            devID.replace(0, strlen("zonefs:"), "");
            s = NewAquaFS(&fs, ZbdBackendType::kZoneFS, devID);
            if (!s.ok()) {
              *errmsg = s.ToString();
            }
          } else {
            *errmsg = "Malformed URI";
          }
          auto wrapper = new AquaFSFileSystemWrapper(
              std::unique_ptr<aquafs::FileSystem>(fs), nullptr);
          f->reset(wrapper);
          return f->get();
        });

}  // namespace aquafs
