{pkgs}: {
   deps = [
     pkgs.wget
     pkgs.nlohmann_json
     pkgs.libsodium
     pkgs.libopus
     pkgs.openssl
     pkgs.zlib
     pkgs.cmake
     pkgs.pkg-config
     pkgs.openldap
     pkgs.libpsl
     pkgs.libssh2
     pkgs.xorg.libpthreadstubs
     pkgs.libmysqlclient
   ];
 }
