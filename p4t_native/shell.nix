with import <nixpkgs> {};

let boost = pkgs.boost163.override { python = pkgs.python3; };
in pkgs.stdenv.mkDerivation {
  name = "p4t_native";

  src = ./.;

  buildInputs = with pkgs; [ cmake python3 boost gperftools pkgs.gcc7 ];
}
