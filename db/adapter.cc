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
  return {from.data(), from.size()};
}
inline rocksdb::Env::IOPriority fromAqua(const aquafs::IOPriority &from) {
  return static_cast<rocksdb::Env::IOPriority>(from);
}
inline rocksdb::Env::WriteLifeTimeHint fromAqua(
    const aquafs::WriteLifeTimeHint &from) {
  return static_cast<rocksdb::Env::WriteLifeTimeHint>(from);
}
inline rocksdb::IOStatus fromAqua(aquafs::IOStatus &&from) {
  return {fromAqua(from.code()),     fromAqua(from.subcode()),
          fromAqua(from.severity()), from.retryable_,
          from.data_loss_,           from.scope_,
          std::move(from.state_)};
}
inline rocksdb::Status fromAqua(aquafs::Status &&from) {
  return rocksdb::Status(fromAqua(from.code()), fromAqua(from.subcode()),
                         fromAqua(from.severity()), from.retryable_,
                         from.data_loss_, from.scope_, std::move(from.state_));
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

inline aquafs::WriteLifeTimeHint toAqua(
    const rocksdb::Env::WriteLifeTimeHint &from) {
  return static_cast<aquafs::WriteLifeTimeHint>(from);
}
inline aquafs::IOPriority toAqua(const rocksdb::Env::IOPriority &from) {
  return static_cast<aquafs::IOPriority>(from);
}
inline aquafs::FileOptions toAqua(const rocksdb::FileOptions &from) {
  aquafs::FileOptions r;
  r.io_options = toAqua(from.io_options);
  r.temperature = toAqua(from.temperature);
  r.handoff_checksum_type = toAqua(from.handoff_checksum_type);
  return r;
}
inline aquafs::Slice toAqua(const rocksdb::Slice &from) {
  aquafs::Slice r;
  r.data_ = from.data_;
  r.size_ = from.size_;
  return r;
}

AquaFSFileSystemWrapper::AquaFSFileSystemWrapper(
    std::unique_ptr<aquafs::FileSystem> inner,
    std::shared_ptr<rocksdb::FileSystem> aux_fs)
    : FileSystemWrapper(aux_fs), inner_(std::move(inner)) {
  assert(inner_ != nullptr);
}

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

class FSRandomAccessFileAdapter : public rocksdb::FSRandomAccessFile {
  std::unique_ptr<aquafs::FSRandomAccessFile> inner;

 public:
  explicit FSRandomAccessFileAdapter(
      std::unique_ptr<aquafs::FSRandomAccessFile> &&inner_)
      : inner(std::move(inner_)) {}
  rocksdb::IOStatus Read(uint64_t offset, size_t n,
                         const rocksdb::IOOptions &options,
                         rocksdb::Slice *result, char *scratch,
                         rocksdb::IODebugContext *dbg) const override {
    aquafs::Slice data;
    auto ret = fromAqua(
        inner->Read(offset, n, toAqua(options), &data, scratch, nullptr));
    *result = fromAqua(std::move(data));
    return ret;
  }
  rocksdb::IOStatus Prefetch(uint64_t uint64, size_t size,
                             const rocksdb::IOOptions &options,
                             rocksdb::IODebugContext *context) override {
    // OK for zoned random assess files
    return rocksdb::IOStatus::OK();
  }
  rocksdb::IOStatus MultiRead(rocksdb::FSReadRequest *reqs, size_t num_reqs,
                              const rocksdb::IOOptions &options,
                              rocksdb::IODebugContext *dbg) override {
    // default logics
    for (size_t i = 0; i < num_reqs; ++i) {
      rocksdb::FSReadRequest &req = reqs[i];
      req.status =
          Read(req.offset, req.len, options, &req.result, req.scratch, dbg);
    }
    return rocksdb::IOStatus::OK();
  }
  size_t GetUniqueId(char *string, size_t size) const override {
    // for zoned file
    return 0;
  }
  void Hint(AccessPattern pattern) override {
    // nothing in zoned file
  }
  bool use_direct_io() const override { return inner->use_direct_io(); }
  size_t GetRequiredBufferAlignment() const override {
    return inner->GetRequiredBufferAlignment();
  }
  rocksdb::IOStatus InvalidateCache(size_t size, size_t size1) override {
    return fromAqua(inner->InvalidateCache(size, size1));
  }
  rocksdb::IOStatus ReadAsync(
      rocksdb::FSReadRequest &req, const rocksdb::IOOptions &opts,
      std::function<void(const rocksdb::FSReadRequest &, void *)> cb,
      void *cb_arg, void ** /*io_handle*/, IOHandleDeleter * /*del_fn*/,
      rocksdb::IODebugContext * /*dbg*/) override {
    req.status =
        Read(req.offset, req.len, opts, &(req.result), req.scratch, nullptr);
    cb(req, cb_arg);
    return rocksdb::IOStatus::OK();
  }
  rocksdb::Temperature GetTemperature() const override {
    return fromAqua(inner->GetTemperature());
  }
};

class FSWritableFileAdapter : public rocksdb::FSWritableFile {
  std::unique_ptr<aquafs::FSWritableFile> inner;

 public:
  explicit FSWritableFileAdapter(
      std::unique_ptr<aquafs::FSWritableFile> &&inner_)
      : inner(std::move(inner_)) {}
  rocksdb::IOStatus Append(const rocksdb::Slice &data,
                           const rocksdb::IOOptions &options,
                           rocksdb::IODebugContext *dbg) override {
    aquafs::Slice d = toAqua(data);
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Append(d, o, nullptr));
  }
  rocksdb::IOStatus Append(const rocksdb::Slice &data,
                           const rocksdb::IOOptions &options,
                           const rocksdb::DataVerificationInfo &info,
                           rocksdb::IODebugContext *dbg) override {
    return Append(data, options, dbg);
  }
  rocksdb::IOStatus PositionedAppend(
      const rocksdb::Slice &slice, uint64_t uint64,
      const rocksdb::IOOptions &options,
      rocksdb::IODebugContext *context) override {
    aquafs::Slice d = toAqua(slice);
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->PositionedAppend(d, uint64, o, nullptr));
  }
  rocksdb::IOStatus PositionedAppend(
      const rocksdb::Slice &slice, uint64_t uint64,
      const rocksdb::IOOptions &options,
      const rocksdb::DataVerificationInfo &info,
      rocksdb::IODebugContext *context) override {
    return PositionedAppend(slice, uint64, options, context);
  }
  rocksdb::IOStatus Truncate(uint64_t uint64, const rocksdb::IOOptions &options,
                             rocksdb::IODebugContext *context) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Truncate(uint64, o, nullptr));
  }
  rocksdb::IOStatus Close(const rocksdb::IOOptions &options,
                          rocksdb::IODebugContext *context) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Close(o, nullptr));
  }
  rocksdb::IOStatus Flush(const rocksdb::IOOptions &options,
                          rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Flush(o, nullptr));
  }
  rocksdb::IOStatus Sync(const rocksdb::IOOptions &options,
                         rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Sync(o, nullptr));
  }
  rocksdb::IOStatus Fsync(const rocksdb::IOOptions &options,
                          rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Fsync(o, nullptr));
  }
  bool IsSyncThreadSafe() const override { return inner->IsSyncThreadSafe(); }
  bool use_direct_io() const override { return inner->use_direct_io(); }
  size_t GetRequiredBufferAlignment() const override {
    return inner->GetRequiredBufferAlignment();
  }
  void SetWriteLifeTimeHint(rocksdb::Env::WriteLifeTimeHint hint) override {
    inner->SetWriteLifeTimeHint(toAqua(hint));
  }
  void SetIOPriority(rocksdb::Env::IOPriority pri) override {
    inner->SetIOPriority(toAqua(pri));
  }
  rocksdb::Env::IOPriority GetIOPriority() override {
    return fromAqua(inner->GetIOPriority());
  }
  rocksdb::Env::WriteLifeTimeHint GetWriteLifeTimeHint() override {
    return fromAqua(inner->GetWriteLifeTimeHint());
  }
  uint64_t GetFileSize(const rocksdb::IOOptions &options,
                       rocksdb::IODebugContext *context) override {
    aquafs::IOOptions o;  // not used
    return inner->GetFileSize(o, nullptr);
    // // default 0...
    // return 0;
  }
  void SetPreallocationBlockSize(size_t size) override {
    inner->SetPreallocationBlockSize(size);
  }
  void GetPreallocationStatus(size_t *block_size,
                              size_t *last_allocated_block) override {
    inner->GetPreallocationStatus(block_size, last_allocated_block);
  }
  size_t GetUniqueId(char *string, size_t size) const override {
    return inner->GetUniqueId(string, size);
  }
  rocksdb::IOStatus InvalidateCache(size_t size, size_t size1) override {
    return fromAqua(inner->InvalidateCache(size, size1));
  }
  rocksdb::IOStatus RangeSync(uint64_t uint64, uint64_t uint641,
                              const rocksdb::IOOptions &options,
                              rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->RangeSync(uint64, uint641, o, nullptr));
  }
  void PrepareWrite(size_t offset, size_t len,
                    const rocksdb::IOOptions &options,
                    rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    inner->PrepareWrite(offset, len, o, nullptr);
  }
  rocksdb::IOStatus Allocate(uint64_t uint64, uint64_t uint641,
                             const rocksdb::IOOptions &options,
                             rocksdb::IODebugContext *context) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Allocate(uint64, uint641, o, nullptr));
  }
};

