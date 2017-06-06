with import <nixpkgs> {};
with pkgs.python35Packages;

let oct2py = import ./oct2py.nix {
  pythonPackages = pkgs.python35Packages; 
  inherit fetchFromGitHub;
};
    psc_python = python35.withPackages (pkgs: [
      pytest bitstring pytestrunner oct2py ipython] );
    psc_boost = pkgs.boost.override { python = python35; };
in pkgs.stdenv.mkDerivation rec {
  name = "cls";
  src = ./.;
  propagatedBuildInputs = [ psc_boost psc_python ];
  PYTHONPATH = "p4t_native/build";
}

