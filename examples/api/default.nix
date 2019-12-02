{ stdenv, nix-gitignore, cmake, electionguard }:
stdenv.mkDerivation {
  pname = "electionguard-sdk";
  version = "0.0.1";

  nativeBuildInputs = [ cmake electionguard ];

  enableParallelBuilding = true;

  doCheck = true;

  src = nix-gitignore.gitignoreSourcePure ["*.nix" ../../.gitignore] ./.;
}
