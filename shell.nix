with import <nixpkgs> {};
with pkgs.python3Packages;

let psc_python = python35.withPackages (pkgs: [
      pytest bitstring pytestrunner oct2py ipython
    ]);
    psc_boost = pkgs.boost.override { python = python3; };
in pkgs.stdenv.mkDerivation rec {
  name = "cls";
  src = ./.;
  propagatedBuildInputs = [ psc_boost psc_python ];
  PYTHONPATH = "p4t_native/build";
}