class FSRandomRWFileAdapter : public rocksdb::FSRandomRWFile {
  std::unique_ptr<aquafs::FSRandomRWFile> inner;

 public:
  explicit FSRandomRWFileAdapter(
      std::unique_ptr<aquafs::FSRandomRWFile> &&inner_)
      : inner(std::move(inner_)) {}
  bool use_direct_io() const override { return inner->use_direct_io(); }
  size_t GetRequiredBufferAlignment() const override {
    return inner->GetRequiredBufferAlignment();
  }
  rocksdb::IOStatus Write(uint64_t offset, const rocksdb::Slice &data,
                          const rocksdb::IOOptions &options,
                          rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    aquafs::Slice d = toAqua(data);
    return fromAqua(inner->Write(offset, d, o, nullptr));
  }
  rocksdb::IOStatus Read(uint64_t offset, size_t n,
                         const rocksdb::IOOptions &options,
                         rocksdb::Slice *result, char *scratch,
                         rocksdb::IODebugContext *dbg) const override {
    aquafs::IOOptions o;  // not used
    aquafs::Slice d;
    auto ret = fromAqua(inner->Read(offset, n, o, &d, scratch, nullptr));
    *result = fromAqua(std::move(d));
    return ret;
  }
  rocksdb::IOStatus Flush(const rocksdb::IOOptions &options,
                          rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Flush(o, nullptr));
  }
  rocksdb::IOStatus Sync(const rocksdb::IOOptions &options,
                         rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Sync(o, nullptr));
  }
  rocksdb::IOStatus Fsync(const rocksdb::IOOptions &options,
                          rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Fsync(o, nullptr));
  }
  rocksdb::IOStatus Close(const rocksdb::IOOptions &options,
                          rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Close(o, nullptr));
  }
  rocksdb::Temperature GetTemperature() const override {
    return fromAqua(inner->GetTemperature());
  }
};

