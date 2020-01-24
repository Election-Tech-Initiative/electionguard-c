let
  rev = "ed27ba064ca6f8c783";
  nixpkgs = builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/${rev}.tar.gz";
    sha256 = "1lhpbgi8gcdmwwwkgdvlqdk1n4hr5fri8vracl3axzh9n9qrdrxr";
  };
in with import nixpkgs {}; rec {
  electionguard = callPackages ./derivation.nix { };
  api = callPackages ./examples/api { inherit electionguard; };
}
