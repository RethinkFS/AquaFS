//
// Created by chiro on 23-5-27.
//

#include <gflags/gflags.h>

#include <iostream>

#include "../AquaZFS/fs/version.h"

int main(int argc, char **argv) {
  std::string help_text = std::string("\nUSAGE:\n") + argv[0] +
                          +" <command> [OPTIONS]...\nCommands: mkfs ...";
  gflags::SetUsageMessage(help_text);
  if (argc < 2) {
    std::cout << help_text << std::endl;
    return 1;
  }
  gflags::SetVersionString(AQUAFS_VERSION);
  std::string subcmd(argv[1]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // TODO
  return 0;
}