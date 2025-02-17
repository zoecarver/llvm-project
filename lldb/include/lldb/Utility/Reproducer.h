//===-- Reproducer.h --------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_UTILITY_REPRODUCER_H
#define LLDB_UTILITY_REPRODUCER_H

#include "lldb/Utility/FileSpec.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileCollector.h"
#include "llvm/Support/YAMLTraits.h"

#include <mutex>
#include <string>
#include <vector>

namespace lldb_private {
namespace repro {

class Reproducer;

enum class ReproducerMode {
  Capture,
  Replay,
  Off,
};

/// The provider defines an interface for generating files needed for
/// reproducing.
///
/// Different components will implement different providers.
class ProviderBase {
public:
  virtual ~ProviderBase() = default;

  const FileSpec &GetRoot() const { return m_root; }

  /// The Keep method is called when it is decided that we need to keep the
  /// data in order to provide a reproducer.
  virtual void Keep(){};

  /// The Discard method is called when it is decided that we do not need to
  /// keep any information and will not generate a reproducer.
  virtual void Discard(){};

  // Returns the class ID for this type.
  static const void *ClassID() { return &ID; }

  // Returns the class ID for the dynamic type of this Provider instance.
  virtual const void *DynamicClassID() const = 0;

  virtual llvm::StringRef GetName() const = 0;
  virtual llvm::StringRef GetFile() const = 0;

protected:
  ProviderBase(const FileSpec &root) : m_root(root) {}

private:
  /// Every provider knows where to dump its potential files.
  FileSpec m_root;

  virtual void anchor();
  static char ID;
};

template <typename ThisProviderT> class Provider : public ProviderBase {
public:
  static const void *ClassID() { return &ThisProviderT::ID; }

  const void *DynamicClassID() const override { return &ThisProviderT::ID; }

  llvm::StringRef GetName() const override { return ThisProviderT::Info::name; }
  llvm::StringRef GetFile() const override { return ThisProviderT::Info::file; }

protected:
  using ProviderBase::ProviderBase; // Inherit constructor.
};

class FileProvider : public Provider<FileProvider> {
public:
  struct Info {
    static const char *name;
    static const char *file;
  };

  FileProvider(const FileSpec &directory)
      : Provider(directory),
        m_collector(std::make_shared<llvm::FileCollector>(
            directory.CopyByAppendingPathComponent("root").GetPath(),
            directory.GetPath())) {}

  std::shared_ptr<llvm::FileCollector> GetFileCollector() {
    return m_collector;
  }

  void Keep() override {
    auto mapping = GetRoot().CopyByAppendingPathComponent(Info::file);
    // Temporary files that are removed during execution can cause copy errors.
    if (auto ec = m_collector->copyFiles(/*stop_on_error=*/false))
      return;
    m_collector->writeMapping(mapping.GetPath());
  }

  static char ID;

private:
  std::shared_ptr<llvm::FileCollector> m_collector;
};

/// Provider for the LLDB version number.
///
/// When the reproducer is kept, it writes the lldb version to a file named
/// version.txt in the reproducer root.
class VersionProvider : public Provider<VersionProvider> {
public:
  VersionProvider(const FileSpec &directory) : Provider(directory) {}
  struct Info {
    static const char *name;
    static const char *file;
  };
  void SetVersion(std::string version) {
    assert(m_version.empty());
    m_version = std::move(version);
  }
  void Keep() override;
  std::string m_version;
  static char ID;
};

class DataRecorder {
public:
  DataRecorder(const FileSpec &filename, std::error_code &ec)
      : m_filename(filename.GetFilename().GetStringRef()),
        m_os(filename.GetPath(), ec, llvm::sys::fs::OF_Text), m_record(true) {}

  static llvm::Expected<std::unique_ptr<DataRecorder>>
  Create(const FileSpec &filename);

  template <typename T> void Record(const T &t, bool newline = false) {
    if (!m_record)
      return;
    m_os << t;
    if (newline)
      m_os << '\n';
    m_os.flush();
  }

  const FileSpec &GetFilename() { return m_filename; }

  void Stop() {
    assert(m_record);
    m_record = false;
  }

private:
  FileSpec m_filename;
  llvm::raw_fd_ostream m_os;
  bool m_record;
};

class CommandProvider : public Provider<CommandProvider> {
public:
  struct Info {
    static const char *name;
    static const char *file;
  };

  CommandProvider(const FileSpec &directory) : Provider(directory) {}

