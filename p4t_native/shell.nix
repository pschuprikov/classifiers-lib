with import <nixpkgs> {};

let boost = pkgs.boost163.override { python = pkgs.python3; };
    boost_np = boost.overrideAttrs (attrs: { 
      buildInputs = attrs.buildInputs ++ [python3Packages.numpy];
    });
in pkgs.stdenv.mkDerivation {
  name = "p4t_native";

  src = ./.;

  buildInputs = with pkgs; [ cmake python3 boost_np gperftools pkgs.gcc7 ];
}
