with import <nixpkgs> {};
with pkgs.python3Packages;

let oct2py = import ./oct2py.nix {
      pythonPackages = pkgs.python3Packages; 
      inherit fetchFromGitHub;
    };
    psc_python = python3.withPackages (pkgs: [
      pytest pytestrunner oct2py ipython numpy
    ]);
    psc_boost = pkgs.boost.override { python = python3; };
in pkgs.stdenv.mkDerivation rec {
  name = "cls";
  src = ./.;
  propagatedBuildInputs = [ psc_boost psc_python ];
  PYTHONPATH = "p4t_native/build";
}