  DataRecorder *GetNewDataRecorder();

  void Keep() override;
  void Discard() override;

  static char ID;

private:
  std::vector<std::unique_ptr<DataRecorder>> m_data_recorders;
};

class ProcessGDBRemoteProvider
    : public repro::Provider<ProcessGDBRemoteProvider> {
public:
  struct Info {
    static const char *name;
    static const char *file;
  };

  ProcessGDBRemoteProvider(const FileSpec &directory) : Provider(directory) {}

  llvm::raw_ostream *GetHistoryStream();

  void SetCallback(std::function<void()> callback) {
    m_callback = std::move(callback);
  }

  void Keep() override { m_callback(); }
  void Discard() override { m_callback(); }

  static char ID;

private:
  std::function<void()> m_callback;
  std::unique_ptr<llvm::raw_fd_ostream> m_stream_up;
};

/// The generator is responsible for the logic needed to generate a
/// reproducer. For doing so it relies on providers, who serialize data that
/// is necessary for reproducing  a failure.
class Generator final {

public:
  Generator(const FileSpec &root);
  ~Generator();

  /// Method to indicate we want to keep the reproducer. If reproducer
  /// generation is disabled, this does nothing.
  void Keep();

  /// Method to indicate we do not want to keep the reproducer. This is
  /// unaffected by whether or not generation reproduction is enabled, as we
  /// might need to clean up files already written to disk.
  void Discard();

  /// Create and register a new provider.
  template <typename T> T *Create() {
    std::unique_ptr<ProviderBase> provider = std::make_unique<T>(m_root);
    return static_cast<T *>(Register(std::move(provider)));
  }

  /// Get an existing provider.
  template <typename T> T *Get() {
    auto it = m_providers.find(T::ClassID());
    if (it == m_providers.end())
      return nullptr;
    return static_cast<T *>(it->second.get());
  }

  /// Get a provider if it exists, otherwise create it.
  template <typename T> T &GetOrCreate() {
    auto *provider = Get<T>();
    if (provider)
      return *provider;
    return *Create<T>();
  }

  const FileSpec &GetRoot() const;

private:
  friend Reproducer;

  ProviderBase *Register(std::unique_ptr<ProviderBase> provider);

  /// Builds and index with provider info.
  void AddProvidersToIndex();

  /// Map of provider IDs to provider instances.
  llvm::DenseMap<const void *, std::unique_ptr<ProviderBase>> m_providers;
  std::mutex m_providers_mutex;

  /// The reproducer root directory.
  FileSpec m_root;

  /// Flag to ensure that we never call both keep and discard.
  bool m_done;
};

class Loader final {
public:
  Loader(const FileSpec &root);

  template <typename T> FileSpec GetFile() {
    if (!HasFile(T::file))
      return {};

    return GetRoot().CopyByAppendingPathComponent(T::file);
  }

  llvm::Error LoadIndex();

  const FileSpec &GetRoot() const { return m_root; }

private:
  bool HasFile(llvm::StringRef file);

  FileSpec m_root;
  std::vector<std::string> m_files;
  bool m_loaded;
};

/// The reproducer enables clients to obtain access to the Generator and
/// Loader.
class Reproducer {
public:
  static Reproducer &Instance();
  static llvm::Error Initialize(ReproducerMode mode,
                                llvm::Optional<FileSpec> root);
  static bool Initialized();
  static void Terminate();

  Reproducer() = default;

  Generator *GetGenerator();
  Loader *GetLoader();

  const Generator *GetGenerator() const;
  const Loader *GetLoader() const;

  FileSpec GetReproducerPath() const;

protected:
  llvm::Error SetCapture(llvm::Optional<FileSpec> root);
  llvm::Error SetReplay(llvm::Optional<FileSpec> root);

private:
  static llvm::Optional<Reproducer> &InstanceImpl();

  llvm::Optional<Generator> m_generator;
  llvm::Optional<Loader> m_loader;

  mutable std::mutex m_mutex;
};

/// Helper class for replaying commands through the reproducer.
class CommandLoader {
public:
  CommandLoader(std::vector<std::string> files) : m_files(files) {}

  static std::unique_ptr<CommandLoader> Create(Loader *loader);
  llvm::Optional<std::string> GetNextFile();

private:
  std::vector<std::string> m_files;
  unsigned m_index = 0;
};

} // namespace repro
} // namespace lldb_private

#endif // LLDB_UTILITY_REPRODUCER_H
