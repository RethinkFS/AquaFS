
AquaFS_SOURCES-y = \
	src/router.cc \
  AquaZFS/programs/aquazfs.cc \
  AquaZFS/programs/test_mkfs.cc \
  AquaZFS/programs/defconfig.cc \
  AquaZFS/fs/zonefs_aquafs.cc \
  AquaZFS/fs/configuration.cc \
  AquaZFS/fs/zbd_aquafs.cc \
  AquaZFS/fs/zbdlib_aquafs.cc \
  AquaZFS/fs/io_aquafs.cc \
  AquaZFS/fs/metrics_prometheus.cc \
  AquaZFS/fs/raid/zone_raidc.cc \
  AquaZFS/fs/raid/zone_raid_auto.cc \
  AquaZFS/fs/raid/zone_raid.cc \
  AquaZFS/fs/raid/zone_raid0.cc \
  AquaZFS/fs/raid/zone_raid1.cc \
  AquaZFS/fs/fs_aquafs.cc \
  AquaZFS/base/hash.cc \
  AquaZFS/base/slice.cc \
  AquaZFS/base/fs_posix.cc \
  AquaZFS/base/file_system.cc \
  AquaZFS/base/xxhash.cc \
  AquaZFS/base/coding.cc \
  AquaZFS/base/string_util.cc \
  AquaZFS/base/io_status.cc \
  AquaZFS/base/unique_id_gen.cc \
  AquaZFS/base/crc32c.cc \
  AquaZFS/base/port.cc \
  AquaZFS/base/io_posix.cc \
  AquaZFS/base/status.cc \
  AquaZFS/base/env.cc \
  db/adapter.cc

AquaFS_HEADERS-y = \
	src/router.h \
  AquaZFS/fs/filesystem_utility.h \
  AquaZFS/fs/metrics_sample.h \
  AquaZFS/fs/metrics_prometheus.h \
  AquaZFS/fs/io_aquafs.h \
  AquaZFS/fs/configuration.h \
  AquaZFS/fs/version.h \
  AquaZFS/fs/zonefs_aquafs.h \
  AquaZFS/fs/zbd_aquafs.h \
  AquaZFS/fs/snapshot.h \
  AquaZFS/fs/raid/zone_raid1.h \
  AquaZFS/fs/raid/zone_raid_auto.h \
  AquaZFS/fs/raid/zone_raid0.h \
  AquaZFS/fs/raid/zone_raidc.h \
  AquaZFS/fs/raid/zone_raid.h \
  AquaZFS/fs/zbdlib_aquafs.h \
  AquaZFS/fs/fs_aquafs.h \
  AquaZFS/fs/metrics.h \
  AquaZFS/base/env.h \
  AquaZFS/base/lang.h \
  AquaZFS/base/coding.h \
  AquaZFS/base/sys_time.h \
  AquaZFS/base/math128.h \
  AquaZFS/base/xxph3.h \
  AquaZFS/base/port.h \
  AquaZFS/base/status.h \
  AquaZFS/base/iostats_context_imp.h \
  AquaZFS/base/unique_id_gen.h \
  AquaZFS/base/fastrange.h \
  AquaZFS/base/coding_lean.h \
  AquaZFS/base/file_system.h \
  AquaZFS/base/crc32c.h \
  AquaZFS/base/io_posix.h \
  AquaZFS/base/hash128.h \
  AquaZFS/base/slice_transform.h \
  AquaZFS/base/string_util.h \
  AquaZFS/base/mutexlock.h \
  AquaZFS/base/slice.h \
  AquaZFS/base/hash.h \
  AquaZFS/base/io_status.h \
  AquaZFS/base/math.h \
  AquaZFS/base/sync_point.h \
  AquaZFS/base/system_clock.h \
  AquaZFS/base/xxhash.h \
  db/adapter.h

AquaFS_PKGCONFIG_REQUIRES-y += "libzbd >= 1.5.0"

#AQUAFS_EXPORT_PROMETHEUS ?= n
#AquaFS_HEADERS-$(AQUAFS_EXPORT_PROMETHEUS) += fs/metrics_prometheus.h
#AquaFS_SOURCES-$(AQUAFS_EXPORT_PROMETHEUS) += fs/metrics_prometheus.cc
#AquaFS_CXXFLAGS-$(AQUAFS_EXPORT_PROMETHEUS) += -DAQUAFS_EXPORT_PROMETHEUS=1
#AquaFS_PKGCONFIG_REQUIRES-$(AQUAFS_EXPORT_PROMETHEUS) += ", prometheus-cpp-pull == 1.1.0"

AquaFS_SOURCES += $(AquaFS_SOURCES-y)
AquaFS_HEADERS += $(AquaFS_HEADERS-y)
AquaFS_CXXFLAGS += $(AquaFS_CXXFLAGS-y)
AquaFS_LDFLAGS += -u aquafs_filesystem_reg

AQUAFS_ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

#$(shell cd $(AQUAFS_ROOT_DIR) && ./generate-version.sh)
#ifneq ($(.SHELLSTATUS),0)
#$(error Generating AquaFS version failed)
#endif

AquaFS_PKGCONFIG_REQUIRES = $(AquaFS_PKGCONFIG_REQUIRES-y)
