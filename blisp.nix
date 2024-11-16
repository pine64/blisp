{
  lib,
  self,
  stdenv,
  fetchFromGitHub,
  argtable,
  cmake,
  libserialport,
  pkg-config,
  testers,
  IOKit ? null,
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "blisp";
  version = "0.0.4-unstable";
  src = self;

  nativeBuildInputs = [
    cmake
    pkg-config
  ];

  buildInputs = [
    argtable
    libserialport
  ] ++ lib.optional stdenv.hostPlatform.isDarwin IOKit;

  cmakeFlags = [
    "-DBLISP_BUILD_CLI=ON"
    "-DBLISP_USE_SYSTEM_LIBRARIES=ON"
  ];

  env.NIX_CFLAGS_COMPILE = lib.optionalString stdenv.hostPlatform.isDarwin "-Wno-error=implicit-function-declaration";

  passthru.tests.version = testers.testVersion {
    package = finalAttrs.finalPackage;
    version = "v${finalAttrs.version}";
  };

  meta = with lib; {
    description = "In-System-Programming (ISP) tool & library for Bouffalo Labs RISC-V Microcontrollers and SoCs";
    license = licenses.mit;
    mainProgram = "blisp";
    homepage = "https://github.com/pine64/blisp";
    platforms = platforms.unix;
    maintainers = [ maintainers.bdd ];
  };
})
