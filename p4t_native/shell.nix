with import <nixpkgs> {};

let boost = pkgs.boost.override { python = pkgs.python35; };
in pkgs.stdenv.mkDerivation {
  name = "p4t_native";

  src = ./.;

  buildInputs = with pkgs; [ cmake python35 boost gperftools ];
}
