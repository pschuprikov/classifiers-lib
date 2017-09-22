with import <nixpkgs> {};

let boost = pkgs.boost.override { python = pkgs.python3; };
in pkgs.stdenv.mkDerivation {
  name = "p4t_native";

  src = ./.;

  buildInputs = with pkgs; [ cmake python3 boost gperftools ];
}
