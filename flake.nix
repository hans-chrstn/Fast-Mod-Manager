{
  description = "Native Linux mod manager development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        llvm = pkgs.llvmPackages;
        scriptRuntime = [
          pkgs.bash
          pkgs.cmake
          pkgs.coreutils
          pkgs.cppcheck
          pkgs.git
          pkgs.jq
          pkgs.nix
          pkgs.ninja
          pkgs.pkg-config
          pkgs.ripgrep
          pkgs.shellcheck
          pkgs.catch2_3
          pkgs.qt6.qtbase
          pkgs.qt6.qtwayland
          pkgs.qt6.qtsvg
          pkgs.bubblewrap
          pkgs.just
          pkgs.valgrind
          pkgs.libarchive
          llvm.clang
          pkgs.clang-tools
        ];
        mkProjectScript = name: script:
          pkgs.writeShellApplication {
            name = "project-${name}";
            runtimeInputs = scriptRuntime;
            text = ''
              if [ -n "''${PROJECT_SOURCE_ROOT:-}" ]; then
                project_source_root="$PROJECT_SOURCE_ROOT"
              elif [ -f "$PWD/Source/flake.nix" ]; then
                project_source_root="$PWD/Source"
              else
                project_source_root="$PWD"
              fi
              export PROJECT_SOURCE_ROOT="$project_source_root"
              export PROJECT_SCRIPTS_DIR="${toString ./scripts}"
              exec ${pkgs.bash}/bin/bash "${script}" "$@"
            '';
          };
        configure = mkProjectScript "configure" ./scripts/configure.sh;
        build = mkProjectScript "build" ./scripts/build.sh;
        test = mkProjectScript "test" ./scripts/test.sh;
        format = mkProjectScript "format" ./scripts/format.sh;
        lint = mkProjectScript "lint" ./scripts/lint.sh;
        check = mkProjectScript "check" ./scripts/check.sh;
        run = mkProjectScript "run" ./scripts/run.sh;
        lock = mkProjectScript "lock" ./scripts/lock.sh;
      in {
        formatter = pkgs.nixfmt-rfc-style;

        devShells.default = pkgs.mkShell {
          packages = [
            pkgs.bashInteractive
            pkgs.catch2_3
            llvm.clang
            pkgs.clang-tools
            pkgs.cmake
            pkgs.cppcheck
            pkgs.direnv
            pkgs.git
            pkgs.jq
            pkgs.ninja
            pkgs.nixfmt-rfc-style
            pkgs.pkg-config
            pkgs.qt6.qtbase
            pkgs.qt6.qtwayland
            pkgs.qt6.qtsvg
            pkgs.bubblewrap
            pkgs.just
            pkgs.valgrind
            pkgs.libarchive
            pkgs.ripgrep
            pkgs.shellcheck
          ];

          shellHook = ''
            if [ -f "$PWD/Source/flake.nix" ]; then
              export PROJECT_SOURCE_ROOT="$PWD/Source"
            else
              export PROJECT_SOURCE_ROOT="$PWD"
            fi
            export PROJECT_SCRIPTS_DIR="$PROJECT_SOURCE_ROOT/scripts"
            export CC="${llvm.clang}/bin/clang"
            export CXX="${llvm.clang}/bin/clang++"
            export CMAKE_GENERATOR="Ninja"
            export PATH="$PROJECT_SCRIPTS_DIR:$PATH"
          '';
        };

        packages = {
          inherit configure build test format lint check run lock;
          default = check;
        };

        apps = {
          configure = {
            type = "app";
            program = "${configure}/bin/project-configure";
          };
          build = {
            type = "app";
            program = "${build}/bin/project-build";
          };
          test = {
            type = "app";
            program = "${test}/bin/project-test";
          };
          format = {
            type = "app";
            program = "${format}/bin/project-format";
          };
          lint = {
            type = "app";
            program = "${lint}/bin/project-lint";
          };
          check = {
            type = "app";
            program = "${check}/bin/project-check";
          };
          run = {
            type = "app";
            program = "${run}/bin/project-run";
          };
          lock = {
            type = "app";
            program = "${lock}/bin/project-lock";
          };
        };
      });
}
