{pythonPackages, fetchFromGitHub}:

with pythonPackages;

let
  metakernel = pythonPackages.buildPythonPackage rec {
    name = "metakernel-${version}";
    version = "0.20.3";

    src = fetchFromGitHub {
      owner = "Calysto";
      repo = "metakernel";
      rev = "92a90a37bd36a55d9be5df082d3b2110c831892f";
      sha256 = "108dnxdfcw678hy61mk4pklxylpiw1lzq1n7qb19v8vw6js7n9az";
    };

    propagatedBuildInputs = [ ipykernel ];
    doCheck = false;
  };

  octave_kernel = pythonPackages.buildPythonPackage rec {
    name = "octave_kernel-${version}";
    version = "0.26.2";

    src = fetchFromGitHub {
      owner = "Calysto";
      repo = "octave_kernel";
      rev = "7399120bfbeaeccf02b30b3a6b68aa7928b4170d";
      sha256 = "0g58yqm5czn781x1p5mxn1y663pnniaj3p15k2kpanc3w6rg1wvk";
    };
  
    propagatedBuildInputs = [ ipykernel metakernel ];
    doCheck = false;
  };
  
in pythonPackages.buildPythonPackage rec {
  name = "oct2py-${version}";
  version = "4.0.6";

  src = fetchFromGitHub {
    owner = "blink1073";
    repo = "oct2py";
    rev = "9504dbfe95995f77d227ffb68395220cd4012d66";
    sha256 = "0650r9nis26bzcwypqf2rxh7fv8spl27ds25swahbzkdk7i7pgk6";
  };

  propagatedBuildInputs = [ octave_kernel scipy ];
  doCheck = false;
}
