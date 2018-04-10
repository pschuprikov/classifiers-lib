with import <nixpkgs> {};

let my_boost = pkgs.boost165.override { python = pkgs.python3; };
in pkgs.stdenv.mkDerivation {
  name = "p4t_native";

  src = ./.;

  buildInputs = with pkgs; [ cmake my_boost python3 gperftools gcc ];
}
