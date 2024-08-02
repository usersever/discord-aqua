{pkgs}: {
  deps = [
    pkgs.nlohmann_json
    pkgs.libsodium
    pkgs.libopus
    pkgs.openssl
    pkgs.zlib
    pkgs.cmake
  ];
}