class FSDirectoryAdapter : public rocksdb::FSDirectory {
  std::unique_ptr<aquafs::FSDirectory> inner;

 public:
  explicit FSDirectoryAdapter(std::unique_ptr<aquafs::FSDirectory> &&inner_)
      : inner(std::move(inner_)) {}
  rocksdb::IOStatus Fsync(const rocksdb::IOOptions &options,
                          rocksdb::IODebugContext *dbg) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Fsync(o, nullptr));
  }
  rocksdb::IOStatus FsyncWithDirOptions(
      const rocksdb::IOOptions &options, rocksdb::IODebugContext *dbg,
      const rocksdb::DirFsyncOptions &fsyncOptions) override {
    return FSDirectory::FsyncWithDirOptions(options, dbg, fsyncOptions);
  }
  rocksdb::IOStatus Close(const rocksdb::IOOptions &options,
                          rocksdb::IODebugContext *context) override {
    aquafs::IOOptions o;  // not used
    return fromAqua(inner->Close(o, nullptr));
  }
  size_t GetUniqueId(char *string, size_t size) const override {
    return inner->GetUniqueId(string, size);
  }
};

rocksdb::IOStatus AquaFSFileSystemWrapper::NewSequentialFile(
    const std::string &f, const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSSequentialFile> *r,
    rocksdb::IODebugContext *dbg) {
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
  std::unique_ptr<aquafs::FSRandomAccessFile> result;
  auto ret = fromAqua(
      inner_->NewRandomAccessFile(f, toAqua(file_opts), &result, nullptr));
  *r = std::make_unique<FSRandomAccessFileAdapter>(std::move(result));
  return ret;
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NewWritableFile(
    const std::string &f, const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSWritableFile> *r, rocksdb::IODebugContext *dbg) {
  std::unique_ptr<aquafs::FSWritableFile> result;
  auto ret =
      fromAqua(inner_->NewWritableFile(f, toAqua(file_opts), &result, nullptr));
  *r = std::make_unique<FSWritableFileAdapter>(std::move(result));
  return ret;
}
rocksdb::IOStatus AquaFSFileSystemWrapper::ReopenWritableFile(
    const std::string &fname, const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSWritableFile> *result,
    rocksdb::IODebugContext *dbg) {
  std::unique_ptr<aquafs::FSWritableFile> r;
  auto ret = fromAqua(
      inner_->ReopenWritableFile(fname, toAqua(file_opts), &r, nullptr));
  *result = std::make_unique<FSWritableFileAdapter>(std::move(r));
  return ret;
}
rocksdb::IOStatus AquaFSFileSystemWrapper::ReuseWritableFile(
    const std::string &fname, const std::string &old_fname,
    const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSWritableFile> *r, rocksdb::IODebugContext *dbg) {
  std::unique_ptr<aquafs::FSWritableFile> result;
  aquafs::FileOptions o;  // not used
  auto ret = fromAqua(
      inner_->ReuseWritableFile(fname, old_fname, o, &result, nullptr));
  *r = std::make_unique<FSWritableFileAdapter>(std::move(result));
  return ret;
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NewRandomRWFile(
    const std::string &fname, const rocksdb::FileOptions &file_opts,
    std::unique_ptr<rocksdb::FSRandomRWFile> *result,
    rocksdb::IODebugContext *dbg) {
  std::unique_ptr<aquafs::FSRandomRWFile> r;
  aquafs::FileOptions o;  // not used
  auto ret = fromAqua(inner_->NewRandomRWFile(fname, o, &r, nullptr));
  *result = std::make_unique<FSRandomRWFileAdapter>(std::move(r));
  return ret;
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
  aquafs::IOOptions o;  // not used
  std::unique_ptr<aquafs::FSDirectory> r;
  auto ret = fromAqua(inner_->NewDirectory(name, o, &r, nullptr));
  *result = std::make_unique<FSDirectoryAdapter>(std::move(r));
  return ret;
}
rocksdb::IOStatus AquaFSFileSystemWrapper::FileExists(
    const std::string &f, const rocksdb::IOOptions &io_opts,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->FileExists(f, o, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetChildren(
    const std::string &dir, const rocksdb::IOOptions &io_opts,
    std::vector<std::string> *r, rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->GetChildren(dir, o, r, nullptr));
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
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->DeleteFile(f, o, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::Truncate(
    const std::string &fname, size_t size, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->Truncate(fname, size, o, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::CreateDir(
    const std::string &d, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->CreateDir(d, o, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::CreateDirIfMissing(
    const std::string &d, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->CreateDirIfMissing(d, o, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::DeleteDir(
    const std::string &d, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->DeleteDir(d, o, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetFileSize(
    const std::string &f, const rocksdb::IOOptions &options, uint64_t *s,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->GetFileSize(f, o, s, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetFileModificationTime(
    const std::string &fname, const rocksdb::IOOptions &options,
    uint64_t *file_mtime, rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(
      inner_->GetFileModificationTime(fname, o, file_mtime, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetAbsolutePath(
    const std::string &db_path, const rocksdb::IOOptions &options,
    std::string *output_path, rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->GetAbsolutePath(db_path, o, output_path, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::RenameFile(
    const std::string &s, const std::string &t,
    const rocksdb::IOOptions &options, rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->RenameFile(s, t, o, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::LinkFile(
    const std::string &s, const std::string &t,
    const rocksdb::IOOptions &options, rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->LinkFile(s, t, o, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::NumFileLinks(
    const std::string &fname, const rocksdb::IOOptions &options,
    uint64_t *count, rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->NumFileLinks(fname, o, count, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::AreFilesSame(
    const std::string &first, const std::string &second,
    const rocksdb::IOOptions &options, bool *res,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->AreFilesSame(first, second, o, res, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::LockFile(
    const std::string &f, const rocksdb::IOOptions &options,
    rocksdb::FileLock **l, rocksdb::IODebugContext *dbg) {
  aquafs::FileLock *lock;
  static_assert(sizeof(aquafs::FileLock) == sizeof(rocksdb::FileLock),
                "FileLock size mismatch");
  aquafs::IOOptions o;  // not used
  auto status = fromAqua(inner_->LockFile(f, o, &lock, nullptr));
  if (status.ok()) {
    *l = reinterpret_cast<rocksdb::FileLock *>(lock);
  }
  return status;
}
rocksdb::IOStatus AquaFSFileSystemWrapper::UnlockFile(
    rocksdb::FileLock *l, const rocksdb::IOOptions &options,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(
      inner_->UnlockFile(reinterpret_cast<aquafs::FileLock *>(l), o, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::GetTestDirectory(
    const rocksdb::IOOptions &options, std::string *path,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  if (inner_ != nullptr) {
    return fromAqua(inner_->GetTestDirectory(o, path, nullptr));
  } else {
    *path = std::string("rocksdbtest");
    printf("GetTestDirectory: %s aux\n", path->c_str());
    // std::string dirname =
    //     (static_cast<aquafs::AquaFS *>(inner_.get()))->ToAuxPath(*path);
    // return fromAqua(inner_->CreateDirIfMissing(dirname, o, nullptr));
    return rocksdb::IOStatus::OK();
  }
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
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->GetFreeSpace(path, o, diskfree, nullptr));
}
rocksdb::IOStatus AquaFSFileSystemWrapper::IsDirectory(
    const std::string &path, const rocksdb::IOOptions &options, bool *is_dir,
    rocksdb::IODebugContext *dbg) {
  aquafs::IOOptions o;  // not used
  return fromAqua(inner_->IsDirectory(path, o, is_dir, nullptr));
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
          AquaFSFileSystemWrapper *wrapper = nullptr;
          if (fs != nullptr) {
            wrapper = new AquaFSFileSystemWrapper(
                std::unique_ptr<aquafs::FileSystem>(fs),
                rocksdb::FileSystem::Default());
          }
          f->reset(wrapper);
          return f->get();
        });

}  // namespace aquafs
